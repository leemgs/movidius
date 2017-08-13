#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mvnc.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize.h"

typedef unsigned short half;

// Defined in fp16.c
void floattofp16(unsigned char *dst, float *src, unsigned nelem);
void fp16tofloat(float *dst, unsigned char *src, unsigned nelem);

double seconds()
{
	static double s;
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	if(!s)
		s = ts.tv_sec + ts.tv_nsec * 1e-9;
	return ts.tv_sec + ts.tv_nsec * 1e-9 - s;
}

void *loadfile(const char *path, unsigned int *length)
{
	FILE *fp;
	char *buf;

	fp = fopen(path, "rb");
	if(fp == NULL)
		return 0;
	fseek(fp, 0, SEEK_END);
	*length = ftell(fp);
	rewind(fp);
	if(!(buf = malloc(*length)))
	{
		fclose(fp);
		return 0;
	}
	if(fread(buf, 1, *length, fp) != *length)
	{
		fclose(fp);
		free(buf);
		return 0;
	}
	fclose(fp);
	return buf;
}

static float *cmpdata;
int indexcmp(const void *a1, const void *b1)
{
	int *a = (int *)a1;
	int *b = (int *)b1;
	float diff = cmpdata[*b] - cmpdata[*a];
	if(diff < 0)
		return -1;
	else if(diff > 0)
		return 1;
	else return 0;
}

int loadgraphdata(const char *dir, int *reqsize, float *mean, float *std)
{
	char path[300];
	int i;
	
	snprintf(path, sizeof(path), "%s/stat.txt", dir);
	FILE *fp = fopen(path, "r");
	if(!fp)
	{
		perror(path);
		return -1;
	}
	if(fscanf(fp, "%f %f %f\n%f %f %f\n", mean, mean+1, mean+2, std, std+1, std+2) != 6)
	{
		printf("%s: mean and stddev not found in file\n", path);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	for(i = 0; i < 3; i++)
	{
		mean[i] = 255.0 * mean[i];
		std[i] = 1.0 / (255.0 * std[i]);
	}
	snprintf(path, sizeof(path), "%s/inputsize.txt", dir);
	fp = fopen(path, "r");
	if(!fp)
	{
		perror(path);
		return -1;
	}
	if(fscanf(fp, "%d", reqsize) != 1)
	{
		printf("%s: inputsize not found in file\n", path);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}

half *loadimage(const char *path, int reqsize, float *mean, float *std)
{
	int width, height, cp, i;
	unsigned char *img, *imgresized;
	float *imgfp32;
	half *imgfp16;

	img = stbi_load(path, &width, &height, &cp, 3);
	if(!img)
	{
		printf("The picture %s could not be loaded\n", path);
		return 0;
	}
	imgresized = malloc(3*reqsize*reqsize);
	if(!imgresized)
	{
		free(img);
		perror("malloc");
		return 0;
	}
	stbir_resize_uint8(img, width, height, 0, imgresized, reqsize, reqsize, 0, 3);
	free(img);
	imgfp32 = malloc(sizeof(*imgfp32) * reqsize * reqsize * 3);
	if(!imgfp32)
	{
		free(imgresized);
		perror("malloc");
		return 0;
	}
	for(i = 0; i < reqsize * reqsize * 3; i++)
		imgfp32[i] = imgresized[i];
	free(imgresized);
	imgfp16 = malloc(sizeof(*imgfp16) * reqsize * reqsize * 3);
	if(!imgfp16)
	{
		free(imgfp32);
		perror("malloc");
		return 0;
	}
	for(i = 0; i < reqsize*reqsize; i++)
	{
		imgfp32[3*i] = (imgfp32[3*i]-mean[0])*std[0];
		imgfp32[3*i+1] = (imgfp32[3*i+1]-mean[1])*std[1];
		imgfp32[3*i+2] = (imgfp32[3*i+2]-mean[2])*std[2];
	}
	floattofp16((unsigned char *)imgfp16, imgfp32, 3*reqsize*reqsize);
	free(imgfp32);
	return imgfp16;
}

void runinference(void *graph, void *dev, const char *path, char **categories, int reqsize, float *mean, float *std)
{
	int i, throttling;
	void *userParam;
	void *result;
	unsigned int resultlen;
	float *timetaken;
	unsigned int timetakenlen, throttlinglen;
	int *indexes;
	half *img;
	float *resultfp32;
	double t;

	img = loadimage(path, reqsize, mean, std);
	t = seconds();
	int rc = mvncLoadTensor(graph, img, 3*reqsize*reqsize*sizeof(*img), 0);
	if(rc)
	{
		free(img);
		printf("LoadTensor failed, rc=%d\n", rc);
		return;
	}
	free(img);
	rc = mvncGetResult(graph, &result, &resultlen, &userParam);
	if(rc)
	{
		if(rc == MVNC_MYRIADERROR)
		{
			char *debuginfo;
			unsigned debuginfolen;
			rc = mvncGetGraphOption(graph, MVNC_DEBUGINFO, (void **)&debuginfo, &debuginfolen);
			if(rc == 0)
			{
				printf("GetResult failed, myriad error: %s\n", debuginfo);
				return;
			}
		}
		printf("GetResult failed, rc=%d\n", rc);
		return;
	}
	resultlen /= sizeof(unsigned short);
	resultfp32 = malloc(resultlen * sizeof(*resultfp32));
	fp16tofloat(resultfp32, result, resultlen);
	indexes = malloc(sizeof(*indexes) * resultlen);
	for(i = 0; i < (int)resultlen; i++)
		indexes[i] = i;
	cmpdata = resultfp32;
	qsort(indexes, resultlen, sizeof(*indexes), indexcmp);
	for(i = 0; i < 5 && i < resultlen; i++)
		printf("%s (%.2f%%) ", categories[indexes[i]], resultfp32[indexes[i]] * 100.0);
	printf("\n");
	free(resultfp32);
	free(indexes);
	rc = mvncGetGraphOption(graph, MVNC_TIMETAKEN, (void **)&timetaken, &timetakenlen);
	if(rc)
	{
		printf("GetGraphOption failed, rc=%d\n", rc);
		return;
	}
	timetakenlen = timetakenlen / sizeof(*timetaken);
	float sum = 0;
	for(i = 0; i < timetakenlen; i++)
		sum += timetaken[i];
	printf("Inference time: %f ms, total time %f ms\n", sum, (seconds() - t) * 1000.0);
	rc = mvncGetDeviceOption(dev, MVNC_THERMAL_THROTTLING_LEVEL, (void **)&throttling, &throttlinglen);
	if(rc)
	{
		printf("GetGraphOption failed, rc=%d\n", rc);
		return;
	}
	if(throttling == 1)
		printf("** NCS temperature high - thermal throttling initiated **\n");
	if(throttling == 2)
	{
		printf("*********************** WARNING *************************\n");
		printf("* NCS temperature critical                              *\n");
		printf("* Aggressive thermal throttling initiated               *\n");
		printf("* Continued use may result in device damage             *\n");
		printf("*********************************************************\n");
	}
}

int help()
{
	fprintf(stderr, "./ncs-fullcheck [-l<loglevel>] [-c<count>] <network directory> <image>\n");
	fprintf(stderr, "                <count> is the number of inference iterations, default 2\n");
	fprintf(stderr, "                <network directory> is the directory that contains graph, stat.txt,\n");
	fprintf(stderr, "                categories.txt and inputsize.txt\n");
	return 0;
}

static char **categories;
static int ncategories;

int loadcategories(const char *path)
{
	char line[300], *p;
	FILE *fp = fopen(path, "r");
	if(!fp)
	{
		perror(path);
		return -1;
	}
	ncategories = 0;
	categories = malloc(1000 * sizeof(*categories));
	while(fgets(line, sizeof(line), fp))
	{
		p = strchr(line, '\n');
		if(p)
			*p = 0;
		if(strcasecmp(line, "classes"))
		{
			categories[ncategories++] = strdup(line);
			if(ncategories == 1000)
				break;
		}
	}
	fclose(fp);
	return 0;
}

int main(int argc, char **argv)
{
	char name[MVNC_MAXNAMESIZE];
	int rc, i;
	void *h, *g;
	int loglevel = 0, inference_count = 2;
	const char *network = 0;
	const char *picture = 0;
	float mean[3], std[3];
	int reqsize;
	void *graphfile;
	char path[300];
	unsigned len;

	for(i = 1; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			if(argv[i][1] == 'l')
				loglevel = atoi(argv[i]+2);
			if(argv[i][1] == 'c')
				inference_count = atoi(argv[i]+2);
		} else if(!network)
			network = argv[i];
		else if(!picture)
			picture = argv[i];
	}
	if(!picture)
		return help();
	mvncSetDeviceOption(0, MVNC_LOGLEVEL, &loglevel, sizeof(loglevel));
	if(mvncGetDeviceName(0, name, sizeof(name)))
	{
		printf("No devices found\n");
		return -1;
	}
	if( (rc=mvncOpenDevice(name, &h) ))
	{
		printf("OpenDevice %s failed, rc=%d\n", name, rc);
		return -1;
	}
	printf("OpenDevice %s succeeded\n", name);
	snprintf(path, sizeof(path), "%s/graph", network);
	graphfile = loadfile(path, &len);
	if(graphfile)
	{
		snprintf(path, sizeof(path), "%s/categories.txt", network);
		if(!loadcategories(path))
		{
			if(!loadgraphdata(network, &reqsize, mean, std))
			{
				rc = mvncAllocateGraph(h, &g, graphfile, len);
				if(!rc)
				{
					printf("Graph allocated\n");
					for(i = 0; i < inference_count; i++)
						runinference(g, h, picture, categories, reqsize, mean, std);
					rc = mvncDeallocateGraph(g);
					printf("Deallocate graph, rc=%d\n", rc);
				} else printf("AllocateGraph failed, rc=%d\n", rc);
			}
		}
		free(graphfile);
	} else fprintf(stderr, "%s/graph not found\n", network);
	printf("Device closed, rc=%d\n", mvncCloseDevice(h));
	for(i = 0; i < ncategories; i++)
		free(categories[i]);
	free(categories);
	return 0;
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "mvnc.h"

static void timestamp(const char *text)
{
	time_t t;
	struct tm *tm_info;
	char date[80];
	time(&t);
	tm_info = localtime(&t);

	strftime(date, sizeof(date)-1, "%c", tm_info);
	printf("%s%s\n", text, date);
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

void *getresults(void *params)
{
	int i, rc, throttling;
	void *userParam;
	void *result;
	unsigned int resultlen;
	float *timetaken;
	unsigned int timetakenlen, throttlinglen;
	void *graph, *dev;

	graph = ((void **)params)[0];
	dev = ((void **)params)[1];
	for(;;)
	{
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
					return 0;
				}
			}
			printf("GetResult failed, rc=%d\n", rc);
			return 0;
		}
		printf("Returned %u bytes of result\n", resultlen);
		rc = mvncGetGraphOption(graph, MVNC_TIMETAKEN, (void **)&timetaken, &timetakenlen);
		if(rc)
		{
			printf("GetGraphOption failed, rc=%d\n", rc);
			return 0;
		}
		printf("Returned %u bytes of timetaken:\n", timetakenlen);
		timetakenlen = timetakenlen / sizeof(*timetaken);
		float sum = 0;
		for(i = 0; i < timetakenlen; i++)
		{
			printf("%d: %f\n", i, timetaken[i]);
			sum += timetaken[i];
		}
		printf("Total time: %f ms\n", sum);
		timestamp("result @ ");
		rc = mvncGetDeviceOption(dev, MVNC_THERMAL_THROTTLING_LEVEL, (void **)&throttling, &throttlinglen);
		if(rc)
		{
			printf("GetGraphOption failed, rc=%d\n", rc);
			return 0;
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
		if((int)(size_t)userParam & 0x40000000)
			break;
	}
	return 0;
}

int help()
{
	fprintf(stderr, "./ncs-threadcheck [-l<loglevel>] [-c<count>] <network directory>\n");
	fprintf(stderr, "                  <count> is the number of inference iterations, default 2\n");
	fprintf(stderr, "                  <network directory> is the directory that contains graph, stat.txt,\n");
	fprintf(stderr, "                  categories.txt and inputsize.txt\n");
	fprintf(stderr, "                  a dummy picture will be used for inference\n");
	return 0;
}

int main(int argc, char **argv)
{
	char name[MVNC_MAXNAMESIZE];
	int rc, i;
	void *h;
	int loglevel = 0, inference_count = 2;
	const char *network = 0;

	for(i = 1; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			if(argv[i][1] == 'l')
				loglevel = atoi(argv[i]+2);
			if(argv[i][1] == 'c')
				inference_count = atoi(argv[i]+2);
		} else network = argv[i];
	}
	if(!network)
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
	char path[300];
	void *graphfile;
	unsigned len;
	void *g;
	snprintf(path, sizeof(path), "%s/graph", network);
	graphfile = loadfile(path, &len);
	if(graphfile)
	{
		rc = mvncAllocateGraph(h, &g, graphfile, len);
		if(!rc)
		{
			printf("Graph allocated\n");
			pthread_t tid;
			void *retval;
			void *params[2] = {g, h};
			pthread_create(&tid, 0, getresults, params);
			for(i = 0; i < inference_count; i++)
			{
				char inputtensor[100];
				int last = i == inference_count-1 ? 0x40000000 : 0;
				rc = mvncLoadTensor(g, inputtensor, sizeof(inputtensor), (void *)(size_t)(i | last));
				if(rc)
				{
					printf("LoadTensor failed, rc=%d\n", rc);
					return -1;
				}
			}
			pthread_join(tid, &retval);
			rc = mvncDeallocateGraph(g);
			printf("Deallocate graph, rc=%d\n", rc);
		} else printf("AllocateGraph failed, rc=%d\n", rc);
		free(graphfile);
	} else fprintf(stderr, "%s/graph not found\n", network);
	printf("Device closed, rc=%d\n", mvncCloseDevice(h));
	return 0;
}


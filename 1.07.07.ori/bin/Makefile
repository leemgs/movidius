python_version_full := $(wordlist 2,4,$(subst ., ,$(shell python3 --version 2>&1)))
python_version_minor := $(word 2,${python_version_full})


.PHONY: py
py:
	@echo ${python_version_minor}


.PHONY: check
check:
	@python3 test_install.py

.PHONY: help
help:
	@cat Readme.txt
	@grep "echo" Makefile | egrep -v "grep|ERR" | sed 's/.*echo/   make/g'  | sed 's/\"//g'

.PHONY: example00
example00:
	@echo "example00 - lenet8 generation"
	python3 ./mvNCCompile.pyc data/lenet8.prototxt -s 12

.PHONY: example01
example01:
	echo "example01 - GoogleNet profiling"
	python3 ./mvNCProfile.pyc data/googlenet.prototxt -s 12

.PHONY: example02
example02:
	echo "example02 - lenet8 profiling"
	python3 ./mvNCProfile.pyc data/lenet8.prototxt -s 12

.PHONY: example03
example03:
	echo "example03 - lenet8 validation (single image)"
	python3 ./mvNCCheck.pyc data/lenet8.prototxt -i examples/data/test/0/00157.png -s 12

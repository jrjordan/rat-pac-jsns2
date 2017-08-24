# -*- mode: Makefile -*-

CC = g++

ROOT_INCLUDES = `root-config --cflags`
ROOT_LIBS = `root-config --glibs` -lGeom
RAT_INCLUDES = -I$(RATROOT)/include
RAT_LIBS = -L$(RATROOT)/lib -lRATEvent -lrat
LOCAL_INC = -I. 


CFLAGS = -g -fPIC
CFLAGS += $(ROOT_INCLUDES) $(RAT_INCLUDES) $(LOCAL_INC)
LDFLAGS += `root-config --ldflags` 
LDLIBS = $(RAT_LIBS) $(ROOT_LIBS) -lstdc++

all: analyzeRUT analyzeCaptureTest analyzeCaptureTest

analyzeRUT.o: analyzeRUT.cc
	$(CC) $(CFLAGS) -c -o $@ $^

analyzeRUT: analyzeRUT.o
	$(CC) $(LDFLAGS) -o $@ analyzeRUT.o $(LDLIBS)

analyzeReflectTest.o: analyzeReflectTest.cc
	$(CC) $(CFLAGS) -c -o $@ $^

analyzeReflectTest: analyzeReflectTest.o
	$(CC) $(LDFLAGS) -o $@ analyzeReflectTest.o $(LDLIBS)

analyzeCaptureTest.o: analyzeCaptureTest.cc
	$(CC) $(CFLAGS) -c -o $@ $^

analyzeCaptureTest: analyzeCaptureTest.o
	$(CC) $(LDFLAGS) -o $@ analyzeCaptureTest.o $(LDLIBS)

clean: 
	rm *.o analyzeRUT analyzeCaptureTest analyzeCaptureTest

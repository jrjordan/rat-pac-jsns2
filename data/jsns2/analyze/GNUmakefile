# -*- mode: Makefile -*-

#CC = clang++
CC = g++

ROOT_INCLUDES = `root-config --cflags`
ROOT_LIBS = `root-config --glibs` -lGeom
RAT_INCLUDES = -I$(RATROOT)/include
RAT_LIBS = -L$(RATROOT)/lib -lRATEvent -lrat
LOCAL_INC = -I. 


CFLAGS = -g -fPIC
#CFLAGS = -O3
CFLAGS += $(ROOT_INCLUDES) $(RAT_INCLUDES) $(LOCAL_INC)
LDFLAGS += `root-config --ldflags` 
LDLIBS = $(RAT_LIBS) $(ROOT_LIBS) -lstdc++

all: analyze_data

analyze_data.o: analyze_data.cc
	$(CC) $(CFLAGS) -c -o $@ $^

analyze_data: analyze_data.o
	$(CC) $(LDFLAGS) -o $@ analyze_data.o $(LDLIBS)

clean: 
	rm *.o analyze_data 

#
# Radiostity 
#

CXX=g++
#CFLAG=-O2 -fopenmp -std=c++11 -pg -g3 -msse -Wall -Iinclude -D_DEBUG
CFLAG=-O2 -fopenmp -std=c++11 -msse -Wall -Iinclude

OBJECTS=shade.o raycast.o report.o
HEADERS=./include/rad.h ./include/raycast.h ./include/report.h ./include/shade.h ./include/vector.h ./include/config.h

.PHONY: all clean

all: rad $(OBJECTS)

rad: ./src/rad.cpp $(OBJECTS) $(HEADERS)
	$(CXX) $(CFLAG) $(OBJECTS) ./src/rad.cpp -o rad -lm 

shade.o: ./src/shade.cpp $(HEADERS)
	$(CXX) $(CFLAG) -c ./src/shade.cpp

raycast.o: ./src/raycast.cpp $(HEADERS)
	$(CXX) $(CFLAG) -c ./src/raycast.cpp

report.o: ./src/report.c $(HEADERS)
	$(CXX) $(CFLAG) -c ./src/report.c

clean: 
	-rm rad
	-rm *.o

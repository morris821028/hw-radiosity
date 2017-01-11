#
# Radiostity 
#

CXX=g++
#CFLAG=-O2 -fopenmp -std=c++11 -pg -g3 -msse -msse2 -Wall -Iinclude -D_DEBUG
CFLAG=-O2 -fopenmp -march=native -std=c++11 -msse -msse2 -Wall -Iinclude
#CFLAG=-O2 -fopenmp -std=c++11 -msse -msse2 -Wall -Iinclude

OBJECTS=shade.o raycast.o
HEADERS=./include/*.h

.PHONY: all clean

all: rad $(OBJECTS)

rad: ./src/rad.cpp $(OBJECTS) $(HEADERS)
	$(CXX) $(CFLAG) $(OBJECTS) ./src/rad.cpp -o rad -lm 

shade.o: ./src/shade.cpp $(HEADERS)
	$(CXX) $(CFLAG) -c ./src/shade.cpp

raycast.o: ./src/raycast.cpp $(HEADERS)
	$(CXX) $(CFLAG) -c ./src/raycast.cpp

clean: 
	-rm rad
	-rm *.o

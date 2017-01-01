#
# Radiostity 
#

CXX=g++
#CFLAG=-O2 -pg -msse -Wall -Iinclude
CFLAG=-O2 -msse -Wall -Iinclude

OBJECTS=shade.o raycast.o vector.o report.o
HEADERS=./include/rad.h ./include/raycast.h  ./include/report.h  ./include/shade.h  ./include/vector.h

all: rad $(OBJECTS)

rad: ./src/rad.c $(OBJECTS) $(HEADERS)
	$(CXX) $(CFLAG) $(OBJECTS) ./src/rad.c -o rad -lm 

shade.o: ./src/shade.cpp $(HEADERS)
	$(CXX) $(CFLAG) -c ./src/shade.cpp

raycast.o: ./src/raycast.c $(HEADERS)
	$(CXX) $(CFLAG) -c ./src/raycast.c

vector.o: ./src/vector.c $(HEADERS)
	$(CXX) $(CFLAG) -c ./src/vector.c

report.o: ./src/report.c $(HEADERS)
	$(CXX) $(CFLAG) -c ./src/report.c

clean: 
	-rm rad
	-rm *.o

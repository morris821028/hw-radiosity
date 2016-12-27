#
# Radiostity 
#

CXX=g++
CFLAG=-O2 -g -Wall -Iinclude

OBJS=shade.o raycast.o vector.o report.o

all: rad $(OBJS)

rad: ./src/rad.c $(OBJS)
	$(CXX) $(CFLAG) $(OBJS) ./src/rad.c -o rad -lm 

shade.o: ./src/shade.c
	$(CXX) $(CFLAG) -c ./src/shade.c

raycast.o: ./src/raycast.c
	$(CXX) $(CFLAG) -c ./src/raycast.c

vector.o: ./src/vector.c
	$(CXX) $(CFLAG) -c ./src/vector.c

report.o: ./src/report.c
	$(CXX) $(CFLAG) -c ./src/report.c

clean: 
	-rm rad
	-rm *.o

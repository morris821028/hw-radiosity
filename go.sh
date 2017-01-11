#!/bin/bash

#MODELNAME=church.tri
MODELNAME=room.tri
#MODELNAME=blocks.tri
#MODELNAME=hall.tri

make
mkdir -p output
rm output/*
time -p ./rad \
	-write_cycle 100 \
	-triangle 40000 \
	-light 1 \
	-jobs 20 \
	-parallel_src 10 \
	model/${MODELNAME} -o output/test
#time -p ./rad -delta_ff 0.001 -write_cycle 20 -triangle 1000000 -light 1 model/${MODELNAME} -o ./output/test
cd ./WebGL_demo && ./run.sh ../output/test.fin
#cd ./demo && ./run.sh test.fin

#cp output/test.fin ./demo
#cd ./demo && make
#./tri2json -i test.fin -o test.json --format COLOR
#mv test.json ./public/asset

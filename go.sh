#!/bin/bash

#MODELNAME=church.tri
MODELNAME=room.tri
#MODELNAME=blocks.tri
#MODELNAME=hall.tri

make
mkdir -p output
rm output/*
#time -p ./rad -write_cycle 10 -light 50 model/${MODELNAME} -o ./output/test
time -p ./rad -converge 1200 -write_cycle 1 -triangle 30000 -light 1 model/${MODELNAME} -o ./output/test
#time -p ./rad -converge 800 -write_cycle 1 -triangle 200000 -light 10 model/${MODELNAME} -o ./output/test
cd ./demo && ./run.sh test.fin

#cp output/test.fin ./demo
#cd ./demo && make
#./tri2json -i test.fin -o test.json --format COLOR
#mv test.json ./public/asset

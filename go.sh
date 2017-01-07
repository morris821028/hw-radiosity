#!/bin/bash

MODELNAME=room.tri
#MODELNAME=blocks.tri
#MODELNAME=hall.tri

make
mkdir -p output
rm output/*
#time -p ./rad -l 10 -t 1000000 -i 50 model/${MODELNAME} output/test
time -p ./rad -c 800 -l 10 -t 30000 -i 1 model/${MODELNAME} output/test
cd ./demo && ./run.sh test.fin

#cp output/test.fin ./demo
#cd ./demo && make
#./tri2json -i test.fin -o test.json --format COLOR
#mv test.json ./public/asset

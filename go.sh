#!/bin/bash

#MODELNAME=room.tri
MODELNAME=blocks.tri

make
mkdir -p output
rm output/*
time -p ./rad -f 0.0001 -t 20000 -i 2 model/${MODELNAME} output/test
cd ./demo && ./run.sh

#cp output/test.fin ./demo
#cd ./demo && make
#./tri2json -i test.fin -o test.json --format COLOR
#mv test.json ./public/asset

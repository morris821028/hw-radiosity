#!/bin/bash

MODELNAME=room.tri
#MODELNAME=blocks.tri

make
mkdir -p output
time -p ./rad model/${MODELNAME} output/test

cp output/test.fin ./demo
cd ./demo && make
./tri2json -i test.fin -o test.json --format COLOR
mv test.json ./public/asset

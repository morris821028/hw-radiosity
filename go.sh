#!/bin/bash

MODELNAME=room.tri

make
mkdir -p output
./rad model/${MODELNAME} output/test
cp output/test.fin ./demo
cd ./demo && ./tri2json -i test.fin -o test.json --format COLOR

#!/bin/bash

FILENAME=test.fin

make
cp ../output/${FILENAME} .
./tri2json -i ${FILENAME} -o test.json --format COLOR
mv test.json public/asset/


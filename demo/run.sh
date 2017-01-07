#!/bin/bash

#FILENAME=test.fin
FILENAME=${1}

make
cp ../output/${FILENAME} .
./tri2json -i ${FILENAME} -o test.json --format COLOR
mv test.json public/asset/


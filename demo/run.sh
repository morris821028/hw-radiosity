#!/bin/bash

cp ../output/test.fin .
./tri2json -i test.fin -o test.json --format COLOR

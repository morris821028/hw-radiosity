#!/bin/bash

make
./rad model/blocks.tri output/test
cp output/test.fin ./demo
./demo/tri2json -i ./demo/test.fin -o ./demo/public/asset/test.json --format COLOR

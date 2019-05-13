#!/bin/bash

counter=0

for file in `cat data/memo`
do
    echo $file
    root -l -q "converter.cpp+O(\"$file\")"
    mv wave.root "wave$counter.root"
    ((counter++))
done

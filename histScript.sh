#!/bin/bash

root -l -q -e ".L makeHists.cpp+O"

for file in `ls wave*.root`
do
    echo $file
    root -l -q "makeHists.cpp+O(\"$file\")"
    mv hists.root "hists_$file"
done

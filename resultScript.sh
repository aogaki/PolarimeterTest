#!/bin/bash

root -l -q -e ".L test.cpp+O"

for file in `ls Data/hists_*`
do
    echo $file
    root -l -q "test.cpp+O(\"$file\")"
done

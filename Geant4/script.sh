#!/bin/bash

macFile="tmp.mac"
echo "/run/beamOn 10000000" > $macFile

for ((i=23;i<=200;i++)) do
    ene=`echo "scale=1; $i / 10.0" | bc`
    echo $ene
    
    ./polarymeter -e $ene -m $macFile

    hadd -f pol$ene.root result_t*
    
done

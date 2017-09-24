#!/bin/bash
make && time ./a.out > ./out.ppm 

picname=$(date +'./images/%d_%m_%Y-%H_%M_%S.png')
codename=$(date +'./codebackups/%d_%m_%Y-%H_%M_%S.c')

ppmtopng ./out.ppm $picname && cp ./raymarching2.c $codename
echo -ne "\007"
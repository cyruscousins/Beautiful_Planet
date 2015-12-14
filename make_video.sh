#! /bin/bash


set -e

./test 8 w=700 f=200

cd test_video

mkdir -p png
mkdir -p mp4

#for i in *.ppm ; do convert "$i" "png/${i%.*}.png" & ; done 
#wait

#TODO -pix_fmt, want more pixels!
rm -f mp4/video.mp4
#avconv -f image2 -r 1/2 -i png/%04d.png -c:v libx264 -pix_fmt yuv420p mp4/video.mp4
avconv -f image2 -r 10 -i ./%05d.ppm -c:v libx264 -pix_fmt yuv420p -aspect 1 mp4/video.mp4

cd -

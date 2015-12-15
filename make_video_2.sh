#! /bin/bash


set -e

rm -f test_video_9/*.ppm

#./test 9 w=500 f=10
#./test 9 w=500 f=200
./test 9 w=1000 f=200

cd test_video_9

mkdir -p mp4

#TODO -pix_fmt, want more pixels!
rm -f mp4/video.mp4
#avconv -f image2 -r 1/2 -i png/%04d.png -c:v libx264 -pix_fmt yuv420p mp4/video.mp4
#avconv -f image2 -r 22 -i ./%05d.ppm -c:v libx264 -s -pix_fmt yuv420p -aspect 1 mp4/video.mp4
avconv -f image2 -r 22 -i ./%05d.ppm -c:v libx264 -s 1000x1000 -aspect 1:1 mp4/video.mp4

cd -

vlc test_video_9/mp4/video.mp4 &


# AFF4 Fuse Mounter
Program to fuse mount an AFF4 image as a raw DD image

This is a really basic quick (about 1 hr) put together fuse program to mount AFF4 images as a raw image, which you can then run through the sleuthkit tools, or any other tools you want.
I have even then use apfs-fuse to mount an APFS volume from the fuse mounted raw image of a AFF4 image using this tool.

I based it off the AFF affuse code and libfuse examples.

You need the [AFF4 CPP Lite](https://github.com/aff4/aff4-cpp-lite) library installed.

Get the [latest release](https://github.com/aff4/aff4-cpp-lite/archive/refs/tags/v2.1.1-pre.tar.gz) of AFF4 CPP Lite.

## Compile
There is no make file or build system, to compile just run the following command:

`g++ -Wall aff4fuse.cpp -I/usr/include/fuse3 -lfuse3 -lpthread  -laff4 -o aff4mount`

Enjoy.

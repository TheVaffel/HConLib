#!/bin/bash
g++ cam.cpp -D USE_TURBOJPEG -o bin/cam -I .. -lX11 -lturbojpeg

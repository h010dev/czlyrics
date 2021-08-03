#!/bin/bash

gcc -c czlyrics-spider.c;
gcc -c czlyrics-api.c;
gcc czlyrics-api.o czlyrics-spider.o main.c external/mongoose/mongoose.c external/mjson/mjson.c -I external/mongoose -I external/mjson;
rm *.o;

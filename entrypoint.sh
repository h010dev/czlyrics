#!/bin/bash

source /etc/environment \
  && cd /src/ \
  && make clean \
  && make \
  && mv Makefile.docker Makefile \
  && ./czlyrics

#!/bin/bash

source /etc/environment \
  && cd /src/ \
  && make clean \
  && make \
  && ./czlyrics

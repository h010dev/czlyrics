#!/bin/bash

source /etc/environment \
  && cd /src/ \
  && make clean \
  && make \
  && touch app.log \
  && mkdir -p cache \
  && ./czlyrics >> app.log

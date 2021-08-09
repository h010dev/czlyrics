#!/bin/bash

source /etc/environment \
  && cd /src/ \
  && make clean \
  && make \
  && touch app.log \
  && ./czlyrics >> app.log

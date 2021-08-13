#!/bin/bash

docker build -t czlyrics . \
  && docker run -d -p 8080:8080 czlyrics


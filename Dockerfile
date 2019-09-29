FROM ubuntu:18.04

RUN apt-get update && apt-get install -y build-essential cgdb \
  libcunit1 libcunit1-dev libcunit1-doc python2.7 valgrind

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    llvm libcurlpp-dev

WORKDIR /app
ADD build/bin/qpkg /usr/bin/qpkg


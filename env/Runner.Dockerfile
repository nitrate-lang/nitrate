FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    llvm

WORKDIR /app
ADD bin/qpkg /usr/bin/qpkg


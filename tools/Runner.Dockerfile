FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    llvm

WORKDIR /app
ADD build/bin/qpkg /usr/bin/qpkg


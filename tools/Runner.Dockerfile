FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    llvm

WORKDIR /app
ADD build/bin/no3 /usr/bin/no3


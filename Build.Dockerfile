# Known good build environment for the project

FROM ubuntu:22.04

# Set the working directory
WORKDIR /app

# Install dependencies
RUN apt update
RUN apt install -y cmake g++ llvm-14 libclang-common-14-dev libssl-dev libyaml-cpp-dev librapidxml-dev zlib1g-dev rapidjson-dev upx
RUN cd /usr/include && ln -s llvm-14/llvm llvm && ln -s llvm-c-14/llvm-c llvm-c

VOLUME /app/

CMD ["python3", "/app/build.py", "--release"]
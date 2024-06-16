FROM ubuntu:22.04

WORKDIR /app
VOLUME /app/

# Install dependencies
RUN apt update
RUN apt install -y cmake g++ make upx git cargo patchelf
RUN apt install -y llvm-14

# I don't know if all of these are needed, but I use them a lot in other projects
# RUN apk add openssl-dev openssl-libs-static zlib-static readline-static \
#     ncurses-static brotli-static nghttp2-static curl-static zstd-static \
#     libidn2-static libunistring-static rapidjson-dev yaml-cpp-dev \
#     curl-dev zlib-dev libdeflate-dev readline-dev libxml2-static \
#     boost-dev boost-static

RUN apt install -y libssl-dev libboost-all-dev
RUN apt install -y libzstd-dev libclang-common-14-dev

# Create symlinks for llvm14
RUN cd /usr/include && ln -s llvm-14/llvm llvm && ln -s llvm-c-14/llvm-c llvm-c
RUN cd /usr/bin && ln -s llvm-config-14 llvm14-config

# Make the build script
RUN echo "#!/bin/sh" > /opt/build.sh
RUN echo "cmake -S /app/libquixcc -B /app/build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=Off -DLIBQUIXCC_SHARED=On -DLIBQUIXCC_STATIC=Off" >> /opt/build.sh
RUN echo "cmake --build /app/build -j`nproc`" >> /opt/build.sh
RUN echo "mkdir -p /app/bin" >> /opt/build.sh
RUN echo "cp /app/build/libquixcc.so /app/bin/libquixcc.so" >> /opt/build.sh
RUN chmod +x /opt/build.sh

WORKDIR /app

CMD ["/opt/build.sh"]

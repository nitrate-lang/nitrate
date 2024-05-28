FROM alpine:3.19

WORKDIR /app
VOLUME /app/

# Install dependencies
RUN apk update
RUN apk add cmake clang make upx git cargo patchelf
RUN apk add -X http://dl-cdn.alpinelinux.org/alpine/v3.18/main llvm14-dev llvm14-static

# I don't know if all of these are needed, but I use them a lot in other projects
RUN apk add openssl-dev openssl-libs-static zlib-static readline-static \
    ncurses-static brotli-static nghttp2-static curl-static zstd-static \
    libidn2-static libunistring-static rapidjson-dev yaml-cpp-dev \
    curl-dev zlib-dev libdeflate-dev readline-dev libxml2-static \
    boost-dev boost-static

# Create symlinks for llvm14
RUN cd /usr/include && ln -s llvm14/llvm llvm && ln -s llvm14/llvm-c llvm-c

# Build and install yaml-cpp static library
WORKDIR /tmp
RUN wget https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.tar.gz -O yaml-cpp-0.8.0.tar.gz
RUN tar -xvf yaml-cpp-0.8.0.tar.gz
WORKDIR /tmp/yaml-cpp-0.8.0
RUN mkdir build
WORKDIR /tmp/yaml-cpp-0.8.0/build
RUN cmake -DYAML_BUILD_SHARED_LIBS=OFF ..
RUN make -j$(nproc) && make install
RUN rm -rf /tmp/yaml-cpp-0.8.0 /tmp/yaml-cpp-0.8.0.tar.gz

# Install tool to merge libraries
RUN cargo install --git https://github.com/tux3/armerge
RUN mv /root/.cargo/bin/armerge /usr/bin

# Make the build script
RUN echo "#!/bin/sh" > /opt/build.sh
RUN echo "cmake -S /app -B /app/build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=Off" >> /opt/build.sh
RUN echo "cmake --build /app/build -j`nproc`" >> /opt/build.sh
RUN echo "mkdir -p /app/bin" >> /opt/build.sh
RUN echo "cp /app/build/qcc/qcc /app/bin/qcc" >> /opt/build.sh
RUN echo "cp /app/build/qld/qld /app/bin/qld" >> /opt/build.sh  
RUN echo "cp /app/build/qpkg/qpkg /app/bin/qpkg" >> /opt/build.sh
RUN chmod +x /opt/build.sh

WORKDIR /app

CMD ["/opt/build.sh"]

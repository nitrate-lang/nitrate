FROM ubuntu:22.04

WORKDIR /app
VOLUME /app/

# Install dependencies
RUN apt update
RUN apt install -y cmake clang make llvm-14

RUN apt install -y libssl-dev libboost-all-dev libzstd-dev libclang-common-14-dev

# Create symlinks for llvm14
RUN cd /usr/include && ln -s llvm-14/llvm llvm && ln -s llvm-c-14/llvm-c llvm-c
RUN cd /usr/bin && ln -s llvm-config-14 llvm14-config

# Make the build script
RUN echo "#!/bin/sh" > /opt/build.sh
RUN echo "cmake -S /app -B /app/build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=Off -DLIBQUIXCC_SHARED=On -DLIBQUIXCC_STATIC=Off" >> /opt/build.sh
RUN echo "cmake --build /app/build -j`nproc`" >> /opt/build.sh
RUN echo "mkdir -p /app/bin" >> /opt/build.sh
RUN echo "cp /app/build/libquixcc/libquixcc.so /app/bin/libquixcc.so" >> /opt/build.sh
RUN chmod +x /opt/build.sh

WORKDIR /app

CMD ["/opt/build.sh"]

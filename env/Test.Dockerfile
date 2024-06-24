FROM ubuntu:22.04

WORKDIR /app
VOLUME /app/

# Install dependencies
RUN apt update
RUN apt install -y cmake clang make llvm-14 upx cargo

RUN apt install -y libssl-dev libboost-all-dev libzstd-dev libclang-common-14-dev rapidjson-dev libyaml-cpp-dev

# Install tool to merge libraries
RUN cargo install --git https://github.com/tux3/armerge
RUN mv /root/.cargo/bin/armerge /usr/bin

# Create symlinks for llvm14
RUN cd /usr/include && ln -s llvm-14/llvm llvm && ln -s llvm-c-14/llvm-c llvm-c
RUN cd /usr/bin && ln -s llvm-config-14 llvm14-config

RUN cd /usr/lib/x86_64-linux-gnu && ln -s /usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/libcrypto.a
RUN cd /usr/lib/x86_64-linux-gnu && ln -s /usr/lib/x86_64-linux-gnu/libtinfo.a /usr/lib/libtinfo.a

# Hack to fix term symbol linking
RUN cd /usr/lib/x86_64-linux-gnu && ln -sf /usr/lib/x86_64-linux-gnu/libtinfo.a /usr/lib/libncurses.a

# Make the build script
RUN echo "#!/bin/sh" > /opt/build.sh
RUN echo "cmake -S /app -B /app/build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=On -DLIBQUIXCC_SHARED=On -DLIBQUIXCC_STATIC=On" >> /opt/build.sh
RUN echo "cmake --build /app/build -j`nproc`" >> /opt/build.sh
RUN echo "mkdir -p /app/bin" >> /opt/build.sh
RUN echo "cp /app/build/qcc/qcc /app/bin/qcc" >> /opt/build.sh
RUN echo "cp /app/build/qld/qld /app/bin/qld" >> /opt/build.sh  
RUN echo "cp /app/build/qpkg/qpkg /app/bin/qpkg" >> /opt/build.sh
RUN echo "cp /app/build/libquixcc/test/libquixcc-test-suite /app/bin/libquixcc-test-suite" >> /opt/build.sh
RUN echo "/app/bin/libquixcc-test-suite" >> /opt/build.sh
RUN chmod +x /opt/build.sh

WORKDIR /app

CMD ["/opt/build.sh"]
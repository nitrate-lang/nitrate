FROM ubuntu:22.04

WORKDIR /app
VOLUME /app/

# Install dependencies
RUN apt update
RUN apt install -y cmake g++ make llvm-14 upx
RUN apt install -y libssl-dev libboost-all-dev libzstd-dev libclang-common-14-dev rapidjson-dev libyaml-cpp-dev

# Create symlinks for llvm14
RUN cd /usr/include && ln -s llvm-14/llvm llvm && ln -s llvm-c-14/llvm-c llvm-c
RUN cd /usr/bin && ln -s llvm-config-14 llvm14-config

# Make the build script
RUN echo "#!/bin/sh" > /opt/build.sh
RUN echo "cmake -S /app -B /app/build -DCMAKE_BUILD_TYPE=Release" >> /opt/build.sh
RUN echo "cmake --build /app/build -j`nproc`" >> /opt/build.sh
RUN echo "mkdir -p /app/bin" >> /opt/build.sh
RUN echo "cp /app/build/libquixcc/libquixcc.so /app/bin/libquixcc.so" >> /opt/build.sh
RUN echo "cp /app/build/qcc/qcc /app/bin/qcc" >> /opt/build.sh
RUN echo "cp /app/build/qld/qld /app/bin/qld" >> /opt/build.sh  
RUN echo "cp /app/build/qpkg/qpkg /app/bin/qpkg" >> /opt/build.sh
RUN chmod +x /opt/build.sh

WORKDIR /app

CMD ["/opt/build.sh"]

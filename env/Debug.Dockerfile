FROM ubuntu:22.04

WORKDIR /app
VOLUME /app/

# Install dependencies
RUN apt clean
RUN apt update --fix-missing
RUN apt install -y cmake make llvm-14 upx
RUN apt install -y libssl-dev libboost-all-dev libzstd-dev libclang-common-14-dev rapidjson-dev libdeflate-dev libreadline-dev

RUN apt install -y clang

# Create symlinks for llvm14
RUN cd /usr/include && ln -s llvm-14/llvm llvm && ln -s llvm-c-14/llvm-c llvm-c
RUN cd /usr/bin && ln -s llvm-config-14 llvm14-config

# Make the build script
RUN echo "#!/bin/sh" > /opt/build.sh
RUN echo "mkdir -p /app/build/debug" >> /opt/build.sh
RUN echo "cmake -S /app -B /app/build/debug -DCMAKE_BUILD_TYPE=Debug" >> /opt/build.sh
RUN echo "cmake --build /app/build/debug -j`nproc`" >> /opt/build.sh
RUN echo "mkdir -p /app/bin" >> /opt/build.sh
RUN echo "rm -rf /app/bin/*" >> /opt/build.sh
RUN echo "cp /app/build/debug/*/*.a /app/build/debug/*/*.so /app/bin/" >> /opt/build.sh
RUN echo "cp /app/build/debug/qpkg/qpkg /app/bin/qpkg" >> /opt/build.sh
RUN echo "chmod -R 777 /app/bin/" >> /opt/build.sh
RUN chmod +x /opt/build.sh

WORKDIR /app

CMD ["/opt/build.sh"]

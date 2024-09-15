FROM ubuntu:22.04

WORKDIR /app
VOLUME /app/

# Install dependencies
RUN apt clean
RUN apt update --fix-missing
RUN apt install -y cmake make llvm-14 upx
RUN apt install -y libssl-dev libboost-all-dev libzstd-dev libclang-common-14-dev rapidjson-dev libdeflate-dev libreadline-dev libcurlpp-dev libclang-dev libclang-cpp-dev

RUN apt install -y clang

# Make the build script
RUN echo "#!/bin/sh" > /opt/build.sh
RUN echo "mkdir -p /app/build/release" >> /opt/build.sh
RUN echo "cmake -S /app -B /app/build/release -DCMAKE_BUILD_TYPE=Release" >> /opt/build.sh
RUN echo "cmake --build /app/build/release -j`nproc`" >> /opt/build.sh
RUN echo "mkdir -p /app/bin" >> /opt/build.sh
RUN echo "rm -rf /app/bin/*" >> /opt/build.sh
RUN echo "cp /app/build/release/*/*.a /app/build/release/*/*.so /app/bin/" >> /opt/build.sh
RUN echo "cp /app/build/release/qpkg/qpkg /app/bin/qpkg" >> /opt/build.sh
RUN echo "chmod -R 777 /app/bin/" >> /opt/build.sh
RUN chmod +x /opt/build.sh

WORKDIR /app

CMD ["/opt/build.sh"]

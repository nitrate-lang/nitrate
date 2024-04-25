# Known good build environment for the project

FROM ubuntu:22.04

# Set the working directory
WORKDIR /app

# Install dependencies
RUN apt update
RUN apt install -y cmake g++ llvm-14 libclang-common-14-dev libssl-dev libyaml-cpp-dev librapidxml-dev zlib1g-dev rapidjson-dev upx libboost-serialization-dev
RUN cd /usr/include && ln -s llvm-14/llvm llvm && ln -s llvm-c-14/llvm-c llvm-c

VOLUME /app/

RUN echo "#!/bin/sh" > /opt/build.sh
RUN echo "cmake -S /app -B /app/build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=Off" >> /opt/build.sh
RUN echo "cmake --build /app/build -j`nproc`" >> /opt/build.sh
RUN echo "mkdir -p /app/bin" >> /opt/build.sh
RUN echo "cp /app/build/qcc/qcc /app/bin/qcc" >> /opt/build.sh
RUN echo "cp /app/build/qld/qld /app/bin/qld" >> /opt/build.sh  
RUN echo "cp /app/build/qpkg/qpkg /app/bin/qpkg" >> /opt/build.sh
RUN echo "cp /app/build/libquixcc/libquixcc.so /app/bin/libquixcc.so" >> /opt/build.sh
RUN chmod +x /opt/build.sh

CMD ["/opt/build.sh"]

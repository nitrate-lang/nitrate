FROM ubuntu:22.04

WORKDIR /app
VOLUME /app/

# Install dependencies
RUN apt clean
RUN apt update --fix-missing && apt upgrade -y
RUN apt install -y cmake make llvm-14 upx
RUN apt install -y libssl-dev libboost-all-dev libzstd-dev libclang-common-14-dev rapidjson-dev libdeflate-dev libreadline-dev libcurlpp-dev libclang-dev libclang-cpp-dev nlohmann-json3-dev

RUN apt install -y clang

# Make the build script
RUN echo "#!/bin/sh" > /opt/build.sh
RUN echo "mkdir -p /app/.build/release" >> /opt/build.sh
RUN echo "cmake -S /app -B /app/.build/release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/app/build || exit 1" >> /opt/build.sh
RUN echo "cmake --build /app/.build/release -j`nproc` || exit 1" >> /opt/build.sh
RUN echo "mkdir -p /app/build" >> /opt/build.sh
RUN echo "rm -rf /app/build/*" >> /opt/build.sh
RUN echo "cmake --install /app/.build/release || exit 1" >> /opt/build.sh
RUN echo "chmod -R 777 /app/build/ || exit 1" >> /opt/build.sh
RUN chmod +x /opt/build.sh

WORKDIR /app

CMD ["/opt/build.sh"]

# Description: Dockerfile for building the project in Release mode
# Year: 2024 AD

FROM ubuntu:24.04

######################### Install dependencies #########################
RUN apt clean
RUN apt update --fix-missing && apt upgrade -y
RUN apt install -y  libboost-all-dev libssl-dev libgoogle-glog-dev   \
                    libyaml-cpp-dev rapidjson-dev libreadline-dev    \
                    libzstd-dev libpolly-18-dev llvm-18
RUN apt install -y cmake make clang

############################ Install clang #############################
RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100

########################## Make the build script #######################
RUN echo "#!/bin/sh" > /opt/build.sh
RUN echo "mkdir -p /app/.build/release" >> /opt/build.sh
RUN echo "cmake -S /app -B /app/.build/release -DCMAKE_BUILD_TYPE=Release -DCOVERAGE=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=ON -DCMAKE_INSTALL_PREFIX=/app/build || exit 1" >> /opt/build.sh
RUN echo "cmake --build /app/.build/release -j`nproc` || exit 1" >> /opt/build.sh
RUN echo "mkdir -p /app/build" >> /opt/build.sh
RUN echo "rm -rf /app/build/*" >> /opt/build.sh
RUN echo "cmake --install /app/.build/release || exit 1" >> /opt/build.sh
RUN echo "chmod -R 777 /app/build/ || exit 1" >> /opt/build.sh
RUN chmod +x /opt/build.sh

WORKDIR /app
VOLUME /app/

CMD ["/opt/build.sh"]

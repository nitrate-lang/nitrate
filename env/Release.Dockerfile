# Description: Dockerfile for building the project in Release mode
# Version: 0.1
# Year: 2024 AD

FROM ubuntu:22.04

WORKDIR /app
VOLUME /app/

######################### Install dependencies #########################
RUN apt clean
RUN apt update --fix-missing && apt upgrade -y
RUN apt install -y cmake make clang llvm-14 upx gnupg2 dpkg-dev
RUN apt install -y  libssl-dev libboost-all-dev libzstd-dev libclang-common-14-dev \
                    rapidjson-dev libdeflate-dev libreadline-dev libclang-dev      \
                    libclang-cpp-dev nlohmann-json3-dev libcurl4-openssl-dev       \
                    libgoogle-glog-dev
RUN apt purge -y gcc g++
RUN apt autoremove -y

############################ Install clang #############################
RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100

########################## Install APT sources #########################
RUN echo "deb-src https://mirrors.cicku.me/linuxmint/packages wilma main upstream import backport"  >> /etc/apt/sources.list
RUN echo "deb-src https://la.mirrors.clouvider.net/ubuntu noble main restricted universe multiverse" >> /etc/apt/sources.list
RUN echo "deb-src https://la.mirrors.clouvider.net/ubuntu noble-updates main restricted universe multiverse" >> /etc/apt/sources.list
RUN echo "deb-src https://la.mirrors.clouvider.net/ubuntu noble-backports main restricted universe multiverse" >> /etc/apt/sources.list
RUN echo "deb-src http://security.ubuntu.com/ubuntu/ noble-security main restricted universe multiverse" >> /etc/apt/sources.list
RUN apt-key adv --recv-keys --keyserver keyserver.ubuntu.com A6616109451BBBF2 
RUN apt update

########################## Install libcurlpp ###########################
WORKDIR /opt
RUN apt source -y libcurlpp-dev
WORKDIR /opt/curlpp-0.8.1
RUN cmake -S . -B .build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_CXX_FLAGS="-fPIC" || exit 1
RUN cmake --build .build -j`nproc` || exit 1
RUN cmake --install .build || exit 1

########################## Cleanup container ##########################
WORKDIR /
RUN rm -rf /opt/*

# Make the build script
RUN echo "#!/bin/sh" > /opt/build.sh
RUN echo "mkdir -p /app/.build/release" >> /opt/build.sh
RUN echo "cmake -S /app -B /app/.build/release -DCMAKE_BUILD_TYPE=Release -DCOVERAGE=OFF -DCMAKE_INSTALL_PREFIX=/app/build || exit 1" >> /opt/build.sh
RUN echo "cmake --build /app/.build/release -j`nproc` || exit 1" >> /opt/build.sh
RUN echo "mkdir -p /app/build" >> /opt/build.sh
RUN echo "rm -rf /app/build/*" >> /opt/build.sh
RUN echo "cmake --install /app/.build/release || exit 1" >> /opt/build.sh
RUN echo "chmod -R 777 /app/build/ || exit 1" >> /opt/build.sh
RUN chmod +x /opt/build.sh

WORKDIR /app

CMD ["/opt/build.sh"]

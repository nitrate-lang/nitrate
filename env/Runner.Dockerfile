FROM ubuntu:22.04

WORKDIR /app
ADD bin/qpkg /usr/bin/qpkg

ENTRYPOINT ["/usr/bin/qpkg"]

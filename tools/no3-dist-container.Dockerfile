# Run it like:
# docker run -e HOST_CWD="$PWD" -v /:/mnt --rm -it no3-dist:latest [no3-args]

FROM ubuntu:22.04

RUN apt-get update

ADD build/bin/no3 /usr/bin/no3

RUN echo "#!/bin/bash" >> /chroot.sh
RUN echo "cd \$HOST_CWD" >> /chroot.sh
RUN echo "/dev/shm/no3-launcher/no3 \$@" >> /chroot.sh
RUN echo "rm -rf /dev/shm/no3-launcher" >> /chroot.sh


RUN echo "#!/bin/bash" >> /run.sh
RUN echo "mkdir -p /mnt/dev/shm/no3-launcher" >> /run.sh
RUN echo "cp /usr/bin/no3 /mnt/dev/shm/no3-launcher/no3" >> /run.sh
RUN echo "cp /chroot.sh /mnt/dev/shm/no3-launcher/chroot.sh" >> /run.sh
RUN echo "chmod +x /mnt/dev/shm/no3-launcher/no3 /mnt/dev/shm/no3-launcher/chroot.sh" >> /run.sh
RUN echo "chroot /mnt /dev/shm/no3-launcher/chroot.sh \$@" >> /run.sh
RUN chmod +x /run.sh

ENTRYPOINT ["/run.sh"]

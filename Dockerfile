FROM debian:buster
LABEL maintainer="user"

# KasperskyOS distributive name
ARG KOS_deb="KasperskyOS-Community-Edition_1.1.0.356_amd64.deb"

RUN apt-get update && apt-get -y install \
       sudo systemd systemd-sysv \
       build-essential wget libffi-dev libssl-dev \
       mc git cmake \
       autoconf \
       automake \
       libtool \
       gawk \
       parted \
       udev \
       python \
       qemu-system-arm \
       curl wget git nano \
       vim cmake-curses-gui gdebi-core\
       && apt-get autoremove --yes && apt-get clean --yes

COPY ./$KOS_deb /tmp/$KOS_deb
RUN gdebi  /tmp/$KOS_deb --n

# add user 'docker'
RUN useradd -r -u 1000 -U -m docker-KOS

# prepare working directory
RUN mkdir /workspace && chown -R docker-KOS /workspace
WORKDIR /workspace

ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8
ENV LANGUAGE C.UTF-8

#USER docker  
ENTRYPOINT [ "/bin/bash", "-l", "-c" ]

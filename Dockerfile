FROM ubuntu:latest as build
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y \
        binutils-mips-linux-gnu \
        bsdmainutils \
        build-essential \
        libaudiofile-dev \
        python3 \
        wget
RUN wget https://github.com/ps2dev/ps2dev/releases/download/v1.2.0/ps2dev-ubuntu-latest.tar.gz && \
    tar xzf ps2dev-ubuntu-latest.tar.gz && \
    rm ps2dev-ubuntu-latest.tar.gz

RUN mkdir /sm64
WORKDIR /sm64
ENV PATH="/ps2dev/ee/bin:/ps2dev/iop/bin:/sm64/tools:${PATH}"
ENV PS2SDK=/ps2dev/ps2sdk
ENV GSKIT=/ps2dev/gsKit

CMD echo 'usage: docker run --rm --mount type=bind,source="$(pwd)",destination=/sm64 sm64 make VERSION=${VERSION:-us} -j4'


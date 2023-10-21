FROM alpine:latest

# Install toolchain
RUN apk update && \
    apk upgrade && \
    apk add git \
            python3 \
            py3-pip \
            cmake \
            build-base \
            libusb-dev \
            bsd-compat-headers \
            gcc-arm-none-eabi \
            picolibc-arm-none-eabi \
            newlib-arm-none-eabi \
            binutils-arm-none-eabi \
            g++-arm-none-eabi

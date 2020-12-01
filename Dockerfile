# this Dockerfile is only for checking build
FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# install required libraries
RUN apt-get update && apt-get install -y wget make g++ clang libboost-all-dev
RUN wget https://files.wolframcdn.com/WolframEngine/12.1.1.0/WolframEngine_12.1.1_LINUX.sh && bash WolframEngine_12.1.1_LINUX.sh && rm WolframEngine_12.1.1_LINUX.sh

# settings about libraries
WORKDIR /lib/x86_64-linux-gnu
RUN ln -s libuuid.so.1.3.0 libuuid.so
RUN echo "/usr/local/Wolfram/WolframEngine/12.1/SystemFiles/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions" >> /etc/ld.so.conf && ldconfig

# check build
WORKDIR /work
COPY . .
ENV MATHPATH=/usr/local/Wolfram/WolframEngine/12.1 CC=gcc CXX=g++
RUN make -j

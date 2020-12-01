FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /work
RUN apt-get update && apt-get install -y git wget make clang libboost-all-dev
RUN wget https://files.wolframcdn.com/WolframEngine/12.1.1.0/WolframEngine_12.1.1_LINUX.sh && bash WolframEngine_12.1.1_LINUX.sh && rm WolframEngine_12.1.1_LINUX.sh

WORKDIR /lib/x86_64-linux-gnu
RUN ln -s libuuid.so.1.3.0 libuuid.so
RUN echo "/usr/local/Wolfram/WolframEngine/12.1/SystemFiles/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions" >> /etc/ld.so.conf && ldconfig

WORKDIR /work
COPY . .
ENV MATHPATH=/usr/local/Wolfram/WolframEngine/12.1
RUN make -j

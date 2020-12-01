# HyLaGI - HydLa Guaranteed Implementation

HyLaGI is a C++ implementation of hybrid constraint language HydLa.

Email address :
hydla@ueda.info.waseda.ac.jp

More information about HydLa :
http://www.ueda.info.waseda.ac.jp/hydla/

## Install
### Ubuntu

1. Install required packages
   ```
   sudo apt update && sudo apt install -y git make clang libboost-all-dev
   ```
1. Install Mathematica
1. Library settings  
   e.g. Mathematica 11.3
   ```
   cd /lib/x86_64-linux-gnu && sudo ln -s libuuid.so.1.3.0 libuuid.so
   echo "/usr/local/Wolfram/Mathematica/11.3/SystemFiles/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions" >> /etc/ld.so.conf && ldconfig
   ```
1. Build HyLaGI
   ```
   git clone https://github.com/HydLa/HyLaGI.git
   make -j 4
   ```
   Then, a binary `bin/hylagi` is made.

### Other environments

HyLaGI supports several environments.

- OS: Ubuntu and macOS
- C++ compiler: Clang (default) and GCC

<details>
<summary>Build confirmed environment</summary>

- Ubuntu 18.04.3, GCC 7.5.0, Python 3.6.9
- Ubuntu 18.04.3, Clang 6.0.0, Python 3.6.9
- Ubuntu 20.04.1, GCC 9.3.0, Python 3.8.5
- Ubuntu 20.04.1, Clang 10.0.0, Python 3.8.5
- macOS 10.15.7, Apple clang 12.0.0, Python 3.6.9
- macOS 10.15.7, Apple clang 12.0.0, Python 3.8.5
</details>

### Make options

To build several environments,
you can set environment variables when you exec `make`.  

- Using GCC:
  ```
  make -j 4 CC=gcc CXX=g++
  ```
- Other path/to/Mathematica:
  - in other version:
    ```
    make -j 4 MATHPATH=/usr/local/Wolfram/Mathematica/12.1
    ```
  - using WolframEngine:
    ```
    make -j 4 MATHPATH=/usr/local/Wolfram/WolframEngine/12.1
    ```
- When using Python3 with `python` command:
  ```
  make -j 4 PYTHON_CONFIG=python-config
  ```

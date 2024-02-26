# HyLaGI - HydLa Guaranteed Implementation

![build-on-ubuntu](https://github.com/HydLa/HyLaGI/workflows/build-on-ubuntu/badge.svg)

HyLaGI is a C++ implementation of hybrid constraint language HydLa.

Email address:
hydla@ueda.info.waseda.ac.jp

More information about HydLa:
http://www.ueda.info.waseda.ac.jp/hydla/

## Build

You can also use [webHydLa](http://webhydla.ueda.info.waseda.ac.jp) to run HydLa programs without building HyLaGI on your own.

### Required packages

- Git
- Make
- GCC or Clang, and Boost library
- Python
- Wolfram system (Mathematica, or WolframEngine)

### Ubuntu 22.04 with Clang

1. Install required packages
   ```bash
   sudo apt update && sudo apt install -y git make clang libboost-all-dev uuid-dev
   ```
1. Install and activate Mathematica  
   If you don't have Wolfram's license, you can use [Free Wolfram Engine for Developers](https://www.wolfram.com/engine/index.php).
1. Set `$MATHPATH` (see **What is `MATHPATH`?** below)  
   e.g., Wolfram Engine 14.0
   ```bash
   echo "export MATHPATH='/usr/local/Wolfram/WolframEngine/14.0'" >> ~/.bashrc
   source ~/.bashrc   # or restart the terminal
   ```
1. Library settings
   ```bash
   echo "$MATHPATH/SystemFiles/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions" | sudo tee /etc/ld.so.conf.d/wstp.conf
   sudo ldconfig
   ```
1. Build HyLaGI
   ```bash
   git clone https://github.com/HydLa/HyLaGI.git
   cd HyLaGI
   make -j 4 MATHPATH=$MATHPATH
   echo "export PATH='$PATH:$MATHPATH/Executables:$(pwd)/bin'" >> ~/.bashrc
   source ~/.bashrc   # or restart the terminal
   ```
   Then, you can use `hylagi` command. For example:
   ```bash
   hylagi -p 6 examples/bouncing_particle.hydla
   ```
1. Run tests  
   By default, `make test` runs tests sequentially.  
   If you want it to run in parallel, you can set the number of threads as follows:
   ```bash
   make test fnum=2
   ```
   Note that parallel execution may be restricted by your Wolfram license.
   
### Other environments

HyLaGI supports several environments.

- OS: Ubuntu and macOS
- C++ compiler: Clang (default) and GCC

<details>
<summary>Build confirmed environment</summary>

- Ubuntu 20.04.1, GCC 9.3.0, Python 3.8.5
- Ubuntu 20.04.1, Clang 10.0.0, Python 3.8.5
- Ubuntu 22.04.1, GCC 11.3.0, Python 3.10.6
- Ubuntu 22.04.1, Clang 14.0.0, Python 3.10.6
- macOS 10.15.7, Apple clang 12.0.0, Python 3.6.9
- macOS 10.15.7, Apple clang 12.0.0, Python 3.8.5
</details>

### Make options

To build several environments,
you can set environment variables when you exec `make`.  

#### Using GCC:

```
make -j 4 CC=gcc CXX=g++
```

#### When using Python3 with `python` command:

```
make -j 4 PYTHON_CONFIG=python-config
```

## What is `MATHPATH`?

`MATHPATH` is the path to the directory where the Wolfram system is installed.  
You can see it in [$InstallationDirectory](https://reference.wolfram.com/language/ref/$InstallationDirectory.html).

```
$ math
Wolfram Language 14.0.0 Engine for Linux x86 (64-bit)
Copyright 1988-2023 Wolfram Research, Inc.

In[1]:= $InstallationDirectory

Out[1]= /usr/local/Wolfram/WolframEngine/14.0
```

Examples of `MATHPATH`:
- With WolframEngine 14.0: `/usr/local/Wolfram/WolframEngine/14.0`
- With Mathematica 12.1: `/usr/local/Wolfram/Mathematica/12.1`


## Known issues

### libc++abi: terminating with uncaught exception of type hydla::backend::LinkError: math link error: can not link : 1

HyLaGI uses the wolfram system to calculate constraints.
It uses WSTP communication with the `math` command to make the call.

If you see this error, please make sure that the `math` command is installed and in the path.
If the `math` command does not exist (as confirmed when using WolframEngine on MacOS), create a symbolic link to `WolframKernel` named math.

## CI
CI supports only build (not include testing examples). If you want to test them, do ``` make test ``` after building HyLaGI on your terminal.

# HyLaGI - HydLa Guaranteed Implementation

![build-on-ubuntu](https://github.com/HydLa/HyLaGI/workflows/build-on-ubuntu/badge.svg)

HyLaGI is a C++ implementation of hybrid constraint language HydLa.

Email address:
hydla@ueda.info.waseda.ac.jp

More information about HydLa:
http://www.ueda.info.waseda.ac.jp/hydla/

## Build

Also, you can use HydLa on [webHydLa](http://webhydla.ueda.info.waseda.ac.jp) even if you don't build HyLaGI.

### Required packages

- Git
- Make
- GCC or Clang, and Boost library
- Python
- Wolfram system (Mathematica, or WolframEngine)

### Ubuntu 22.04

1. Install required packages
   ```
   sudo apt update && sudo apt install -y git make clang libboost-all-dev
   ```

1. Install and activate Mathematica  
   If you don't have Wolfram's license, you can use [Free Wolfram Engine for Developers](https://www.wolfram.com/engine/index.php).
   
1. Mathematica PATH settings  
   e.g. Mathematica 11.3 is installed in /usr/local
   ```
   export MATHPATH="/usr/local/Wolfram/Mathematica/11.3"
   ```
   Since this setting is necessary whenever you rebuild HyLaGI, it might be good to make MATHPATH permanent environment variable.
   
2. Library settings  
   e.g. Mathematica 11.3
   ```
   echo "$MATHPATH/SystemFiles/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions" >> /etc/ld.so.conf && ldconfig
   ```
3. Build HyLaGI
   ```
   git clone https://github.com/HydLa/HyLaGI.git
   cd HyLaGI
   make -j 4
   export PATH="$PATH:$MATHPATH/Executables:$(pwd)/bin"
   ```
   Then, you can use `hylagi` command.

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

#### Other path/to/Mathematica:

- in other version:
  ```
  make -j 4 MATHPATH=/usr/local/Wolfram/Mathematica/12.1
  ```
- using WolframEngine:
  ```
  make -j 4 MATHPATH=/usr/local/Wolfram/WolframEngine/12.1
  ```

##### What is `MATHPATH`?

Actually, `MATHPATH` is the path to the directory where the Wolfram system is installed.<br>
You can see it in [$InstallationDirectory](https://reference.wolfram.com/language/ref/$InstallationDirectory.html).

#### When using Python3 with `python` command:

```
make -j 4 PYTHON_CONFIG=python-config
```

## Known issues

### libc++abi: terminating with uncaught exception of type hydla::backend::LinkError: math link error: can not link : 1

HyLaGI uses the wolfram system to calculate constraints.
It uses WSTP communication with the `math` command to make the call.

If you see this error, please make sure that the `math` command is installed and in the path.
If the `math` command does not exist (as confirmed when using WolframEngine on MacOS), create a symbolic link to `WolframKernel` named math.

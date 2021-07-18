libriddle
=========

[![build](https://github.com/mohamed/libriddle/actions/workflows/cmake.yml/badge.svg)](https://github.com/mohamed/libriddle/actions/workflows/cmake.yml)

This is a minimalist and complete implementation of Shamir's secret sharing
algorithm as described in [JACM1979](https://doi.org/10.1145/359168.359176).
The goal is to provide a generic and efficient implementation.


Requirements
------------
You will need a C90 compiler, [CMake](https://cmake.org) 3.6+, and [OpenSSL](https://openssl.org) 1.1+.

Building
--------

To build and run the tests:

```
  mkdir build
  cd build
  cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<path> ..
  cmake --build . --target install
  cmake --build . --target test
```

Bugs/Questions
--------------
In case of bugs/questions, please contact me on
[Github](https://github.com/mohamed).


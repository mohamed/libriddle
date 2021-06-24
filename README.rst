libriddle
=========

This is a minimalist and complete implementation of Shamir's secret sharing
algorithm as described in [JACM1979]_.
The goal is to provide a generic and efficient implementation.

.. [JACM1979]
   Adi Shamir, "How to share a secret", Communications of the ACM 22(11):612-613
   doi:10.1145/359168.359176

Requirements
------------
You will need a C90 compiler, CMake_ 3.6+, libgmp_, and OpenSSL_ 1.1+.

.. _CMake: https://cmake.org/
.. _libgmp: https://gmplib.org/
.. _OpenSSL: https://openssl.org/


Building
--------

To build and run the tests:

::

  mkdir build
  cd build
  cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<path> ..
  cmake --build . --target install
  cmake --build . --target test


Bugs/Questions
--------------
In case of bugs/questions, please contact me on Github_

.. _Github: https://github.com/mohamed

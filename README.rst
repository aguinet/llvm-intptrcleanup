IntToPtr LLVM pass
==================

This is a WIP pass that removes IntToPtr/PtrToInt when used with PHINodes (and valid).
This conversions make ScalarEvolution fails (for good reasons) (see
http://llvm.org/docs/doxygen/html/ScalarEvolution_8cpp_source.html#l03708), and
thus make LoopVectorize miss induction variables, and the whole loop
vectorization process fails.


Usage
=====

.. code::

  $ mkdir build && cd build
  $ cmake -DLLVM_CONFIG=/path/to/llvm-config ..
  $ make

And then compare::

  $ /path/to/clang -Xclang -load -Xclang IntToPtrCleanup.so -O2 ./example/op_zip_operator.cpp -S -emit-llvm -o - -std=c++11

with the original::

  $ /path/to/clang -O2 ./example/op_zip_operator.cpp -S -emit-llvm -o - -std=c++11

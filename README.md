First, make sure that TFHE is installed, and that your environment 
variables contains the
necessary information to find TFHE include directory and library
directory.

During the compilation, `CPLUS_INCLUDE_PATH` and `C_INCLUDE_PATH` should
contain `/usr/local/include` and `LIBRARY_PATH` and
`LD_LIBRARY_PATH` should contain `/usr/local/lib`.
During the execution of a program that uses TFHE, only the variable 
`LD_LIBRARY_PATH` is needed.

You can put the following lines to your .bashrc or .profile config file
to make the changes permanent.

```
TFHE_PREFIX=/usr/local
export CPLUS_INCLUDE_PATH=${TFHE_PREFIX}/include:${CPLUS_INCLUDE_PATH}
export C_INCLUDE_PATH=${TFHE_PREFIX}/include:${C_INCLUDE_PATH}
export LIBRARY_PATH=${TFHE_PREFIX}/lib:${LIBRARY_PATH}
export LD_LIBRARY_PATH=${TFHE_PREFIX}/lib:${LD_LIBRARY_PATH}
```

Then, to compile one of the tutorial program, just pass
`-ltfhe-spqlios-fma` during the linking phase. For instance:

```
gcc alice.c -o alice -ltfhe-spqlios-fma
gcc cloud.c -o cloud -ltfhe-spqlios-fma
gcc verif.c -o verif -ltfhe-spqlios-fma
```

If you do not have the spqlios-fma version, replace with one of the version of
TFHE that you have installed (`-ltfhe-nayuki-portable`, `-ltfhe-nayuki-avx`,
`-ltfhe-fftw`, or `-ltfhe-spqlios-avx`).

You can have a look at the Makefile for other compile options!

Happy coding!


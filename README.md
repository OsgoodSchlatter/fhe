# FULLY HOMOMORPHIC ENCRYPTION CALCULATION TOOLS

## Authors: Eloi Besnard, Tiphaine Henry

Can perform addition, subtraction, multiplication and division with homomorphic encrypted data using TFHE library.

## Usage

Run `make`.  
If problems, see Notice.

- I. [CIPHERING]  
   call alice with the numbers you want to cipher.  
  Say you want to multiply 67 times 8 times 6, run:
  ` ./alice 67 8 6`
- II. [COMPUTING]  
  call cloud to choose the computation you want to do. Dont forget to tell the number of inputs.  
   Here is the format: ` ./cloud <number of inputs> <choice of computation>`  
   Choice of computation is the following:

       - 1: multiplication
       - 2: division
       - 3: addition
       - 4: subtraction
       - 5: max

  In our case, we want to multiply 3 inputs so:
  `./cloud 3 1 `

- III. [DECIPHERING]  
  Then, once cloud is done, run verif to decipher the data:
  `./verif`

## NOTICE

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

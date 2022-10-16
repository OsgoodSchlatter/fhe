# enter your TFHE installation directory here:
TFHE_PREFIX=/usr/local

# if you haven't set your C_INCLUDE_PATH variable, you can also pass 
# the include path to the compiler like this
INCS=-I${TFHE_PREFIX}/include

# if you haven't set your LIBRARY_PATH or LD_LIBRARY_PATH variables, 
# you can pass them like this to the compiler
LIBS=-L${TFHE_PREFIX}/lib -Wl,-rpath=${TFHE_PREFIX}/lib -ltfhe-spqlios-fma 

all: alice cloud verif

# this is how you can compile and link a program with tfhe
# you can try make -n to expand all macros
%: %.c
	${CC} ${INCS} -c $<
	${CC} -o $@ $@.o ${LIBS}


clean:
	rm alice cloud verif alice.o cloud.o verif.o || true
	rm cloud.key secret.key cloud.data answer.data || true

# gcc utils/full_adder_one_bit.c -o full_adder_one_bit -ltfhe-spqlios-fma
# gcc utils/multiplier.c -o multiplier -ltfhe-spqlios-fma
# gcc utils/diviser.c -o diviser -ltfhe-spqlios-fma
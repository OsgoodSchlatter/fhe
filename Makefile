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
%: %.c utils
	${CC} ${INCS} -c $<
	${CC} -o $@ $@.o ${UTILS} ${LIBS}

UTILS= addition_multiple.o compare_bit_own.o diviser_multiple.o diviser.o full_adder_one_bit.o full_adder.o full_subtract_one_bit.o full_subtract.o multiplier_multiple.o multiplier.o offset.o subtraction_multiple.o max_multiple.o

utils:${UTILS}

%.o: utils/%.c 
	${CC} ${INCS} -c $< 
	

clean:
	rm alice cloud verif alice.o cloud.o verif.o *.o || true
	rm cloud.key secret.key cloud.data answer.data || true

utils:
	${CC} ${INCS} -c utils/full_adder_one_bit.c -ltfhe-spqlios-fma
	${CC} ${INCS} -c utils/multiplier.c -ltfhe-spqlios-fma
	${CC} ${INCS} -c utils/diviser.c -ltfhe-spqlios-fma
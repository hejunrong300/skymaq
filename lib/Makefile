#######################
# Makefile
#######################

# compile and lib parameter
CROSS_COMPILE=#loongarch64-linux-gnu-
CC      := $(CROSS_COMPILE)gcc
LIBS    :=
LDFLAGS := 
DEFINES :=
INCLUDE := -I.
CFLAGS  := 
CXXFLAGS:= 

# link parameter
AR  := ar
LIB := libmath.a

#link
$(LIB):add.o
	$(AR) -r $@ $^
#compile
add.o:add.c
	$(CC) -c $^ -o $@ 

# clean
clean:
	rm -fr *.o $(LIB)
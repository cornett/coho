include config.mk

SRC =		compat.c \
		smiles.c

OBJ = $(SRC:c=o)

all: libcoho.a python

clean:
	rm -f libcoho.a $(OBJ)
	@cd python && $(MAKE) clean
	@cd test && $(MAKE) clean

python: libcoho.a
	@cd python && $(MAKE)

python.sdist:
	@cd python && $(MAKE) sdist

python.wheel:
	@cd python && $(MAKE) wheel

test: libcoho.a
	@cd test && $(MAKE)

libcoho.a: $(OBJ)
	$(AR) -r $@ $?

$(OBJ): coho.h config.mk

.PHONY: all clean python python.sdist python.wheel test

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) -I. $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

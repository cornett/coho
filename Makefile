VERSION		= 0.3

PREFIX		= /usr/local
MANPREFIX	= $(PREFIX)/share/man

CPPFLAGS	=
CFLAGS		= -Wall -O2

PYTHON		= python3
CYTHON		= cython
PYTHON_CONFIG	= $(PYTHON)-config
PYTHON_CFLAGS	= $(shell $(PYTHON_CONFIG) --cflags)
PYTHON_LDFLAGS	= $(shell $(PYTHON_CONFIG) --ldflags)
PYTHON_LIBS	= $(shell $(PYTHON_CONFIG) --libs)

-include config.mk

SRC_C		= compat.c \
		  smiles.c \
		  vec.c

SRC_H		= $(SRC_C:.c=.h)

PYTHON_SRC_PYX	= py/coho/__init__.pyx \
		  py/coho/smiles.pyx

OBJ_O		= $(SRC_C:.c=.o)

PYTHON_OBJ_C	= $(PYTHON_SRC_PYX:.pyx=.c)
PYTHON_OBJ_O	= $(PYTHON_SRC_PYX:.pyx=.o)
PYTHON_OBJ_SO	= $(PYTHON_SRC_PYX:.pyx=.so)

PYTHON_OBJ	= $(PYTHON_OBJ_C) \
		  $(PYTHON_OBJ_O) \
		  $(PYTHON_OBJ_SO)

OBJ		= libcoho.a \
		  $(OBJ_O) \
		  $(PYTHON_OBJ)

AFL		= afl/smiles/smiles

TEST		= test/smiles


all:				libcoho.a \
				$(PYTHON_OBJ_SO) \
				$(TEST)


clean:				python.clean
	rm -f $(AFL) $(OBJ) $(TEST)
	rm -rf doc/_build


doc:
	$(PYTHON) -m sphinx -M html doc doc/_build


install:			libcoho.a
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include/coho
	install -d $(DESTDIR)$(MANPREFIX)/man3
	install -m 0444 libcoho.a $(DESTDIR)$(PREFIX)/lib
	install -m 0444 smiles.h $(DESTDIR)$(PREFIX)/include/coho
	install -m 0444 man/smiles_parse.3 $(DESTDIR)$(MANPREFIX)/man3


$(AFL) $(TEST):			libcoho.a
	$(CC) $(CFLAGS) $(CPPFLAGS) -I. -o $@ $@.c libcoho.a


$(OBJ_O):			compat.h
smiles.o:			smiles.h \
				vec.h
vec.o:				vec.h


libcoho.a:			$(OBJ_O)
	$(AR) -r $@ $?


%.o:				%.c
	$(CC) $(CFLAGS) -fPIC $(CPPFLAGS) -I. -o $@ -c $<


# Python {{{

python.clean:
	cd py && \
	rm -f version.txt && \
	rm -rf __pycache__ *.egg-info && \
	rm -rf dist src


python.pre.setup.py:		python.clean
	echo $(VERSION) > py/version.txt
	rm -rf py/src
	install -d py/src
	install -m 0644 $(SRC_C) $(SRC_H) py/src


python.sdist:			python.pre.setup.py \
				$(PYTHON_OBJ_C)
	$(PYTHON) py/setup.py sdist


python.wheel:			python.pre.setup.py \
				$(PYTHON_OBJ_C)
	$(PYTHON) py/setup.py bdist_wheel


py/coho/smiles.c:		py/coho/smiles.pxd
py/coho/smiles.o:		smiles.h

py/coho/__init__.o:		PYTHON_CFLAGS += -DVERSION='"$(VERSION)"'


py/coho/%.c:			py/coho/%.pyx
	$(CYTHON) -3 -X embedsignature=True -I py/coho $<


py/coho/%.o:			py/coho/%.c
	$(CC) $(PYTHON_CFLAGS) -I. -o $@ -c $<


py/coho/%.so:			py/coho/%.o \
				libcoho.a
	$(CC) -shared $(PYTHON_LDFLAGS) -o $@ $^ $(PYTHON_LIBS)

# }}}

test:				$(TEST)
	@for t in $(TEST); do \
		echo -n "./$${t}... " ; \
		./$$t >/dev/null 2>&1 || { echo "fail"; exit 1; }; \
		echo "ok"; \
	done


.PHONY:				all \
				clean \
				doc \
				install \
				python.clean \
				python.pre.setup.py \
				python.sdist \
				python.wheel \
				test


.SUFFIXES:

.SUFFIXES:
default:		all

VERSION			= 0.4

PREFIX			= /usr/local
MANPREFIX		= $(PREFIX)/share/man

CPPFLAGS		=
CFLAGS			= -Wall -O2

PYTHON			= python3
CYTHON			= cython
PYTHON_CONFIG		= $(PYTHON)-config
PYTHON_CFLAGS		= `$(PYTHON_CONFIG) --cflags`
PYTHON_LDFLAGS		= `$(PYTHON_CONFIG) --ldflags`
PYTHON_LIBS		= `$(PYTHON_CONFIG) --libs`

CYTHON_FLAGS		= -3 -X embedsignature=True -I python/coho

-include config.mk

CFLAGS			+= -fPIC $(CPPFLAGS) -I.
PYTHON_CFLAGS		+= -fPIC -I.
PYTHON_LDFLAGS		+= -shared

SRC_C			= compat.c \
			  smiles.c

SRC_H			= coho.h

PYTHON_SRC_PYX		= python/coho/__init__.pyx \
			  python/coho/smiles.pyx

OBJ_O			= $(SRC_C:c=o)

PYTHON_OBJ_C		= $(PYTHON_SRC_PYX:pyx=c)
PYTHON_OBJ_O		= $(PYTHON_SRC_PYX:pyx=o)
PYTHON_OBJ_SO		= $(PYTHON_SRC_PYX:pyx=so)

PYTHON_OBJ		= $(PYTHON_OBJ_C) \
			  $(PYTHON_OBJ_O) \
			  $(PYTHON_OBJ_SO)

OBJ			= libcoho.a \
			  $(OBJ_O) \
			  $(PYTHON_OBJ)

AFL			= afl/smiles/smiles

TEST			= test/smiles


afl/smiles/smiles:		afl/smiles/smiles.c \
				coho.h \
				libcoho.a
	$(CC) $(CFLAGS) -o $@ $@.c libcoho.a


compat.o:			compat.c \
				coho.h
	$(CC) $(CFLAGS) -o $@ -c $(@:o=c)


libcoho.a:			$(OBJ_O)
	$(AR) -r $@ $?


python/coho/__init__.c:		python/coho/__init__.pyx \
				python/coho/__init__.pxd
	$(CYTHON) $(CYTHON_FLAGS) $(@:c=pyx)


python/coho/__init__.o:		python/coho/__init__.c \
				coho.h
	$(CC) $(PYTHON_CFLAGS) -DVERSION='"$(VERSION)"' -o $@ -c $(@:o=c)


python/coho/__init__.so:	python/coho/__init__.o \
				libcoho.a
	$(CC) $(PYTHON_LDFLAGS) -o $@ $(@:so=o) libcoho.a $(PYTHON_LIBS)


python/coho/smiles.c:		python/coho/smiles.pyx \
				python/coho/smiles.pxd
	$(CYTHON) $(CYTHON_FLAGS) $(@:c=pyx)


python/coho/smiles.o:		python/coho/smiles.c \
				coho.h
	$(CC) $(PYTHON_CFLAGS) -o $@ -c $(@:o=c)


python/coho/smiles.so:		python/coho/smiles.o \
				libcoho.a
	$(CC) $(PYTHON_LDFLAGS) -o $@ $(@:so=o) libcoho.a $(PYTHON_LIBS)


smiles.o:			smiles.c \
				coho.h
	$(CC) $(CFLAGS) -o $@ -c $(@:o=c)


test/smiles:			test/smiles.c \
				coho.h \
				libcoho.a
	$(CC) $(CFLAGS) -o $@ $@.c libcoho.a



.PHONY:				all \
				clean \
				default \
				doc \
				install \
				python.clean \
				python.pre.setup.py \
				python.sdist \
				python.wheel \
				test


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
	install -m 0444 coho.h $(DESTDIR)$(PREFIX)/include/coho
	install -m 0444 man/smiles_parse.3 $(DESTDIR)$(MANPREFIX)/man3


python.clean:
	cd python && \
	rm -f version.txt && \
	rm -rf __pycache__ *.egg-info && \
	rm -rf build dist src


python.pre.setup.py:		python.clean \
				$(PYTHON_OBJ_C)
	echo $(VERSION) > python/version.txt
	rm -rf python/src
	install -d python/src
	install -m 0644 $(SRC_C) $(SRC_H) python/src


python.sdist:			python.pre.setup.py
	$(PYTHON) python/setup.py sdist


python.wheel:			python.pre.setup.py
	$(PYTHON) python/setup.py bdist_wheel


test:				$(TEST)
	@for t in $(TEST); do \
		echo -n "./$${t}... " ; \
		./$$t >/dev/null 2>&1 || { echo "fail"; exit 1; }; \
		echo "ok"; \
	done

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
		  smiles.c

SRC_H		= coho.h

PYTHON_SRC_PYX	= python/coho/__init__.pyx \
		  python/coho/smiles.pyx

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
	install -m 0444 coho.h $(DESTDIR)$(PREFIX)/include/coho
	install -m 0444 man/smiles_parse.3 $(DESTDIR)$(MANPREFIX)/man3


$(AFL) $(TEST):			libcoho.a \
				coho.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -I. -o $@ $@.c libcoho.a


$(OBJ_O):			coho.h


libcoho.a:			$(OBJ_O)
	$(AR) -r $@ $?


%.o:				%.c
	$(CC) $(CFLAGS) -fPIC $(CPPFLAGS) -I. -o $@ -c $<


# Python {{{

python.clean:
	cd python && \
	rm -f version.txt && \
	rm -rf __pycache__ *.egg-info && \
	rm -rf build dist src


python.pre.setup.py:		python.clean
	echo $(VERSION) > python/version.txt
	rm -rf python/src
	install -d python/src
	install -m 0644 $(SRC_C) $(SRC_H) python/src


python.sdist:			python.pre.setup.py \
				$(PYTHON_OBJ_C)
	$(PYTHON) python/setup.py sdist


python.wheel:			python.pre.setup.py \
				$(PYTHON_OBJ_C)
	$(PYTHON) python/setup.py bdist_wheel


python/coho/smiles.c:		python/coho/smiles.pxd
python/coho/smiles.o:		coho.h

python/coho/__init__.o:		PYTHON_CFLAGS += -DVERSION='"$(VERSION)"'


python/coho/%.c:		python/coho/%.pyx
	$(CYTHON) -3 -X embedsignature=True -I python/coho $<


python/coho/%.o:		python/coho/%.c
	$(CC) $(PYTHON_CFLAGS) -I. -o $@ -c $<


python/coho/%.so:		python/coho/%.o \
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

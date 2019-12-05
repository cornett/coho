default: all
.PHONY: all default dep install lib
.PHONY: python python.stage python.sdist python.wheel
.SUFFIXES:

BUILD_DIR	= build
PREFIX		= /usr/local
CFLAGS		= -Wall -Wextra -fPIC -O2
CYTHON		= cython
CYTHON_FLAGS	= -3 -X embedsignature=True
PYTHON		= python3
PYTHON_CONFIG	= $(PYTHON)-config
CFLAGS_PY	= `$(PYTHON_CONFIG) --cflags` -fPIC

-include config.mk

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

version		= 0.4

b		= ${BUILD_DIR}

src.lib		= src/compat.c \
		  src/smiles.c

py.pyx		= python/coho/__init__.pyx \
		  python/coho/smiles.pyx

src		= ${src.lib} \
		  ${py.pyx}

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

include auto.mk

CFLAGS_PY.__INIT__ = -DVERSION='"${version}"'

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

$b/libcoho.a:
	${AR} -rs $@ $?

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

python:		${py.so}


python.sdist:	python.stage
	cd $b/python && ${PYTHON} setup.py sdist


python.wheel:	python.stage
	cd $b/python && ${PYTHON} setup.py bdist_wheel


python.stage:	${py.c} ${py.pyi}
	rm -rf $b/python/{src,build,dist}
	install -m 0755 -d $b/python/src
	echo ${version} > $b/python/version.txt
	touch $b/python/coho/py.typed
	install -m 0644 src/coho.h ${src.lib} $b/python/src
	install -m 0644 python/MANIFEST.in python/setup.py $b/python

#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

all:		lib \
		python


dep:
	./auto.pl Makefile > $@


install:	$b/libcoho.a
	install -d ${DESTDIR}${PREFIX}/include
	install -d ${DESTDIR}${PREFIX}/lib
	install -m0644 coho.h ${DESTDIR}${PREFIX}/include
	install -m0644 $b/libcoho.a ${DESTDIR}${PREFIX}/lib


lib:		$b/libcoho.a

# Customize to fit your system.
# See doc/INSTALL.rst for instructions.

PREFIX		= /usr/local
MANPREFIX	= $(PREFIX)/share/man

CPPFLAGS	=
CFLAGS		= -Wall -Wextra -O2 -fPIC

PYTHON		= python3
CYTHON		= cython

PY.CONFIG	= $(PYTHON)-config
PY.CFLAGS	= `$(PY.CONFIG) --cflags`
PY.LDFLAGS	= `$(PY.CONFIG) --ldflags`
PY.LDLIBS	= `$(PY.CONFIG) --libs`

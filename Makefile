VERSION		= 0.1

PREFIX		= /usr/local
MANPREFIX	= $(PREFIX)/share/man

CPPFLAGS	=
CFLAGS		= -Wall -Wextra -Werror -O2 -fPIC

PYTHON		= python3
CYTHON		= cython

PY.CONFIG	= $(PYTHON)-config
PY.CFLAGS	= `$(PY.CONFIG) --cflags`
PY.LDFLAGS	= `$(PY.CONFIG) --ldflags`
PY.LDLIBS	= `$(PY.CONFIG) --libs`

-include config.mk

LIB.C		= reallocarray.c \
		  smi.c \
		  strlcpy.c \
		  vec.c

LIB.O		= $(LIB.C:.c=.o)

PY.PYX		= py/coho/__init__.pyx \
		  py/coho/smi.pyx

PY.C		= $(PY.PYX:.pyx=.c)
PY.O		= $(PY.PYX:.pyx=.o)
PY.SO		= $(PY.PYX:.pyx=.so)

AFL		= afl/smi/smi

TEST		= test/smi

BIN		= $(AFL) \
		  $(TEST)


all:				libcoho.a \
				$(TEST) \
				$(PY.SO)


clean:
	rm -f $(BIN) $(LIB.O) libcoho.a
	rm -f $(PY.C) $(PY.O) $(PY.SO)
	rm -rf py/dist py/__pycache__ py/coho.egg-info
	rm -rf py/src py/version.txt
	rm -rf doc/_build


doc:
	$(PYTHON) -m sphinx -M html doc doc/_build


install:			all
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include/coho
	install -d $(DESTDIR)$(MANPREFIX)/man3
	install -m 0444 libcoho.a $(DESTDIR)$(PREFIX)/lib
	install -m 0444 smi.h $(DESTDIR)$(PREFIX)/include/coho
	install -m 0444 man/smi_parse.3 $(DESTDIR)$(MANPREFIX)/man3


py.sdist:			$(PY.C) \
				py/version.txt
	$(PYTHON) py/setup.py sdist


test:				$(TEST)
	@for t in $(TEST); do \
		echo -n "./$${t}... " ; \
		./$$t >/dev/null 2>&1 || { echo "fail"; exit 1; }; \
		echo "ok"; \
	done


wheel:				$(PY.SO)
	@mkdir -p py/dist
	$(PYTHON) py/wheel.py $(VERSION) $(PY.SO)


libcoho.a:			$(LIB.O)
	$(AR) -r $@ $?


py/version.txt:			Makefile
	echo $(VERSION) > $@


$(LIB.O):		compat.h
reallocarray.o:		reallocarray.c
smi.o:			smi.c smi.h vec.h
strlcpy.o:		strlcpy.c
vec.o:			vec.c vec.h
$(LIB.O):
	$(CC) $(CFLAGS) $(CPPFLAGS) -I. -o $@ -c $(@:.o=.c)


py/coho/__init__.c:	py/coho/__init__.pyx
py/coho/smi.c:		py/coho/smi.pyx py/coho/smi.pxd
$(PY.C):
	$(CYTHON) -X embedsignature=True -I py/coho -3 $(@:.c=.pyx)


py/coho/__init__.o:	py/coho/__init__.c
py/coho/smi.o:		py/coho/smi.c smi.h
$(PY.O):
	$(CC) $(PY.CFLAGS) -DVERSION='"$(VERSION)"' -I. -o $@ -c $(@:.o=.c)


$(PY.SO):		libcoho.a
py/coho/__init__.so:	py/coho/__init__.o
py/coho/smi.so:		py/coho/smi.o
$(PY.SO):
	$(CC) -shared $(PY.LDFLAGS) -o $@ $(@:.so=.o) libcoho.a $(PY.LDLIBS)


$(BIN):			libcoho.a
afl/smi/smi:		afl/smi/smi.c smi.h
test/smi:		test/smi.c smi.h
$(BIN):
	$(CC) $(CFLAGS) -I. -o $@ $@.c libcoho.a


.PHONY:				all \
				doc \
				clean \
				install \
				py.sdist
				test \
				wheel

.SUFFIXES:

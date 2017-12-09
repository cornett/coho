VERSION		= 0.2

PREFIX		= /usr/local
MANPREFIX	= $(PREFIX)/share/man

CPPFLAGS	=
CFLAGS		= -Wall -O2

PYTHON		= python3
CYTHON		= cython

PY.CONFIG	= $(PYTHON)-config
CFLAGS.PY	= `$(PY.CONFIG) --cflags` -I.
LDFLAGS.PY	= `$(PY.CONFIG) --ldflags`
LDLIBS.PY	= `$(PY.CONFIG) --libs`

-include config.mk

LIB.C		= compat.c \
		  smi.c \
		  vec.c

LIB.O		= $(LIB.C:.c=.o)

PY.PYX		= py/coho/__init__.pyx \
		  py/coho/smi.pyx

PY.C		= $(PY.PYX:.pyx=.c)
PY.O		= $(PY.PYX:.pyx=.o)
PY.SO		= $(PY.PYX:.pyx=.so)


AFL		= afl/smi/smi

TEST		= test/smi

CCO	= $(CC) $(CFLAGS) -fPIC $(CPPFLAGS) -I. -o $@ -c $(@:.o=.c)
CCPYO	= $(CC) $(CFLAGS.PY) -o $@ -c $(@:.o=.c)
CCPYSO	= $(CC) -shared $(LDFLAGS.PY) -o $@ $(@:.so=.o) libcoho.a $(LDLIBS.PY)
CYTHONC	= $(CYTHON) -X embedsignature=True -I py/coho -3 $(@:.c=.pyx)
CCEXE	= $(CC) $(CFLAGS) $(CPPFLAGS) -I. -o $@ $@.c libcoho.a


all:	libcoho.a $(TEST) $(PY.SO)


clean:
	rm -f *.o libcoho.a
	rm -f test/smi afl/smi/smi
	rm -f py/coho/*.[co] py/coho/*.so py/version.txt
	rm -rf py/dist py/__pycache__ py/coho.egg-info py/src
	rm -rf doc/_build


doc:
	$(PYTHON) -m sphinx -M html doc doc/_build


install: libcoho.a
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include/coho
	install -d $(DESTDIR)$(MANPREFIX)/man3
	install -m 0444 libcoho.a $(DESTDIR)$(PREFIX)/lib
	install -m 0444 smi.h $(DESTDIR)$(PREFIX)/include/coho
	install -m 0444 man/smi_parse.3 $(DESTDIR)$(MANPREFIX)/man3


pre.setup.py:
	echo $(VERSION) > py/version.txt
	rm -rf py/src
	install -d py/src
	install -m 0644 compat.[ch] smi.[ch] vec.[ch] py/src


sdist: pre.setup.py $(PY.C)
	$(PYTHON) py/setup.py sdist


test: $(TEST)
	@for t in $(TEST); do \
		echo -n "./$${t}... " ; \
		./$$t >/dev/null 2>&1 || { echo "fail"; exit 1; }; \
		echo "ok"; \
	done


wheel: pre.setup.py $(PY.C)
	$(PYTHON) py/setup.py bdist_wheel


$(LIB.O): compat.h
$(AFL) $(TEST) $(PY.SO): libcoho.a


afl/smi/smi: afl/smi/smi.c smi.h
	$(CCEXE)


compat.o: compat.c compat.h
	$(CCO)


libcoho.a: $(LIB.O)
	$(AR) -r $@ $?


py/coho/__init__.c: py/coho/__init__.pyx
	$(CYTHONC)


py/coho/__init__.o: py/coho/__init__.c
	$(CC) $(CFLAGS.PY) -DVERSION='"$(VERSION)"' -o $@ -c py/coho/__init__.c


py/coho/__init__.so: py/coho/__init__.o
	$(CCPYSO)


py/coho/smi.c: py/coho/smi.pyx py/coho/smi.pxd
	$(CYTHONC)


py/coho/smi.o: py/coho/smi.c smi.h
	$(CCPYO)


py/coho/smi.so: py/coho/smi.o
	$(CCPYSO)


smi.o: smi.c smi.h vec.h
	$(CCO)


test/smi: test/smi.c smi.h
	$(CCEXE)


vec.o: vec.c vec.h
	$(CCO)


.PHONY:	all doc clean install pre.setup.py sdist test wheel


.SUFFIXES:

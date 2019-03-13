include ../config.mk

PYX	= coho/__init__.pyx \
	  coho/smiles.pyx

OBJ_C	= $(PYX:pyx=c)
OBJ_O	= $(PYX:pyx=o)
OBJ_SO	= $(PYX:pyx=so)

all: $(OBJ_SO)

clean:
	rm -f $(OBJ_C) $(OBJ_O) $(OBJ_SO) version.txt
	rm -rf src
	rm -rf __pycache__ *.egg-info build dist

sdist: distsrc
	$(PYTHON) setup.py sdist

wheel: distsrc
	$(PYTHON) setup.py bdist_wheel

distsrc: $(OBJ_C)
	echo $(VERSION) > version.txt
	rm -rf src
	install -m 0755 -d src
	install -m 0644 ../*.[ch] src

.PHONY: all clean distsrc sdist wheel

$(OBJ_SO): ../libcoho.a
$(OBJ_C) $(OBJ_O): ../coho.h

coho/__init__.o: coho/__init__.c
	$(CC) -I.. $(PYTHON_CFLAGS) -DVERSION='"$(VERSION)"' -o $@ -c $(@:o=c)

.SUFFIXES:
.SUFFIXES: .c .o .pyx .so

.c.o:
	$(CC) -I.. $(PYTHON_CFLAGS) -o $@ -c $<

.pyx.c:
	$(CYTHON) -3 -X embedsignature=True -Icoho $<

.o.so:
	$(CC) -shared -o $@ $< ../libcoho.a

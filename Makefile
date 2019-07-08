.PHONY: all clean install
.PHONY: doc doc.deploy
.PHONY: python python.sdist python.wheel
.SUFFIXES:

include config.mk

LIB.SRC		= $(LIB.H) \
		  $(LIB.C)

LIB.H		= coho.h

LIB.C		= compat.c \
		  smiles.c

LIB.O		= $B/compat.o \
		  $B/smiles.o

OBJ		= $(LIB.O) \
		  $(PY.OBJ) \
		  $(DOC.OBJ)

all: $B/libcoho.a python

clean:
	rm -rf $B

install: $B/libcoho.a
	install -d $(DESTDIR)$(PREFIX)/{include,lib}
	install -m 0644 coho.h $(DESTDIR)$(PREFIX)/include
	install -m 0644 $B/libcoho.a $(DESTDIR)$(PREFIX)/lib

$B/libcoho.a: $(LIB.O)
	$(AR) -rs $@ $?

LIB.CC = mkdir -p $(@D) && $(CC) -I. $(CFLAGS) $(CPPFLAGS) -o $@ -c

$B/compat.o: compat.c					; $(LIB.CC) compat.c
$B/smiles.o: smiles.c					; $(LIB.CC) smiles.c

$(LIB.O): coho.h

#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

DOC.MAN3	= doc/coho_smiles_read.3

DOC.MAN3.HTML	= $B/doc/coho_smiles_read.3.html

DOC.MD		= doc/CHANGELOG.md \
		  doc/index.md \
		  doc/install.md

DOC.MD.HTML	= $B/doc/CHANGELOG.html \
		  $B/doc/index.html \
		  $B/doc/install.html

DOC.HTML	= $(DOC.MAN3.HTML) \
		  $(DOC.MD.HTML)

DOC.OBJ		= $(DOC.HTML)

doc: $(DOC.HTML)

doc.deploy:
	B=$B ./doc/deploy

MAN2HTML = mkdir -p $(@D) && doc/man2html doc/$(@F:.html=) > $@

$B/doc/coho_smiles_read.3.html:		doc/coho_smiles_read.3	; $(MAN2HTML)

MD2HTML = mkdir -p $(@D) && doc/md2html doc/$(@F:html=md) > $@

$B/doc/CHANGELOG.html:			doc/CHANGELOG.md	; $(MD2HTML)
$B/doc/index.html:			doc/index.md		; $(MD2HTML)
$B/doc/install.html:			doc/install.md		; $(MD2HTML)

$(DOC.HTML):		doc/style.css doc/layout.erb
$(DOC.MAN3.HTML):	doc/man2html
$(DOC.MD.HTML):		doc/md2html

#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

P		= python/coho
BP		= $B/$P

PY.PYX		= $P/__init__.pyx \
		  $P/smiles.pyx

PY.C		= $(BP)/__init__.c \
		  $(BP)/smiles.c

PY.O		= $(BP)/__init__.o \
		= $(BP)/smiles.o

PY.SO		= $(BP)/__init__.so \
		= $(BP)/smiles.so

PY.OBJ		= $(PY.C) \
		  $(PY.O) \
		  $(PY.SO)

python: $(PY.SO)

python.sdist: python.stage
	cd $B/python && $(PYTHON) setup.py sdist

python.wheel: python.stage
	cd $B/python && $(PYTHON) setup.py bdist_wheel

.PHONY: python.stage

python.stage: $(PY.C)
	rm -rf $B/python/{src,build,dist}
	install -m 0755 -d $B/python/src
	echo $(VERSION) > $B/python/version.txt
	install -m 0644 $(LIB.SRC) $B/python/src
	install -m 0644 python/MANIFEST.in python/setup.py $B/python

PY.CYTHON = mkdir -p $(@D) && $(CYTHON) $(CYTHON_FLAGS) -I$P -o $@ $P/$(@F:c=pyx)

$(BP)/__init__.c:	$P/__init__.pyx			; $(PY.CYTHON)
$(BP)/smiles.c:		$P/smiles.pyx			; $(PY.CYTHON)

PY.CC = $(CC) -I. $(PYTHON_CFLAGS) -o $@ -DVERSION='"$(VERSION)"' -c $(@:o=c)

$(BP)/__init__.o:	$(BP)/__init__.c		; $(PY.CC)
$(BP)/smiles.o:		$(BP)/smiles.c			; $(PY.CC)

PY.LD = $(CC) -shared -o $@ $(@:so=o) $B/libcoho.a

$(BP)/__init__.so:	$(BP)/__init__.o		; $(PY.LD)
$(BP)/smiles.so:	$(BP)/smiles.o			; $(PY.LD)

$(PY.C): $P/__init__.pxd
$(PY.O): coho.h
$(PY.SO): $B/libcoho.a

#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

$(OBJ): Makefile config.mk

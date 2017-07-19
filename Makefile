VERSION		= 0.1

CONFIG		= config.mk
include $(CONFIG)

LIB.OBJ		= smi.o \
		  util/vec.o \
		  $(COMPAT.OBJ)

PY.MOD		= py/coho/__init__.so \
		  py/coho/smi.so

TEST		= test/smi

COMP		= $(CC) $(CFLAGS) $(CPPFLAGS) -I. -o $@ -c $(@:.o=.c)
CY.COMP		= $(CYTHON) -X embedsignature=True -I py/coho -3 $(@:.c=.pyx)
PY.COMP		= $(CC) $(PY.CFLAGS) -I. -o $@ -c $(@:.o=.c)
PY.LINK		= $(CC) -shared $(PY.LDFLAGS) -o $@ $(@:.so=.o) \
		  libcoho.a $(PY.LDLIBS)
TEST.COMP	= $(CC) $(CFLAGS) -I. -o $@ $@.c libcoho.a


all:				libcoho.a \
				$(TEST) \
				$(PY.MOD)


clean:
	rm -f *.o
	rm -f util/*.o compat/*.o
	rm -f libcoho.a
	rm -f $(TEST)
	rm -f py/coho/*.[co] $(PY.MOD)
	rm -rf py/dist py/__pycache__
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


test:				$(TEST)
	@for t in $(TEST); do \
		echo -n "./$${t}... " ; \
		./$$t >/dev/null 2>&1 || { echo "fail"; exit 1; }; \
		echo "ok"; \
	done


wheel:				$(PY.MOD)
	@mkdir -p py/dist
	$(PYTHON) py/wheel.py $(VERSION) $(PY.MOD)


compat/reallocarray.o:		compat/reallocarray.c \
				compat.h
	$(COMP)


compat/strlcpy.o:		compat/strlcpy.c \
				compat.h
	$(COMP)


libcoho.a:			$(LIB.OBJ)
	$(AR) -r $@ $?


py/coho/smi.c:			py/coho/smi.pyx \
				py/coho/smi.pxd
	$(CY.COMP)


py/coho/smi.o:			py/coho/smi.c \
				smi.h
	$(PY.COMP)


py/coho/smi.so:			py/coho/smi.o \
				libcoho.a
	$(PY.LINK)


py/coho/__init__.c:		py/coho/__init__.pyx
	$(CY.COMP)


py/coho/__init__.o:		py/coho/__init__.c
	$(CC) $(PY.CFLAGS) -I. -DVERSION='"$(VERSION)"' -o $@ -c $(@:.o=.c)


py/coho/__init__.so:		py/coho/__init__.o \
				libcoho.a
	$(PY.LINK)


smi.o:				smi.c \
				smi.h \
				compat.h \
				util.h
	$(COMP)


$(TEST):			libcoho.a


test/smi:			test/smi.c \
				smi.h
	$(TEST.COMP)


util/vec.o:			util/vec.c \
				util.h \
				compat.h
	$(COMP)


.PHONY:				all \
				doc \
				clean \
				install \
				test \
				wheel


.SUFFIXES:

include ../config.mk

TEST = smiles.t

test: $(TEST)
	@for t in $(TEST); do \
		echo -n "$${t%.t}... "; \
		./$$t >/dev/null 2>&1 || { echo "fail"; exit 1; }; \
		echo ok; \
	done

clean:
	rm -f $(TEST) *.o

.PHONY: clean test

$(TEST:t=o): ../coho.h
$(TEST): ../libcoho.a

.SUFFIXES:
.SUFFIXES: .c .o .t

.c.o:
	$(CC) -I.. $(CFLAGS) -o $@ -c $<

.o.t:
	$(CC) $(LDFLAGS) -o $@ $< ../libcoho.a

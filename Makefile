all:
	gcc test.c -o test

debug:
	gcc test.c -o test.debug -g3

clean:
	rm -f test

.PHONY: all clean
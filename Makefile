fp: fp.c
	gcc -g -O -o fp fp.c -lm

clean:
	rm -f fp

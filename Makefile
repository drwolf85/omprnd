all:
	R CMD SHLIB *.c -o urand.so --preclean

clean:
	rm -f *o

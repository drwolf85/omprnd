all:
	R CMD SHLIB *.c -o ngibbs.so --preclean

clean:
	rm -f *o

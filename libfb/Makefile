all:
	@echo Compiling fb.c
	@gcc -O2 -c fb.c -o libfb.o
	@echo Generating library
	@ar rcs libfb.a libfb.o

clean:
	@echo Removing binaries
	@rm -f libfb.o libfb.a

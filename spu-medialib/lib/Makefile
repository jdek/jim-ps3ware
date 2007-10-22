include ../config.mak

all: lib

lib:
	ar rcs $(LIB) ../spu/cesof/*.o ../src/*.o
	$(CC) -shared  ../spu/cesof/*.o ../src/*.o -o $(LIBSO)
clean:
	rm -rf *.a
	rm -rf *.so

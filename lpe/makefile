all:
	rm -rf bin
	mkdir -p bin/libnss_X
	gcc -o bin/main main.c
	gcc -fPIC -shared -o 'bin/libnss_X/X1234.so.2' shellcode.c
COSMO=cosmocc
CPPFLAGS=-g

all-linux: CXX=g++
all-linux: all

all-multi: CXX=${COSMO}/bin/cosmoc++
all-multi: all

all: main

main:
	mkdir -p bin
	${CXX} ${CPPFLAGS} -o bin/malware src/main.cpp

clean:
	rm -rf ./bin
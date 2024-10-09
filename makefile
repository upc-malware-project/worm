COSMO=../cosmocc
CPPFLAGS=-g
CXX=${COSMO}/bin/cosmoc++

all: main

main:
	mkdir -p bin
	${CXX} ${CPPFLAGS} -o bin/malware src/main.cpp

clean:
	rm -rf ./bin
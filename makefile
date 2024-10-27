COSMOCC=cosmocc
CXX=$(COSMOCC)/bin/x86_64-unknown-cosmo-c++

CPPFLAGS=\
-Os

export BUILDLOG=bin/log.txt
#export MODE=tiny

all: main

main:
	mkdir -p bin
	${CXX} ${CPPFLAGS} \
	-o bin/main.com \
	src/*.cpp \
	src/*.h

clean:
	rm -rf ./bin
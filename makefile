COSMOCC=cosmocc
CXX=$(COSMOCC)/bin/x86_64-unknown-cosmo-c++

CPPFLAGS=-g \
-I${COSMOCC}/library \
-L${COSMOCC}/lib \
-D__x86_64__ \
-DNDEBUG \
-DTINY \

all: BUILDLOG=bin/log.txt
all: main

main:
	mkdir -p bin
	${CXX} ${CPPFLAGS} \
	-o bin/main.com \
	src/*.cpp \
	src/*.h

clean:
	rm -rf ./bin
CC=clang
CXX=clang++
LINK=clang++
LARGS=-rdynamic -pthread -static-libstdc++ -v
CCARGS=-std=c++23 -c -fno-omit-frame-pointer -fno-rtti -fno-exceptions -g
CARGS=-c -fno-omit-frame-pointer -g
WARN=-Werror -Wpedantic -Wall -Wextra -Wno-unused-parameter
OPT=-O0
VERSION=0.0.1
V8_VERSION=12.9
RUNTIME=done
DONE_HOME=$(shell pwd)
ARCH=x64
os=linux
TARGET=${RUNTIME}
V8_FLAGS=-DV8_TYPED_ARRAY_MAX_SIZE_IN_HEAP=0 -DV8_COMPRESS_POINTERS -DV8_INTL_SUPPORT=1 -DENABLE_HUGEPAGE
OUT=./out

.PHONY: help clean cleanall check install 

rebuild: clean done ## removes all build files, and then make clean build

${OUT}:
	mkdir ${OUT}

help:
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z0-9\/_\.-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

v8/include: ## download the v8 headers
	curl -L -o v8-include.tar.gz https://github.com/just-js/v8/releases/download/${V8_VERSION}/include.tar.gz
	tar -xvf v8-include.tar.gz

v8/src: ## download the v8 source code for debugging
	curl -L -o v8-src.tar.gz https://github.com/just-js/v8/releases/download/${V8_VERSION}/src.tar.gz
	tar -xf v8-src.tar.gz

v8/libv8_monolith.a: ## download the v8 static library for linux/macos
	curl -L -o v8/libv8_monolith.a.gz https://github.com/just-js/v8/releases/download/${V8_VERSION}/libv8_monolith-${os}-${ARCH}.a.gz
	gzip -d v8/libv8_monolith.a.gz
	rm -f v8/libv8_monolith.a.gz

main.o: ${OUT} main.cc## compile the main.cc object file
	$(CXX) ${CCARGS} ${OPT} -DRUNTIME='"${RUNTIME}"' -DVERSION='"${VERSION}"' -I./v8 -I./v8/include -I./src ${WARN} ${V8_FLAGS} main.cc -o ${OUT}/main.o

console.o: ${OUT} src/console.cc## compile the console.cc object file
	$(CXX) ${CCARGS} ${OPT} -DRUNTIME='"${RUNTIME}"' -DVERSION='"${VERSION}"' -I./v8 -I./v8/include -I./src ${WARN} ${V8_FLAGS} src/console.cc -o ${OUT}/console.o

syscall-wrapper.o: ${OUT} src/syscall-wrapper.cc## compile the syscall-wrapper.cc object file
	$(CXX) ${CCARGS} ${OPT} -DRUNTIME='"${RUNTIME}"' -DVERSION='"${VERSION}"' -I./v8 -I./v8/include -I./src ${WARN} ${V8_FLAGS} src/syscall-wrapper.cc -o ${OUT}/syscall-wrapper.o

fetch.o: ${OUT} src/fetch.cc## compile the fetch.cc object file
	$(CXX) ${CCARGS} ${OPT} -DRUNTIME='"${RUNTIME}"' -DVERSION='"${VERSION}"' -I./v8 -I./v8/include -I./src ${WARN} ${V8_FLAGS} src/fetch.cc -o ${OUT}/fetch.o

process.o: ${OUT} src/process.cc## compile the process.cc object file
	$(CXX) ${CCARGS} ${OPT} -DRUNTIME='"${RUNTIME}"' -DVERSION='"${VERSION}"' -I./v8 -I./v8/include -I./src ${WARN} ${V8_FLAGS} src/process.cc -o ${OUT}/process.o

text-decoder.o: ${OUT} src/text-decoder.cc## compile the text-decoder.cc object file
	$(CXX) ${CCARGS} ${OPT} -DRUNTIME='"${RUNTIME}"' -DVERSION='"${VERSION}"' -I./v8 -I./v8/include -I./src ${WARN} ${V8_FLAGS} src/text-decoder.cc -o ${OUT}/text-decoder.o

module.o: ${OUT} src/module.cc## compile the module.cc object file
	$(CXX) ${CCARGS} ${OPT} -DRUNTIME='"${RUNTIME}"' -DVERSION='"${VERSION}"' -I./v8 -I./v8/include -I./src ${WARN} ${V8_FLAGS} src/module.cc -o ${OUT}/module.o


${RUNTIME}.o: ${OUT} ## compile runtime into an object file
	$(CXX) ${CCARGS} ${OPT} -DRUNTIME='"${RUNTIME}"' -DVERSION='"${VERSION}"' ${V8_FLAGS} -I./v8 -I./v8/include -I./src ${WARN} ${RUNTIME}.cc -o ${OUT}/${RUNTIME}.o

${RUNTIME}: ${OUT} v8/include v8/libv8_monolith.a main.o ${RUNTIME}.o console.o syscall-wrapper.o fetch.o process.o text-decoder.o module.o ## link the runtime for linux/macos
	@echo building ${RUNTIME} for ${os} on ${ARCH}
	$(LINK) $(LARGS) ${OPT} ${OUT}/main.o ${OUT}/${RUNTIME}.o ${OUT}/console.o ${OUT}/syscall-wrapper.o ${OUT}/fetch.o ${OUT}/process.o ${OUT}/text-decoder.o ${OUT}/module.o  -o ${OUT}/${TARGET} -L"./v8" -lv8_monolith -L"./src" ${LIB_DIRS}

install:
	mkdir -p ${HOME}/.huf/bin
	cp huf ${HOME}/.huf/bin/

clean:
	rm -f ./out/*

cleanall:
	$(MAKE) clean
	rm -fr v8


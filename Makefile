CXX=clang++
CC=clang

#LLVM=/opt/homebrew/opt/llvm/
#CXX=${LLVM}/bin/clang++
#CC=${LLVM}/bin/clang

#GCC=/usr/local
#CXX=${GCC}/bin/g++-11
#CC=${GCC}/bin/gcc-11
#NLOPT=/opt/nlopt/2.70-gcc
BREWPATH=/opt/homebrew
SHAPATH=./include/sha256
SSLPATH=$(BREWPATH)/opt/openssl
CFLAGS=-O3
ifeq ($(BUILD), DYNAMIC)
	RESTPATH=$(BREWPATH)/opt/cpprestsdk
	CFLAGS+= -I$(BREWPATH)/include \
		-I$(RESTPATH)/include \
		-I$(SSLPATH)/include	
	LINKS=-lm \
		-framework CoreFoundation \
		-framework IOKit \
		-L$(RESTPATH)/lib -lcpprest \
		-L$(BREWPATH)/lib \
		-L$(SSLPATH)/lib -lssl -lcrypto	
endif
ifeq ($(BUILD), STATIC)
	RESTPATH=/opt/cpprestsdk
	CFLAGS+= -I$(RESTSDK)/include \
		-I$(SSLPATH)/include \
		-I$(BREWPATH)/include		
	LINKS=-lm -lz\
		-framework CoreFoundation \
		-framework IOKit \
		-framework Security \
		$(RESTPATH)/lib/libcpprest.a \
		$(SSLPATH)/lib/libssl.a \
		$(SSLPATH)/lib/libcrypto.a
endif
CXXFLAGS=-std=c++17 -stdlib=libc++
OBJS= main.o 

.SUFFIXES: .c .cpp .o

PROGRAM=validate_license

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CXX) $(OBJS) $(SHAPATH)/sha256.o -o $(PROGRAM) $(CXXFLAGS) $(CFLAGS) $(LINKS)

.cxx.o:
	$(CXX) -c $< $(CXXFLAGS) $(CFLAGS) -o $@

.cpp.o:
	$(CXX) -c $< $(CXXFLAGS) $(CFLAGS) -o $@

.c.o:
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -f *.o *.out

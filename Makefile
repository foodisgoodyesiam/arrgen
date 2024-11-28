
CFLAGS ?= -O2 -flto=auto
CFLAGS := $(CFLAGS) -MMD

.PHONY: clean

all: arrgen
clean:
	rm -rf arrgen *.o

arrgen: arrgen.o

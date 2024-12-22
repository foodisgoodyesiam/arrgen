# Copyright © 2024 Steven Marion <steven@dragons.fish>
#
# This file is part of arrgen.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the gnu general public license as published by
# the free software foundation, either version 3 of the license, or
# (at your option) any later version.
#
# This file is distributed in the hope that it will be useful,
# but without any warranty; without even the implied warranty of
# merchantability or fitness for a particular purpose.  see the
# gnu general public license for more details.
#
# you should have received a copy of the gnu general public license
# along with this file.  if not, see <https://www.gnu.org/licenses/>.

# extra setting variables:
# DEBUG:
#   0 for production
#   1 for debug (with sanitize and debug symbols)
#   2 for verbose debug (extra logging statements)
#   default is 0
# lto:
#   0 for no link-time optimization
#   1 for link-time optimization
#   2 for whole-program optimization (only supported for gcc)
#   default is 2
# optflags:
#   optimization level added to CFLAGS and CXXFLAGS
#   default is -O2 for no debug and -Og otherwise. you can add stuff like -march=native.
# prefix:
#   directory to install arrgen for the install target
#   default is /usr/local/bin

#always first run this, before any dependencies are checked
#TODO: check if this works on non-GNU make
ignoreme ::= $(shell ./createversionmessage.sh)

DEBUG ?= 0
ifeq ($(DEBUG),0)
	lto ?= 2
else
	lto ?= 0
endif

prefix ?= /usr/local/bin
ifeq ($(DEBUG),0)
	optflags ?= -O3
	CFLAGS ?= $(optflags) -Werror=format -Werror=implicit-function-declaration -fno-exceptions
	CXXFLAGS ?= $(optflags) -Werror=format
	CPPFLAGS ?= -DNDEBUG
else
	optflags ?= -Og
	CFLAGS ?= -fsanitize=address,undefined,leak -g -fno-exceptions
	CXXFLAGS ?= -fsanitize=address,undefined,leak -g
	ifeq ($(DEBUG),2)
		CPPFLAGS ::= $(CPPFLAGS) -DVERBOSE_DEBUG
	endif
endif
ifneq ($(lto),0)
	optflags := $(optflags) -flto=auto
endif
LDFLAGS ?= $(CFLAGS)
CFLAGS := $(optflags) $(CFLAGS) -MMD
CXXFLAGS := $(optflags) $(CXXFLAGS) -MMD
LDFLAGS := $(optflags) $(LDFLAGS)
ifeq ($(lto),2)
	LDFLAGS := $(LDFLAGS) -fwhole-program
endif

.PHONY: clean install

all: arrgen
clean:
	rm -rf src/*.o \
		src/*.d \
		gen_src/*.* \
		arrgen

arrgen: src/arrgen.o \
	src/errors.o \
	src/handlefile.o \
	src/pagesize.o \
	src/c_string_stuff.o \
	src/parameters.o \
	gen_src/parameter_lookup.o \
	src/writearray.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LOADLIBES) $(LDLIBS)

install: $(prefix)/arrgen

$(prefix)/arrgen: arrgen
	install -m 755 $< $@

src/version_message.h: gen_src/build_version_message.h

gen_src/parameter_lookup.c: src/parameter_lookup.gperf
	gperf -m 100 $< >$@

gen_src/build_version_message.h:
	./createversionmessage.sh

-include $(wildcard src/*.d) $(wildcard gen_src/*.d)

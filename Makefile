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

# 0 for production, 1 for debug (with sanitize and debug symbols), 2 for verbose debug (extra logging statements)
DEBUG ?= 0

prefix ?= /usr/local/bin
ifeq ($(DEBUG),0)
	CFLAGS ?= -O2 -Werror=format -Werror=implicit-function-declaration
	CXXFLAGS ?= -O2 -Werror=format
	CPPFLAGS ?= -DNDEBUG
else
	CFLAGS ?= -Og -fsanitize=address,undefined,leak -g
	ifeq ($(DEBUG),2)
		CPPFLAGS ::= $(CPPFLAGS) -DVERBOSE_DEBUG
	endif
endif
LDFLAGS ?= $(CFLAGS)
CFLAGS := $(CFLAGS) -MMD
CXXFLAGS := $(CXXFLAGS) -MMD

.PHONY: clean install

all: arrgen
clean:
	rm -rf arrgen *.o *.d

arrgen: arrgen.o \
	errors.o \
	handlefile.o \
	pagesize.o \
	writearray.o

install: $(prefix)/arrgen

$(prefix)/arrgen: arrgen
	install -m 755 $< $@

-include $(wildcard *.d)

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

CFLAGS ?= -O2 -Werror=format
CFLAGS := $(CFLAGS) -MMD
LDFLAGS ?= $(CFLAGS)
prefix ?= /usr/local/bin

.PHONY: clean install

all: arrgen
clean:
	rm -rf arrgen *.o *.d

arrgen: arrgen.o \
	errors.o \
	writearray.o

install: $(prefix)/arrgen

$(prefix)/arrgen: arrgen
	install -m 755 $< $@

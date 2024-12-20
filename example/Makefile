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

example1: example1.o gen_arrays_test.o

#this shold tell it that the header only exists once the c file exists
gen_arrays.h: gen_arrays_test.c

gen_arrays_test.c: example.arrgen ../arrgen.c small_image.png
	../arrgen -f example.arrgen

#you need to tell it that the header won't exist yet
example1.c: gen_arrays.h

.PHONY: clean
clean:
	rm -f example1 \
	gen_arrays_test.c \
	gen_arrays.h \
	*.o


#!/usr/bin/env python3

# made this to check the speed of fortran compilation. it's much slower than C for the fortran compilers I tried, so I won't add fortran support to the code
# it also would be pointless, because the point of this program is to be universally portable... fortran is less portable than C, and architecture- and os- dependent alternatives are available from binutils, etc that are faster than the C compilation

modulename="arrgen_arrays_mod"
lengthname="ARRGEN_ALLN_TXT_LENGTH"
arrayname="ARRGEN_ALLN_TXT"
indent="        "
usesuffix=True
maxlinelength=120

import os, sys

def fatal(msg):
	print("%s: %s" % (sys.argv[0], msg), file=sys.stderr)
	exit(1)

def writeBytes(num: int) -> str:
	if usesuffix:
		if num==-128:
			return "UM"
		elif num>127:
			return "%d_1" % (num-128)
		else:
			return "%d_1" % num
	else:
		if num>127:
			return "%d" % (num-128)
		else:
			return "%d" % num

if len(sys.argv)<2:
	fatal("you forgot to give me a file")

filepath = sys.argv[1]
total_size = os.path.getsize(filepath)
linelimit=maxlinelength-len("&")

print("module %s" % modulename)
print("    use, intrinsic :: iso_c_binding, only: c_int8_t, c_size_t")
print("    implicit none")
print("")
if usesuffix:
	print("    integer(kind=c_int8_t), parameter :: UM = -128")
print("    integer(kind=c_size_t), parameter :: %s = %d" % (lengthname, total_size))
print("    integer(kind=c_int8_t), dimension(%s), &" % lengthname)
print("        bind(c, name=\"%s\")&" % lengthname)
print("        :: %s = [&" % arrayname)
with open(filepath, 'rb') as file:
	line = "%s%s" % (indent, writeBytes(file.read(1)[0]))
	byte_stream = (byte for byte in file.read())
	for byte in byte_stream:
		toadd = "," + writeBytes(byte)
		if len(line)+len(toadd)+1>=maxlinelength:
			print(line + "&")
			line=indent
		line = line+toadd
	print(line + "]")
print("end module %s" % modulename)

# ########################################################################
#
#                   Makefile for Utility Programs
#
#   Rules:
#
#       all     -- builds program.
#       clean   -- remove object and other work files.
#
#       If no arguments are givin, the program is built by default.
#


# ########################################################################
# Installation Options:
#
#   You may modify any value as needed. Change only the ones you are
#   absolutly sure that requires modification.
#
PREFIX = /usr


# ########################################################################
# Compiler Flags:
#
#   These are definations to enable or disable certain compile time
#   options. Omitting a defination turns that option off.
#
#   Each argument is of the format -D<option> where <option> is
#   one of the following:
#
#	* None available *
#
#   Other arguments include:
#
#       -O#                     Specifies the optimization level the
#                               compiler is to compile at. This (attempts)
#                               to improve the efficiency of the generated
#                               program when it runs. Available values for
#                               # are from 0 to 2 (some compilers allow
#                               higher values). When in doubt, set # to 2.
#
#       -g                      Compile with debugging information,
#                               this is useful for determining why this
#                               program (if it did) crash. However this
#                               may hinder performance, so don't use
#                               this option unless you are attempting
#                               to debug the program.
#
CFLAGS  = -D__USE_BSD -Dc_plusplus -D__cplusplus -Dc_plusplus -D__cplusplus -O2


# ########################################################################
# Dependant Libraries:
#
#   These are dynamic (sometimes called shared) libraries that this
#   program is to be `linked' to.
#
#   Each argument is of the format -l<name> where <name> is the name
#   of the library. You may have to add one or more -l<name> arguments
#   to the LIB line depending on what you have set in the CFLAGS line
#   farther above.
#
LIB = -lY2

# Library Directories:
#
#   All libraries are looked for in the directories specified below.
#
#   Each argument is of the format -L<dir> where <dir> is the full
#   path to the directory.
#
LIB_DIR =

# Header File Directories:
#
#   Required header files that are not in the standard locations are
#   searched for in the directories specified below.
#
#   Each argument is of the format -I<dir> where <dir> is the full
#   path to the directory.
#
INC =


# ########################################################################
# Program Source and Header Files:
#
CPP     = g++
PROGS  = btoheader unvtocht tgacrop


# ########################################################################
# Build rules:
#

all: $(PROGS)

btoheader:
	$(CPP) $(CFLAGS) ctype.cpp fio.cpp btoheader.cpp -o btoheader

unvtocht:
	$(CPP) $(CFLAGS) ctype.cpp unvtocht.cpp unvfile.cpp unvmain.cpp unvmath.cpp \
	unvutil.cpp unvmatch.cpp fio.cpp string.cpp disk.cpp -o unvtocht

tgacrop:
	$(CPP) $(CFLAGS) ctype.cpp fio.cpp tgacrop.cpp tga.cpp tgadither.cpp -o tgacrop


# ########################################################################
# Maintainance and Misc Rules:
#
clean:
	rm -f a.out core *.o $(PROGS)


# ########################################################################

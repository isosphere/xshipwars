# ########################################################################
#
#                   Makefile for Y Utility Programs
#
#   Rules:
#
#       all     -- builds program.
#       install -- install program.
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
PREFIX = /home/swserv


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
#       -shared                 Compile this program as a shared library.
#
CFLAGS  = -O2 -funroll-loops -shared -Dc_plusplus -D__cplusplus


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
LIB =

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
CPP = g++

PLUGINS = hellouniverse moveobj timming


# ########################################################################
# Build rules:
#

all: $(PLUGINS)

hellouniverse:
	$(CPP) $(CFLAGS) $(LIB_DIR) $(LIB) hellouniverse.cpp -o hellouniverse

moveobj:
	$(CPP) $(CFLAGS) $(LIB_DIR) $(LIB) moveobj.cpp -o moveobj

timming:
	$(CPP) $(CFLAGS) $(LIB_DIR) $(LIB) timming.cpp -o timming


# ########################################################################
# Install Rules:
#
INSTALL      = install
INSTBINFLAGS = -m 0755
INSTUIDFLAGS = -m 4755
INSTLIBFLAGS = -m 0755
INSTINCFLAGS = -m 0444
INSTMANFLAGS = -m 0444
INSTDATFLAGS = -m 0444

COPY      = cp
COPYFLAGS = -i -v

MKDIR      = mkdir
MKDIRFLAGS = -p

PLUGINS_DIR = $(PREFIX)/plugins

install:
	$(MKDIR) $(MKDIRFLAGS) $(PLUGINS_DIR)
	$(INSTALL) $(INSTBINFLAGS) hellouniverse $(PLUGINS_DIR)
	$(INSTALL) $(INSTBINFLAGS) moveobj $(PLUGINS_DIR)
	$(INSTALL) $(INSTBINFLAGS) timming $(PLUGINS_DIR)
	@echo "---------------------------------------------------------------"
	@echo "Plugins have been installed in:"
	@echo " "
	@echo "        $(PLUGINS_DIR)"
	@echo " "


# ########################################################################
# Maintainance and Misc Rules:
#
clean:
	rm -f a.out core *.o $(PLUGINS)


# ########################################################################

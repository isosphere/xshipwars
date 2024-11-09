# Platform Configurator generated Makefile
#
PREFIX = /usr/

CFLAGS = -Wall -O6 -funroll-loops -fomit-frame-pointer -finline-functions -ffast-math -D__USE_BSD -DUSE_XSHM -DHAVE_MWMUTIL_H -DHAVE_LIBXPM

INC_DIRS =

LIBS = -lm -lX11 -lXext -lXpm

LIB_DIRS =  -L/usr/X11R6/lib/ -L/usr/X11R6/lib/

CC = cc

CPP = c++

# Appended stuff starts here
#

# C++ flags:
CPPFLAGS = -D__cplusplus -Dc_plusplus

# Source files list:
include Makefile.srclist

# Compiler set up:
BIN     = xsw
OBJ_C   = $(SRC_C:.c=.o)
OBJ = $(SRC:.cpp=.o)
.cpp.o:
	$(CPP) -c $*.cpp $(INC_DIRS) $(CFLAGS) $(CPPFLAGS)

# Build rules:
$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN) $(LIBS) $(LIB_DIRS)

all: $(BIN)


# Install Rules:
include Makefile.install.UNIX


# Maintainance and Misc Rules:
clean:
	rm -f a.out core *.o $(BIN)

# Platform Configurator generated Makefile
#
PREFIX = /home/

CFLAGS = -Wall -O6 -funroll-loops -fomit-frame-pointer -finline-functions -ffast-math -D__USE_BSD -DPLUGIN_SUPPORT

INC_DIRS = 

LIBS = -lm -lcrypt -ldl

LIB_DIRS = 

CC = cc

CPP = c++

# Appended stuff starts here
#

# C++ flags:
CPPFLAGS = -D__cplusplus -Dc_plusplus

# Source files list:
include Makefile.srclist

# Compiler set up:
BIN     = swserv
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

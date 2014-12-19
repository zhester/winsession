##############################################################################
#
#  Makefile
#
#  Builds a Win32 program using 64-bit MinGW in a Cygwin environment.
#
#  So far, only used with GNU make under Cygwin.
#
##############################################################################

# Default base name of the project
PROJ := winsession

# Application details
IMAGE_NAME := $(PROJ).exe

# Basic compile environment settings
BINPF   := /usr/bin
CC      := $(BINPF)/i686-w64-mingw32-gcc.exe
CFLAGS   = -Wall -static -mwindows -DWIN32_LEAN_AND_MEAN
LD      := $(CC)
LDFLAGS  = -Wall -static -mwindows -s
WR      := $(BINPF)/i686-w64-mingw32-windres.exe
WRFLAGS := -O coff
SHELL   := $(BINPF)/sh

# Add debug symbols for any debug target.
debug: CFLAGS += -ggdb
debug: LDFLAGS += -ggdb

# Various important paths.
PROJDIR := ../..
INCDIR  := $(PROJDIR)/include
SRCDIR  := $(PROJDIR)/src
TOOLDIR := $(PROJDIR)/tools
BLDDIR  := build

# Project source files
SOURCES := $(wildcard $(SRCDIR)/*.c)

# Derive object targets from source files
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(BLDDIR)/%.o, $(SOURCES))

# Windows program resource information
RESOURCE_SCRIPT := $(BLDDIR)/$(PROJ).rc
RESOURCE_OUTPUT := $(BLDDIR)/$(PROJ).res

# Default target
all: $(BLDDIR)/$(IMAGE_NAME)

# How to build the project binary
$(BLDDIR)/$(IMAGE_NAME): $(OBJECTS) $(RESOURCE_OUTPUT)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(RESOURCE_OUTPUT) && chmod 700 $@

# How to build the resource information
$(RESOURCE_OUTPUT): $(RESOURCE_SCRIPT)
	$(WR) $(WRFLAGS) -o $@ $<

# How to build the resource script
#   Note: This uses the default template stored inside makerc.py.
$(RESOURCE_SCRIPT): $(TOOLDIR)/makerc.py | $(BLDDIR)
	$(TOOLDIR)/makerc.py -p $(PROJ) $(RESOURCE_SCRIPT)

# How to build the project's object files
$(BLDDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/*.h | $(BLDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ -c $<

# Make sure there's an output directory.
$(BLDDIR):
	mkdir -p $(BLDDIR)

# How to clean the output files
clean:
	rm -rf $(BLDDIR)


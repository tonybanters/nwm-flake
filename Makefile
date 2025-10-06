CXX    := g++
CXFLAG := -std=c++14 -O2 -Werror -Wall -Wpedantic
SRC    := src/nwm.cpp src/util.cpp

IX11   := -I/usr/include/freetype2 
LX11   := -lX11 -lXft -lfreetype -lfontconfig

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

.PHONY: default install clean help

default:
	$(CXX) $(CXFLAG) $(IX11) $(SRC) -o nwm $(LX11)

install: default
	mkdir -p $(PREFIX)
	mkdir -p $(BINDIR)
	install -Dm755 nwm $(BINDIR)/$(BIN)
	@echo "Installed nwm to $(BINDIR)"

clean:
	rm -rf nwm $(BINDIR)/nwm

help:
	@echo "Run 'make' to build the project"
	@echo "Run 'make install' to install to $(BINDIR)"
	@echo "Run 'make clean' to remove binary"

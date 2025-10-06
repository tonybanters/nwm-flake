CXX    := g++
CXFLAG := -std=c++14 -Werror -Wall -Wpedantic -O2 
SRC    := src/nwm.cpp src/util.cpp

IX11   := -I/usr/include/freetype2 
LX11   := -lX11 -lXft -lfreetype -lfontconfig

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

.PHONY: default build install clean help

default: build

build:
	mkdir -p $(PREFIX)
	mkdir -p $(BINDIR)
	$(CXX) $(CXFLAG) $(IX11) $(SRC) -o $(BINDIR)/nwm $(LX11)

install: build
	install -Dm755 $(BIN) $(BINDIR)/$(BIN)
	@echo "Installed $(BIN) to $(BINDIR)"

clean:
	rm -rf $(BIN)

help:
	@echo "Run 'make build' to build the project"
	@echo "Run 'make install' to install to $(BINDIR)"
	@echo "Run 'make clean' to remove binary"

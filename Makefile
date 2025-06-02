CC=g++
CFLAGS=-Werror -Wall 
SRC=src/*.cpp
BIN=nwm
IX11=-I/usr/include/X11
LX11=-lX11
THIRDPARTY=-Ithirdparty/
PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
HELP=@echo "Run make start to build the project"

.PHONY: default build install clean help

default:
	$(CC) $(CFLAGS) $(IX11) $(SRC) $(LX11) -o $(BIN)

build:
	$(CC) $(CFLAGS) $(THIRDPARTY) $(IX11) $(SRC) $(LX11) -o $(BIN)

install: build
	install -Dm755 $(BIN) $(BINDIR)/$(BIN)
	@echo "Installed $(BIN) to $(BINDIR)"

clean:
	rm -rf $(BIN)

help:
	$(HELP)


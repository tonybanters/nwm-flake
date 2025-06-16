CC=g++
CFLAGS=-Werror -Wall
SRC=src/*.cpp
BIN=nwm
ILUA=-I/usr/include/lua5.4
LLUA=-llua -ldl -lm
IX11=-I/usr/include/X11
LX11=-lX11
THIRDPARTY=-Ithirdparty/
PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
HELP=@echo "Run make build to build the project"

.PHONY: default build install clean help

default:
	$(CC) $(CFLAGS) $(ILUA) $(IX11) $(SRC) $(LLUA) $(LX11) -o $(BIN)
build:
	$(CC) $(CFLAGS) $(THIRDPARTY) $(ILUA) $(IX11) $(SRC) $(LLUA) $(LX11) -o $(BIN)
install: build
	install -Dm755 $(BIN) $(BINDIR)/$(BIN)
	@echo "Installed $(BIN) to $(BINDIR)"
clean:
	rm -rf $(BIN)
help:
	$(HELP)


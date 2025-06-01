CC=g++
CFLAGS=-Werror -Wall 
SRC=src/*.cpp
BIN=nwm
IX11=-I/usr/include/X11
LX11=-lX11
THIRDPARTY=-Ithirdparty/
HELP=@echo "Run make start to build the project"

.phony: default clean
default:
	$(CC) $(CFLAGS)$(IX11) $(SRC) $(LX11) -o $(BIN)
build:
	$(CC) $(CFLAGS) $(THIRDPARTY) $(IX11) $(SRC) $(LX11) -o $(BIN)
help:
	$(HELP)
clean:
	rm -rf $(BIN)

CXXFLAG = -std=c++14 -O3 -Wall -Wextra -Wpedantic -Werror
SRC    = src/nwm.cpp src/util.cpp src/bar.cpp

IX11   = -I/usr/include/freetype2 
LX11   = -lX11 -lXft -lfreetype -lfontconfig -lXrender -lm

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

.PHONY: install clean help

nwm:
	$(CXX) $(CXXFLAG) $(IX11) $(SRC) -o nwm $(LX11)

install: nwm
	mkdir -p $(PREFIX)
	mkdir -p $(BINDIR)
	install -Dm755 nwm $(BINDIR)/nwm
	@echo "Installed nwm to $(BINDIR)"
	@cp picom.conf $(HOME)/.config/
	@echo "Installed picom.conf to $(HOME)/.config"

clean:
	rm -rf nwm $(BINDIR)/nwm

help:
	@echo "Run 'make' to build the project"
	@echo "Run 'make install' to install to $(BINDIR)"
	@echo "Run 'make clean' to remove binary"

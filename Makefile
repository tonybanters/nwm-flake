CXX    = g++
CXXFLAG = -std=c++14 -O3 -Wall -Wextra -Wpedantic -Werror

SRC    = src/nwm.cpp src/bar.cpp src/tiling.cpp
OBJ    = src/nwm.o src/bar.o src/tiling.o
DEPS   = src/nwm.hpp src/bar.hpp src/tiling.hpp src/config.hpp

IX11   = -I/usr/include/freetype2 
LX11   = -lX11 -lXft -lfreetype -lfontconfig -lXrender -lm

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
SHAREDIR ?= $(PREFIX)/share
XSESSIONSDIR ?= $(SHAREDIR)/xsessions

.PHONY: all install clean uninstall help

all: nwm

src/%.o: src/%.cpp $(DEPS)
	$(CXX) $(CXXFLAG) $(IX11) -c $< -o $@

nwm: $(OBJ)
	$(CXX) $(CXXFLAG) $(OBJ) -o nwm $(LX11)

install: nwm
	mkdir -p $(BINDIR)
	mkdir -p $(XSESSIONSDIR)
	install -Dm755 nwm $(BINDIR)/nwm
	install -Dm644 nwm.desktop $(XSESSIONSDIR)/nwm.desktop
	@echo "Installed nwm to $(BINDIR)"
	@echo "Installed desktop entry to $(XSESSIONSDIR)"

clean:
	rm -f nwm $(OBJ)

uninstall:
	rm -f $(BINDIR)/nwm
	rm -f $(XSESSIONSDIR)/nwm.desktop
	@echo "Uninstalled nwm"

help:
	@echo "Build commands:"
	@echo "  make           - Build with single thread"
	@echo "  make -j        - Build with all available cores"
	@echo "  make -j8       - Build with 8 parallel jobs"
	@echo ""
	@echo "Install commands:"
	@echo "  make install   - Install to $(BINDIR)"
	@echo "  make uninstall - Remove installation"
	@echo ""
	@echo "Clean commands:"
	@echo "  make clean     - Remove binary and object files"

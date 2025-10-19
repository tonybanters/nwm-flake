CXXFLAGS = -std=c++14 -O3 -Wall -Wextra -Wpedantic -Wstrict-aliasing

SRC      = src/nwm.cpp src/bar.cpp src/tiling.cpp src/systray.cpp
OBJ      = src/nwm.o src/bar.o src/tiling.o src/systray.o
DEPS     = src/nwm.hpp src/bar.hpp src/tiling.hpp src/config.hpp src/systray.hpp

LDFLAGS  = -I/usr/include/freetype2 
LDLIBS   = -lX11 -lXft -lfreetype -lfontconfig -lXrender -lm -lXrandr

PREFIX   ?= /usr/local
BINDIR   ?= $(PREFIX)/bin
XSESSIONSDIR ?= /usr/share/xsessions

.PHONY: all install clean uninstall

all: nwm

src/%.o: src/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

nwm: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o nwm $(LDLIBS)

install: nwm
	mkdir -p $(BINDIR)
	mkdir -p $(XSESSIONSDIR)
	install -Dm755 nwm $(BINDIR)/nwm
	install -Dm644 nwm.desktop $(XSESSIONSDIR)/nwm.desktop
	@echo "Installed nwm to $(BINDIR)"
	@echo "Installed nwm.desktop to $(XSESSIONSDIR)"

clean:
	$(RM) nwm $(OBJ)

uninstall:
	$(RM) $(BINDIR)/nwm
	$(RM) $(XSESSIONSDIR)/nwm.desktop
	@echo "Uninstalled nwm"


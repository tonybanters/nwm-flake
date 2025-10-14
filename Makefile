CXXFLAGS = -std=c++14 -O3 -Wall -Wextra -Wpedantic -Werror -Wstrict-aliasing

SRC      = src/nwm.cpp src/bar.cpp src/tiling.cpp
OBJ      = src/nwm.o src/bar.o src/tiling.o
DEPS     = src/nwm.hpp src/bar.hpp src/tiling.hpp src/config.hpp

LDFLAGS  = -I/usr/include/freetype2 
LDLIBS   = -lX11 -lXft -lfreetype -lfontconfig -lXrender -lm

PREFIX   ?= /usr/local
BINDIR   ?= $(PREFIX)/bin

.PHONY: all install clean uninstall

all: nwm

src/%.o: src/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

nwm: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o nwm $(LDLIBS)

install: nwm
	mkdir -p $(BINDIR)
	install -Dm755 nwm $(BINDIR)/nwm
	@echo "Installed nwm to $(BINDIR)"

clean:
	$(RM) nwm $(OBJ)

uninstall:
	$(RM) $(BINDIR)/nwm
	$(RM) $(XSESSIONSDIR)/nwm.desktop
	@echo "Uninstalled nwm"


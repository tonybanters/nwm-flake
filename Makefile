CXXFLAG = -std=c++14 -O3 -Wall -Wextra -Wpedantic -Werror

SRC    = src/nwm.cpp src/bar.cpp src/tiling.cpp
OBJ    = src/nwm.o src/bar.o src/tiling.o
DEPS   = src/nwm.hpp src/bar.hpp src/tiling.hpp src/config.hpp

IX11   = -I/usr/include/freetype2 
LX11   = -lX11 -lXft -lfreetype -lfontconfig -lXrender -lm

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

.PHONY: all install clean uninstall

all: nwm

src/%.o: src/%.cpp $(DEPS)
	$(CXX) $(CXXFLAG) $(IX11) -c $< -o $@

nwm: $(OBJ)
	$(CXX) $(CXXFLAG) $(OBJ) -o nwm $(LX11)

install: nwm
	mkdir -p $(BINDIR)
	install -Dm755 nwm $(BINDIR)/nwm
	@echo "Installed nwm to $(BINDIR)"

clean:
	rm -f nwm $(OBJ)

uninstall:
	rm -f $(BINDIR)/nwm
	rm -f $(XSESSIONSDIR)/nwm.desktop
	@echo "Uninstalled nwm"


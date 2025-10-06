{
  description = "Suckless software builds";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        
        # Helper function to build suckless software
        buildSuckless = { pname, version, src, extraBuildInputs ? [], extraNativeBuildInputs ? [], meta ? {} }: 
          pkgs.stdenv.mkDerivation {
            inherit pname version src;
            
            buildInputs = with pkgs; [
              xorg.libX11
              xorg.libXft
              xorg.libXinerama
              fontconfig
              freetype
            ] ++ extraBuildInputs;
            
            nativeBuildInputs = with pkgs; [
              pkg-config
            ] ++ extraNativeBuildInputs;
            
            prePatch = ''
              sed -i "s@/usr/local@$out@" config.mk
            '';
            
            # Set HOME for tic command (needed for st terminfo compilation)
            preBuild = ''
              export HOME=$TMPDIR
            '';
            
            makeFlags = [
              "PREFIX=$(out)"
              "CC=${pkgs.stdenv.cc.targetPrefix}cc"
            ];
            
            installPhase = ''
              make PREFIX=$out install
            '';
            
            meta = with pkgs.lib; meta // {
              homepage = meta.homepage or "https://suckless.org";
              license = licenses.mit;
              platforms = platforms.linux;
            };
          };
        
        # Build dmenu
        dmenu-custom = buildSuckless {
          pname = "dmenu-custom";
          version = "5.4";
          src = ./dmenu;
          meta.description = "Dynamic menu for X (customized with center patch)";
        };
        
        # Build st (simple terminal)
        st-custom = buildSuckless {
          pname = "st-custom";
          version = "0.9.2";
          src = ./st;
          extraBuildInputs = with pkgs; [ imlib2 ncurses ];
          meta.description = "Simple terminal implementation for X";
        };
        
        # Build dwm (dynamic window manager)
        dwm-custom = buildSuckless {
          pname = "dwm-custom";
          version = "6.5";
          src = ./dwm;
          meta.description = "Dynamic window manager for X";
        };
        
      in
      {
        packages = {
          dmenu = dmenu-custom;
          st = st-custom;
          dwm = dwm-custom;
          default = dmenu-custom;
          
          # Meta-package that builds everything
          all = pkgs.symlinkJoin {
            name = "suckless-all";
            paths = [ dmenu-custom st-custom dwm-custom ];
          };
        };
        
        devShells.default = pkgs.mkShell {
          name = "suckless-dev";
          
          packages = with pkgs; [
            # Build tools
            pkg-config
            gcc
            gnumake
            git
            
            # X11 libraries
            xorg.libX11
            xorg.libXft
            xorg.libXinerama
            
            # Font libraries
            fontconfig
            freetype
            harfbuzz
            
            # Fonts
            nerd-fonts.iosevka
            
            # Image libraries
            imlib2
            
            # Debugging and development
            gdb
            valgrind
            
            # Patch management
            patchutils
            diffutils
          ];
          
          shellHook = ''
            echo "Suckless development environment"
            echo "================================"
            echo ""
            echo "Available projects:"
            echo "  - dmenu/"
            echo "  - st/"
            echo "  - dwm/"
            echo ""
            echo "Build individually:"
            echo "  nix build .#dmenu"
            echo "  nix build .#st"
            echo "  nix build .#dwm"
            echo ""
            echo "Build all at once:"
            echo "  nix build .#all"
            echo ""
          '';
          
          # Set up environment variables
          XINERAMA = "1";
          FREETYPEINC = "${pkgs.freetype.dev}/include/freetype2";
          X11INC = "${pkgs.xorg.libX11.dev}/include";
          X11LIB = "${pkgs.xorg.libX11}/lib";
        };
        
        # Alternative dev shell with more minimal setup
        devShells.minimal = pkgs.mkShell {
          packages = with pkgs; [
            pkg-config
            xorg.libX11
            xorg.libXft
            xorg.libXinerama
            fontconfig
            freetype
            gcc
            gnumake
          ];
        };
      }
    );
}

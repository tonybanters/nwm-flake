{
  description = "NWM - A scrollable tiling window manager";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        
        nwm = pkgs.stdenv.mkDerivation {
          pname = "nwm";
          version = "0.1.0";
          src = ./.;
          
          buildInputs = with pkgs; [
            xorg.libX11
            xorg.libXft
            xorg.libXrender
            fontconfig
            freetype
          ];
          
          nativeBuildInputs = with pkgs; [
            pkg-config
          ];
          
          makeFlags = [
            "PREFIX=$(out)"
            "CC=${pkgs.stdenv.cc.targetPrefix}cc"
          ];
          
          installPhase = ''
            mkdir -p $out/bin
            install -Dm755 nwm $out/bin/nwm
          '';
          
          meta = with pkgs.lib; {
            description = "A scrollable tiling window manager for X11";
            homepage = "https://github.com/xsoder/nwm";
            license = licenses.mit;
            platforms = platforms.linux;
            maintainers = [ ];
          };
        };
        
      in
      {
        packages = {
          default = nwm;
          nwm = nwm;
        };
        
        devShells.default = pkgs.mkShell {
          name = "nwm-dev";
          
          packages = with pkgs; [
            # Build tools
            pkg-config
            gcc
            gnumake
            git
            # X11 libraries
            xorg.libX11
            xorg.libXft
            xorg.libXrender
            # Font libraries
            fontconfig
            freetype
            # Fonts
            nerd-fonts.iosevka
            # Debugging and development
            gdb
            valgrind
            # Testing tools
            xorg.xorgserver
            xorg.xinit
            xorg.xev
            xorg.xdpyinfo
            # Utilities
            feh
            st
            dmenu
          ];
          
          XINERAMA = "1";
          FREETYPEINC = "${pkgs.freetype.dev}/include/freetype2";
          X11INC = "${pkgs.xorg.libX11.dev}/include";
          X11LIB = "${pkgs.xorg.libX11}/lib";
        };
        
        # Minimal dev shell
        devShells.minimal = pkgs.mkShell {
          packages = with pkgs; [
            pkg-config
            xorg.libX11
            xorg.libXft
            xorg.libXrender
            fontconfig
            freetype
            gcc
            gnumake
          ];
        };
      }
    );
}

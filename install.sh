#!/bin/bash

set -e

INSTALL_LOG=$(mktemp)

cleanup() {
    rm -f "$INSTALL_LOG"
    tput sgr0
}
trap cleanup EXIT

C_RED='\033[0;31m'
C_GREEN='\033[0;32m'
C_YELLOW='\033[0;33m'
C_BLUE='\033[0;34m'
C_MAGENTA='\033[0;35m'
C_CYAN='\033[0;36m'
C_BOLD='\033[1m'
C_RESET='\033[0m'

show_banner() {
    clear
    echo -e "${C_CYAN}"
    cat << "EOF"
    ███╗   ██╗██╗    ██╗███╗   ███╗
    ████╗  ██║██║    ██║████╗ ████║
    ██╔██╗ ██║██║ █╗ ██║██╔████╔██║
    ██║╚██╗██║██║███╗██║██║╚██╔╝██║
    ██║ ╚████║╚███╔███╔╝██║ ╚═╝ ██║
    ╚═╝  ╚═══╝ ╚══╝╚══╝ ╚═╝     ╚═╝
EOF
    echo -e "${C_RESET}"
    echo -ne "${C_BOLD}    N Window Manager Installer${C_RESET}\n\n"
}

detect_package_manager() {
    if command -v pacman &> /dev/null; then
        echo "pacman"
    elif command -v apt &> /dev/null; then
        echo "apt"
    elif command -v dnf &> /dev/null; then
        echo "dnf"
    else
        echo "unknown"
    fi
}

PKG_MGR=$(detect_package_manager)

declare -a PACMAN_DEPS=(
    "base-devel" "xorg-server" "xorg-xinit" "libx11" "libxft"
    "fontconfig" "freetype2" "ttf-dejavu" "picom" "feh" "dmenu"
)

declare -a APT_DEPS=(
    "build-essential" "xorg" "libx11-dev" "libxft-dev"
    "libfontconfig1-dev" "libfreetype6-dev" "fonts-dejavu"
    "picom" "feh" "dmenu"
)

declare -a DNF_DEPS=(
    "gcc" "gcc-c++" "make" "xorg-x11-server-Xorg" "libX11-devel"
    "libXft-devel" "fontconfig-devel" "freetype-devel"
    "dejavu-fonts" "picom" "feh" "dmenu"
)

install_all() {
    local deps=()
    case $PKG_MGR in
        pacman) deps=("${PACMAN_DEPS[@]}") ;;
        apt) deps=("${APT_DEPS[@]}") ;;
        dnf) deps=("${DNF_DEPS[@]}") ;;
    esac

    echo -e "${C_YELLOW}Installing dependencies...${C_RESET}"

    case $PKG_MGR in
        pacman)
            sudo pacman -S --needed "${deps[@]}" || true
            ;;
        apt)
            sudo apt install -y "${deps[@]}" || true
            ;;
        dnf)
            sudo dnf install -y "${deps[@]}" || true
            ;;
    esac

    echo -e "${C_GREEN}Dependencies installed${C_RESET}"
}

build_nwm() {
    echo -e "${C_YELLOW}Building NWM...${C_RESET}"
    make clean >> "$INSTALL_LOG" 2>&1
    if ! make -j$(nproc) >> "$INSTALL_LOG" 2>&1; then
        echo -e "${C_RED}Build failed!${C_RESET}"
        cat "$INSTALL_LOG"
        exit 1
    fi
    sudo cp nwm.desktop /usr/share/xsessions
    echo -e "${C_GREEN}Build complete${C_RESET}"
    echo -e "${C_YELLOW}Installing system-wide...${C_RESET}"
    if ! sudo make install >> "$INSTALL_LOG" 2>&1; then
        echo -e "${C_RED}Install failed!${C_RESET}"
        cat "$INSTALL_LOG"
        exit 1
    fi
    echo -e "${C_GREEN}Installation complete${C_RESET}"
}

main() {
    show_banner

    if [ "$PKG_MGR" = "unknown" ]; then
        echo -e "${C_RED}Error: Unsupported package manager${C_RESET}"
        exit 1
    fi

    echo -e "${C_BOLD}Install NWM?${C_RESET} ${C_CYAN}(y/n)${C_RESET}: "
    read -n1 choice
    echo ""
    echo ""

    if [[ ! "$choice" =~ ^[Yy]$ ]]; then
        echo -e "${C_YELLOW}Cancelled${C_RESET}"
        exit 0
    fi

    install_all
    build_nwm

    echo ""
    echo -e "${C_GREEN}${C_BOLD}Installation Complete!${C_RESET}"
    echo ""
    echo -e "${C_CYAN}Start:${C_RESET} exec nwm ${C_MAGENTA}(in ~/.xinitrc)${C_RESET}"
    echo ""
    echo -e "${C_YELLOW}Keybindings:${C_RESET}"
    echo -e "  ${C_CYAN}Super+Return${C_RESET} - Terminal"
    echo -e "  ${C_CYAN}Super+d${C_RESET}      - dmenu"
    echo -e "  ${C_CYAN}Super+t${C_RESET}      - Toggle layout"
    echo -e "  ${C_CYAN}Super+q${C_RESET}      - Close window"
    echo ""
}

main

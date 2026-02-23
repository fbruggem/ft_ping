let
 pkgs = import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/a84b0a7c509bdbaafbe6fe6e947bdaa98acafb99.tar.gz";
    sha256 = "0m8zrg4rp5mx5v9ar91ncnjhagmcrd3y9h56y48swan6a8gwpq52";
  }) {};
 pkgsInetUtils = import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/2a51117bdd6cd618703ea9cfc34cf3933ac4d4aa.tar.gz";
    # sha256 = "";
  }) {};
  buildDeps = import ./default.nix;
in
pkgs.mkShell {
  name = "dev-shell";

  inputsFrom = [ buildDeps ];

  buildInputs = with pkgs; [
    clang-tools
    man-db
    man-pages
    man-pages-posix
    strace
    sudo
    gdb
    git
    openssh
    netcat
    busybox
    pkgsInetUtils.inetutils
    neovim
    ripgrep
  ];

  shellHook = ''
    if [ "''${IN_NIX_SHELL:-}" != "pure" ]; then
      echo "❗ Error: This script must be run inside a Nix shell with pure mode. Run 'nix-shell --pure' IN_NIX_SHELL==''${IN_NIX_SHELL:-})"
      exit 1
    fi

    set -o vi
    export PS1="\[\e[0;32m\]\W>\[\e[0m\] "
    echo ${pkgsInetUtils.inetutils}/bin/ping

    alias ping=${pkgsInetUtils.inetutils}/bin/ping
  '';
}

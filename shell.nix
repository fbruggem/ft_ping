let
 pkgs = import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/a84b0a7c509bdbaafbe6fe6e947bdaa98acafb99.tar.gz";
    sha256 = "0m8zrg4rp5mx5v9ar91ncnjhagmcrd3y9h56y48swan6a8gwpq52";
  }) {};
  buildDeps = import ./default.nix;
in
pkgs.mkShell {
  name = "dev-shell";

  inputsFrom = [ buildDeps ];

  buildInputs = with pkgs; [
    gdb
    netcat
    busybox
    # add extra tools if you want
  ];

  shellHook = ''
    set -o vi
  '';
}

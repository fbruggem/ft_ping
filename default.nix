let
 pkgs = import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/a84b0a7c509bdbaafbe6fe6e947bdaa98acafb99.tar.gz";
    sha256 = "0m8zrg4rp5mx5v9ar91ncnjhagmcrd3y9h56y48swan6a8gwpq52";
  }) {};
  clang21 = pkgs.llvmPackages_21.clang;     # assuming llvmPackages_21 exists
  stdenv   = pkgs.llvmPackages_21.stdenv;   # use clang toolchain
in pkgs.stdenv.mkDerivation {
  name = "ft_ping";
  src = ./.;

  buildInputs = [
    clang21
    # ... other libraries
  ];

  stdenv = stdenv;

  buildPhase = "make";
  installPhase = "make install DESTDIR=$out";
}

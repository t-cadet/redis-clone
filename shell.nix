{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "rediss";
  buildInputs = with pkgs; [
    clang_15
    gnumake
  ];
}

{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "redis-clone";
  buildInputs = with pkgs; [
    clang_15
  ];
}
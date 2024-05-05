{ sources ? import ./nix/sources.nix }:
let
  pkgs = import sources.nixpkgs { config = {}; overlays = []; };
  defaultBuild = pkgs.callPackage ./. { };
  shell = pkgs.mkShell {
    inputsFrom = [ defaultBuild ];
    packages = with pkgs; [
      niv
      gdb
      clang-tools
      clang-analyzer
      clang
    ];

    hardeningDisable = ["all"];
  };
in
{
  inherit shell;
}

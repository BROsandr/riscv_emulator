{ sources ? import ./nix/sources.nix }:
let
  pkgs = import sources.nixpkgs { config = {}; overlays = []; };
  defaultBuild = pkgs.callPackage ./. { };
  shell = pkgs.mkShell {
    inputsFrom = [ defaultBuild ];
    packages = with pkgs; [
      gdb
      clang-tools
      clang-analyzer
      clang
      cmake # helps meson in header-only dependencies searching
    ];

    hardeningDisable = ["all"];

    shellHook = ''
      ln -fs ./build/compile_commands.json compile_commands.json
    '';
  };
in
{
  inherit shell;
}

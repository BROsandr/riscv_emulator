{
  lib,
  stdenv,
  pkgs,
  meson,
  ninja,
  catch2_3,
  pkg-config,
  spdlog,
  sourceFiles ? lib.fileset.unions [./src ./inc ./tests ./meson.build],
}:
let
  fs = pkgs.lib.fileset;
in

stdenv.mkDerivation {
  pname = "riscv_emulator";
  version = "v1.0.0";

  src = fs.toSource {
    root = ./.;
    fileset = sourceFiles;
  };

  nativeBuildInputs = [
    meson
    ninja
    catch2_3
    pkg-config
  ];

  buildInputs = [
    spdlog.dev
  ];
}

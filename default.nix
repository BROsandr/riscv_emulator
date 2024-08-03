{
  lib,
  stdenv,
  meson,
  ninja,
  catch2_3,
  pkg-config,
  spdlog,
  sourceFiles ? lib.fileset.unions [./src ./inc ./tests ./meson.build],
}:
let
  fs = lib.fileset;
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
    pkg-config
  ];

  buildInputs = [
    catch2_3
    spdlog.dev
  ];
}

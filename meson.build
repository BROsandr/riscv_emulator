project('riscv_emulator', 'cpp', default_options : ['cpp_std=c++23'])

incdir = [include_directories('inc')]
catch_dep = dependency('catch2-with-main')

src_test_files = {
    'test_alu.cpp' : [
      'alu.cpp'
    ],
    'test_decoder.cpp' : [
      'decoder.cpp'
    ],
}
test_dir = 'tests'
src_dir = 'src'

foreach test_file, src_files: src_test_files
  test_path = test_dir / test_file
  src_path = []
  foreach file: src_files
    src_path += [src_dir / file]
  endforeach

  exe = executable(test_file, sources : [test_path] + src_path,
      include_directories : incdir,
      cpp_args : [],
      dependencies : catch_dep)
  test(test_file, exe)
endforeach

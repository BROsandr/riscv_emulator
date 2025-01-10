project('riscv_emulator', 'cpp', default_options : ['cpp_std=c++20'])

incdir = [include_directories('inc')]

if get_option('b_sanitize') != ''
  header_only = true
endif

if header_only
  spdlog_dep = dependency('spdlog', method : 'cmake', modules : ['spdlog::spdlog_header_only'],
                          include_type: 'system')
else
  spdlog_dep = dependency('spdlog', include_type: 'system')
endif

catch_dep = dependency('catch2-with-main', include_type: 'system')
dependencies = [catch_dep, spdlog_dep]

test_dir = 'tests'
src_dir = 'src'

src_app_files = [
    src_dir / 'alu.cpp',
    src_dir / 'decoder.cpp',
    src_dir / 'memory.cpp',
    src_dir / 'core.cpp',
    src_dir / 'csr.cpp',
]

# src_all_app_files = src_app_files + [src_dir / 'main.cpp']

src_test_files = {
    'test_alu.cpp' : src_app_files,
    'test_decoder.cpp' : src_app_files,
    'test_rf.cpp' : src_app_files,
    'test_memory.cpp' : src_app_files,
    'test_core.cpp' : src_app_files,
}

foreach test_file, src_files: src_test_files
  test_path = test_dir / test_file
  src_path = []
  foreach file: src_files
    src_path += [file]
  endforeach

  exe = executable(test_file, sources : [test_path] + src_path,
      include_directories : incdir,
      cpp_args : [],
      dependencies : dependencies)
  test(test_file, exe, timeout: 0)
endforeach

# exe_name = 'riscv_emulator'
# executable(exe_name, sources : src_all_app_files, include_directories : incdir, dependencies : dependencies)

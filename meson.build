project('tscpp', 'cpp',
  default_options: [
    'cpp_std=c++20',
    'cpp_eh=none',
    'cpp_rtti=false',
    'default_library=static',
    'prefer_static=true',
    'warning_level=3',
    'buildtype=release',
    'strip=true',
  ])

cpp = meson.get_compiler('cpp')

add_project_arguments(cpp.get_supported_arguments([
  '-Wno-c99-designator',
  '-Wno-c99-extensions',
  '-Wno-format-pedantic',
  '-Wno-gnu-anonymous-struct',
  '-Wno-gnu-case-range',
  '-Wno-gnu-conditional-omitted-operand',
  '-Wno-gnu-statement-expression',
  '-Wno-gnu-zero-variadic-macro-arguments',
  '-Wno-keyword-macro',
  '-Wno-user-defined-literals',
  '-Wno-nested-anon-types',
  '-Werror=return-type',
  '-Werror=switch',
]), language: 'cpp')

subdir('libraries')
subdir('src')
subdir('tests')

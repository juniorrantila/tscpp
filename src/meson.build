tscpp_exe = executable('tscpp', [
  'Codegen.cpp',
  'Lex.cpp',
  'Parse.cpp',
  'Token.cpp',
  'main.cpp',
], dependencies: [
  cli_dep,
  core_dep,
  main_dep,
  ty_dep,
])

tscpp_gen = generator(tscpp_exe,
  output: ['@PLAINNAME@.cpp'],
  arguments: ['@INPUT@', '-o', '@OUTPUT@']
)

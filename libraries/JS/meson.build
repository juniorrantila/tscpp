js_lib = library('js', [
    'Number.cpp',
  ], dependencies: [
    ty_dep,
    core_dep
  ])

js_dep = declare_dependency(
  link_with: js_lib,
  include_directories: '..'
  )

project('tomos', 'cpp', version : '0.1.0', default_options : ['warning_level=3', 'cpp_std=c++20'])

cc            = meson.get_compiler('cpp')
includes      = include_directories('include')
dependencies  = [dependency('boost'), cc.find_library('metis'), dependency('mesh')]

tomos = library(
  meson.project_name()
  , []
  , include_directories : includes
  , dependencies        : dependencies
  , install             : true
  )

tomos_dep = declare_dependency(
  link_with             : tomos
  , include_directories : includes
  , dependencies        : dependencies
  )

if not meson.is_subproject()
  subdir('example')
  subdir('tests')
endif

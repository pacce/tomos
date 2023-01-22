project('tomos', 'cpp', version : '0.1.0', default_options : ['warning_level=3', 'cpp_std=c++20'])

cc            = meson.get_compiler('cpp')
includes      = include_directories('include')
dependencies  = [
    dependency('boost')
  , dependency('OpenCL')
  , cc.find_library('metis')
  , dependency('mesh')
  ]
sources       = ['source/tomos-engine.cpp']

tomos = library(
  meson.project_name()
  , sources
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

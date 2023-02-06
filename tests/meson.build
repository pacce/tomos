gtest         = dependency('gtest')
dependencies  = [gtest, tomos_dep]

engine  = executable('engine', 'engine.cpp', dependencies: dependencies)
mesh    = executable(  'mesh',   'mesh.cpp', dependencies: dependencies)
metis   = executable( 'metis',  'metis.cpp', dependencies: dependencies)
sparse  = executable('sparse', 'sparse.cpp', dependencies: dependencies)

test('engine', engine, workdir : meson.source_root())
test(  'mesh',   mesh)
test( 'metis',  metis)
test('sparse', sparse)

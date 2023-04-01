gtest         = dependency('gtest')
dependencies  = [gtest, tomos_dep]

engine      = executable(   'engine',    'engine.cpp', dependencies: dependencies)
metis       = executable(    'metis',     'metis.cpp', dependencies: dependencies)
partition   = executable('partition', 'partition.cpp', dependencies: dependencies)
sparse      = executable(   'sparse',    'sparse.cpp', dependencies: dependencies)

test(   'engine', engine, workdir : meson.source_root())
test(    'metis',     metis)
test('partition', partition)
test(   'sparse',    sparse)

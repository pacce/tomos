gtest         = dependency('gtest')
dependencies  = [gtest, tomos_dep]

metis   = executable( 'metis',  'metis.cpp', dependencies: dependencies)
sparse  = executable('sparse', 'sparse.cpp', dependencies: dependencies)

test( 'metis',  metis)
test('sparse', sparse)

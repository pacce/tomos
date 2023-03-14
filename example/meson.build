executable(   'colors',     'color.cpp', dependencies: tomos_dep)
executable(    'metis',     'metis.cpp', dependencies: tomos_dep)
executable(   'sparse',    'sparse.cpp', dependencies: tomos_dep)
executable('triangles', 'triangles.cpp', dependencies: tomos_dep)

executable('disk', 'disk.cpp', dependencies: [cc.find_library('gmsh'), tomos_dep])

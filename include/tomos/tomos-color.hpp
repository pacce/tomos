#ifndef TOMOS_COLOR_HPP__
#define TOMOS_COLOR_HPP__

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edge_coloring.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/sequential_vertex_coloring.hpp>
#include <tomos/tomos-mesh.hpp>
#include "tomos/tomos-metis.hpp"

namespace tomos {
namespace color {
    using Index     = std::size_t;
    using Color     = std::size_t;
    using Colors    = std::map<std::size_t, Color>;

    Colors
    build(const tomos::mesh::Mesh& mesh, tomos::metis::Common common);
} // namespace color
} // namespace tomos

#endif // TOMOS_COLOR_HPP__

#ifndef TOMOS_COLOR_HPP__
#define TOMOS_COLOR_HPP__

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edge_coloring.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/sequential_vertex_coloring.hpp>
#include <mesh/mesh.hpp>
#include "tomos/tomos-metis.hpp"

namespace tomos {
namespace color {
    using Index = mesh::element::Number;
    using Color = std::size_t;

    template <typename Precision>
    std::map<mesh::element::Number, std::size_t>
    build(const mesh::Mesh<Precision>& mesh, tomos::metis::Common common) {
        tomos::metis::Adjacency adjacency = tomos::metis::dual(mesh, common);

        typedef std::pair<std::size_t, std::size_t> Edge;
        std::vector<Edge> edges = {};
        for (const auto& [k, vs] : adjacency) {
            for (const tomos::metis::Index& v : vs) {
                std::size_t fst = std::min(k, v) - 1;
                std::size_t snd = std::max(k, v) - 1;
                edges.emplace_back(fst, snd);
            }
        }
        std::sort(edges.begin(), edges.end());
        std::vector<Edge>::iterator it = std::unique(edges.begin(), edges.end());
        edges.resize(std::distance(edges.begin(), it));

        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;

        Graph graph(edges.begin(), edges.end(), adjacency.size());
        std::vector<std::size_t> cs(boost::num_vertices(graph));

        boost::iterator_property_map color(&cs.front(), boost::get(boost::vertex_index, graph));
        boost::sequential_vertex_coloring(graph, color);

        std::map<mesh::element::Number, std::size_t> values;
        for (std::size_t i = 0; i < cs.size(); i++) { values[i + 1] = cs[i]; }
        return values;
    }
} // namespace color
} // namespace tomos

#endif // TOMOS_COLOR_HPP__

#ifndef TOMOS_METIS_HPP__
#define TOMOS_METIS_HPP__

#include <iostream>
#include <mesh/mesh.hpp>
#include <metis.h>

namespace tomos {
namespace metis {
    using Index         = mesh::element::Number;
    using Neighbours    = std::vector<Index>;
    using Adjacency     = std::map<Index, Neighbours>;

    using Partition     = std::size_t;
    using Partitions    = std::map<Index, Partition>;

    enum class Common : uint8_t { NODE = 1, EDGE = 2, FACE = 3 };

    template <typename Precision>
    Adjacency
    dual(const mesh::Mesh<Precision>& mesh, Common common) {
        idx_t ne = static_cast<idx_t>(mesh.element.size());
        idx_t nn = static_cast<idx_t>(mesh.nodes.size());

        std::vector<idx_t> eptr = {0};
        std::vector<idx_t> eind = {};

        Adjacency values;

        for (const auto& [key, e] : mesh.element) {
            values[key] = {};

            eptr.push_back(e.nodes.size() + eptr.back());
            for (const mesh::node::Number& n : e.nodes) { eind.push_back(n - 1); }  // C-style 0-based indexing
        }

        idx_t numflag = 0;  // C-style 0-based indexing
        idx_t ncommon = static_cast<idx_t>(common);

        idx_t * xadj;
        idx_t * adjncy;

        METIS_MeshToDual(
                  &ne   // number of elements
                , &nn   // number of nodes
                , eptr.data()
                , eind.data()
                , &ncommon
                , &numflag
                , &xadj
                , &adjncy
                );

        idx_t * prev = xadj;
        idx_t * next = xadj + 1;
        for (auto& [key, value] : values) {
            for (idx_t j = *prev; j < *next; j++) { value.push_back(adjncy[j] + 1); }
            std::sort(value.begin(), value.end());

            prev++; next++;
        }

        METIS_Free(xadj);
        METIS_Free(adjncy);

        return values;
    }

    template <typename Precision>
    Adjacency
    nodal(const mesh::Mesh<Precision>& mesh) {
        idx_t ne = static_cast<idx_t>(mesh.element.size());
        idx_t nn = static_cast<idx_t>(mesh.nodes.size());

        std::vector<idx_t> eptr = {0};
        std::vector<idx_t> eind = {};

        Adjacency values;
        for (const auto& [key, e] : mesh.nodes) {
            values[key] = {};
        }

        for (const auto& [key, e] : mesh.element) {
            eptr.push_back(e.nodes.size() + eptr.back());
            for (const mesh::node::Number& n : e.nodes) { eind.push_back(n - 1); }  // C-style 0-based indexing
        }

        idx_t numflag = 0;  // C-style 0-based indexing

        idx_t * xadj;
        idx_t * adjncy;

        METIS_MeshToNodal(
                  &ne   // number of elements
                , &nn   // number of nodes
                , eptr.data()
                , eind.data()
                , &numflag
                , &xadj
                , &adjncy
                );

        idx_t * prev = xadj;
        idx_t * next = xadj + 1;
        for (auto& [key, value] : values) {
            for (idx_t j = *prev; j < *next; j++) { value.push_back(adjncy[j] + 1); }

            prev++; next++;
        }

        METIS_Free(xadj);
        METIS_Free(adjncy);

        return values;
    }

    template <typename Precision>
    Partitions
    partition(const mesh::Mesh<Precision>& mesh, Common common, std::size_t count) {
        idx_t ne = static_cast<idx_t>(mesh.element.size());
        idx_t nn = static_cast<idx_t>(mesh.nodes.size());

        std::vector<idx_t> eptr = {0};
        std::vector<idx_t> eind = {};

        for (const auto& [key, e] : mesh.element) {
            eptr.push_back(e.nodes.size() + eptr.back());
            for (const mesh::node::Number& n : e.nodes) { eind.push_back(n - 1); }  // C-style 0-based indexing
        }

        idx_t numflag = 0;  // C-style 0-based indexing
        idx_t ncommon = static_cast<idx_t>(common);

        idx_t * xadj;
        idx_t * adjncy;

        METIS_MeshToDual(
                  &ne   // number of elements
                , &nn   // number of nodes
                , eptr.data()
                , eind.data()
                , &ncommon
                , &numflag
                , &xadj
                , &adjncy
                );

        idx_t ncon      = 1;
        idx_t nparts    = static_cast<idx_t>(count);
        idx_t edgecut   = 0;

        std::vector<idx_t> part(ne);
        idx_t options[METIS_NOPTIONS];
        METIS_SetDefaultOptions(options);

        METIS_PartGraphKway(
                &ne           // number of vertices
                , &ncon         // number of balancing constraints
                , xadj
                , adjncy
                , NULL          // vwgt
                , NULL          // vsize
                , NULL          // adjwgt
                , &nparts       // number of partitions
                , NULL          // tpwgts
                , NULL          // ubvec
                , options       // options
                , &edgecut      // objval
                , part.data()
                );

        std::size_t i = 0;

        Partitions values;
        for (const auto& [key, _] : mesh.element) {
            values[key] = static_cast<std::size_t>(part[i++]);
        }
        return values;
    }
} // namespace metis
} // namespace tomos

#endif // TOMOS_METIS_HPP__

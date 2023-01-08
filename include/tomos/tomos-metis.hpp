#ifndef TOMOS_METIS_HPP__
#define TOMOS_METIS_HPP__

#include <iostream>
#include <mesh/mesh.hpp>
#include <metis.h>

namespace tomos {
namespace metis {
    template <typename Precision>
    std::map<mesh::element::Number, std::vector<mesh::element::Number>>
    dual(const mesh::Mesh<Precision>& mesh) {
        idx_t ne = static_cast<idx_t>(mesh.element.size());
        idx_t nn = static_cast<idx_t>(mesh.nodes.size());

        std::vector<idx_t> eptr = {0};
        std::vector<idx_t> eind = {};

        std::map<mesh::element::Number, std::vector<mesh::element::Number>> values;

        for (const auto& [key, e] : mesh.element) {
            values[key] = {};

            eptr.push_back(e.nodes.size() + eptr.back());
            for (const mesh::node::Number& n : e.nodes) { eind.push_back(n - 1); }  // C-style 0-based indexing
        }

        idx_t numflag = 0;  // C-style 0-based indexing
        idx_t ncommon = 2;  // 2 common nodes per edge

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

            prev++; next++;
        }

        METIS_Free(xadj);
        METIS_Free(adjncy);

        return values;
    }
} // namespace metis
} // namespace tomos

#endif // TOMOS_METIS_HPP__

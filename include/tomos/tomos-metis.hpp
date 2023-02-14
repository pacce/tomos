#ifndef TOMOS_METIS_HPP__
#define TOMOS_METIS_HPP__

#include <iostream>
#include <metis.h>
#include <tomos/tomos-mesh.hpp>

namespace tomos {
namespace metis {
    using Index         = std::size_t;
    using Neighbours    = std::vector<Index>;
    using Adjacency     = std::map<Index, Neighbours>;

    using Partition     = std::size_t;
    using Partitions    = std::map<Index, Partition>;

    enum class Common : uint8_t { NODE = 1, EDGE = 2, FACE = 3 };

    class Dual {
        public:
            Dual(const tomos::mesh::Mesh& mesh, Common common)
                : ne_(static_cast<idx_t>(mesh.elements.size()))
                , nn_(static_cast<idx_t>(mesh.nodes.size()))
            {
                std::vector<idx_t> eptr = {0};
                std::vector<idx_t> eind = {};

                for (const tomos::mesh::Element& e : mesh.elements) {
                    eptr.push_back(e.nodes.size() + eptr.back());
                    for (const mesh::node::Number& n : e.nodes) { eind.push_back(n); }
                }
                idx_t numflag   = 0; // C-style 0-base indexing
                idx_t ncommon   = static_cast<idx_t>(common);

                METIS_MeshToDual(&ne_, &nn_, eptr.data(), eind.data(), &ncommon, &numflag, &xadj_, &adjncy_);
            }

            ~Dual() {
                METIS_Free(xadj_);
                METIS_Free(adjncy_);
            }

            Adjacency
            adjacency() const {
                Adjacency values;

                std::size_t keys    = static_cast<std::size_t>(ne_);
                idx_t * prev        = xadj_;
                idx_t * next        = xadj_ + 1;

                for (std::size_t key = 0; key < keys; key++) {
                    values[key] = {};
                    for (idx_t j = *prev; j < *next; j++) { values[key].push_back(adjncy_[j]); }

                    prev++; next++;
                }
                return values;
            }

            Partitions
            partition(std::size_t count) {
                idx_t ncon      = 1;
                idx_t nparts    = static_cast<idx_t>(count);
                idx_t edgecut   = 0;

                std::vector<idx_t> part(ne_);
                idx_t options[METIS_NOPTIONS];
                METIS_SetDefaultOptions(options);

                METIS_PartGraphKway(
                          &ne_          // number of vertices
                        , &ncon         // number of balancing constraints
                        , xadj_
                        , adjncy_
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

                Partitions values;

                std::size_t keys    = static_cast<std::size_t>(ne_);
                for (std::size_t key = 0; key < keys; key++) {
                    const idx_t& partition  = part[key];
                    values[key]             = static_cast<std::size_t>(partition);
                }
                return values;
            }
        private:
            idx_t ne_;  // number of elements
            idx_t nn_;  // number of nodes

            idx_t * xadj_;      // pointer structure
            idx_t * adjncy_;    // pointer structure
    };

    class Nodal {
        public:
            Nodal(const tomos::mesh::Mesh& mesh)
                : ne_(static_cast<idx_t>(mesh.elements.size()))
                , nn_(static_cast<idx_t>(mesh.nodes.size()))
            {
                std::vector<idx_t> eptr = {0};
                std::vector<idx_t> eind = {};

                for (const tomos::mesh::Element& e : mesh.elements) {
                    eptr.push_back(e.nodes.size() + eptr.back());
                    for (const mesh::node::Number& n : e.nodes) { eind.push_back(n); }
                }

                idx_t numflag = 0;  // C-style 0-based indexing
                METIS_MeshToNodal(&ne_, &nn_, eptr.data(), eind.data(), &numflag, &xadj_, &adjncy_);
            }

            Adjacency
            adjacency() const {
                Adjacency values;

                std::size_t keys    = static_cast<std::size_t>(nn_);
                idx_t * prev        = xadj_;
                idx_t * next        = xadj_ + 1;

                for (std::size_t key = 0; key < keys; key++) {
                    values[key] = {};
                    for (idx_t j = *prev; j < *next; j++) { values[key].push_back(adjncy_[j]); }

                    prev++; next++;
                }
                return values;
            }

            ~Nodal() {
                METIS_Free(xadj_);
                METIS_Free(adjncy_);
            }
        private:
            idx_t ne_;  // number of elements
            idx_t nn_;  // number of nodes

            idx_t * xadj_;      // pointer structure
            idx_t * adjncy_;    // pointer structure
    };
} // namespace metis
} // namespace tomos

#endif // TOMOS_METIS_HPP__

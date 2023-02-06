#ifndef TOMOS_MESH_HPP__
#define TOMOS_MESH_HPP__

#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>

#include <boost/phoenix/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>

namespace tomos {
namespace mesh {
    using Node  = cl_float3;
    using Nodes = std::vector<Node>;
namespace node {
    using Number = cl_uint;
} // namespace node

namespace decoder {
    namespace qi = boost::spirit::qi;

    template <typename Iterator>
    struct nodes : qi::grammar<Iterator, Nodes> {
        nodes() : nodes::base_type(rule) {
            using boost::phoenix::ref;

            coordinates %= qi::skip(qi::space)[
                qi::repeat(3)[qi::float_]
            ];
            line = qi::skip(qi::space)[
                qi::omit[qi::int_] >> coordinates
            ];

            std::size_t count = 0;
            rule %= qi::skip(qi::eol)
                [   qi::lit("$Nodes")
                >>  qi::omit[qi::int_[ref(count) = qi::_1]]
                >>  qi::repeat(ref(count))[line]
                >>  qi::lit("$EndNodes")
                ]
                ;
        }

        qi::rule<Iterator, std::vector<float>>  coordinates;
        qi::rule<Iterator, Node>                line;
        qi::rule<Iterator, Nodes>               rule;
    };

} // namespace decoder
} // namespace mesh
} // namespace tomos

namespace boost {
namespace spirit {
namespace traits {
    template<>
    struct transform_attribute<tomos::mesh::Node, std::vector<float>, boost::spirit::qi::domain> {
        typedef std::vector<float> type;

        static type
        pre(tomos::mesh::Node&) { return {}; }

        static void
        post(tomos::mesh::Node& n, type const& xs) {
            n = {{xs.at(0), xs.at(1), xs.at(2)}};
        }

        static void
        fail(tomos::mesh::Node&) {}
    };
} // namespace traits
} // namespace spirit
} // namespace boost

#endif // TOMOS_MESH_HPP__

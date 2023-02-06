#ifndef TOMOS_MESH_HPP__
#define TOMOS_MESH_HPP__

#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>

#include <boost/phoenix/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <mesh/mesh.hpp>

namespace tomos {
namespace mesh {
    using Node  = cl_float3;
    using Nodes = std::vector<Node>;
namespace node {
    using Number    = cl_uint;
    using Numbers   = std::vector<Number>;
} // namespace node
namespace element {
    enum class Type {
          LINE2         =  1
        , TRIANGLE3     =  2
        , QUADRANGLE4   =  3
        , TETRATHEDRON4 =  4
        , HEXAHEDRON8   =  5
        , PRISM6        =  6
        , PYRAMID5      =  7
        , LINE3         =  8
        , TRIANGLE6     =  9
        , QUADRANGLE9   = 10
        , TETRAHEDRON10 = 11
        , HEXAHEDRON27  = 12
        , PRISM18       = 13
        , PYRAMID14     = 14
        , POINT1        = 15
        , QUADRANGLE8   = 16
        , HEXAHEDRON20  = 17
        , PRISM15       = 18
        , PYRAMID13     = 19
    };

    using Tag   = std::size_t;
    using Tags  = std::vector<Tag>;
} // namespace element
    struct Element {
        element::Type   type;
        node::Numbers   nodes;

        friend std::ostream&
        operator<<(std::ostream& os, const Element& e) {
            switch(e.type) {
                case element::Type::LINE2:
                { os << "2-node line"; break; }
                case element::Type::TRIANGLE3:
                { os << "3-node triangle"; break; }
                case element::Type::QUADRANGLE4:
                { os << "4-node quadrangle"; break; }
                case element::Type::TETRATHEDRON4:
                { os << "4-node tetrahedron"; break; }
                case element::Type::HEXAHEDRON8:
                { os << "8-node hexahedron"; break; }
                case element::Type::PRISM6:
                { os << "6-node prism"; break; }
                case element::Type::PYRAMID5:
                { os << "5-node pyramid"; break; }
                case element::Type::LINE3:
                { os << "3-node line"; break; }
                case element::Type::TRIANGLE6:
                { os << "6-node triangle"; break; }
                case element::Type::QUADRANGLE9:
                { os << "9-node quadrangle"; break; }
                case element::Type::TETRAHEDRON10:
                { os << "10-node tetrahedron"; break; }
                case element::Type::HEXAHEDRON27:
                { os << "27-node hexahedron"; break; }
                case element::Type::PRISM18:
                { os << "18-node prism"; break; }
                case element::Type::PYRAMID14:
                { os << "14-node pyramid"; break; }
                case element::Type::POINT1:
                { os << "1-node point"; break; }
                case element::Type::QUADRANGLE8:
                { os << "8-node quadrangle"; break; }
                case element::Type::HEXAHEDRON20:
                { os << "20-node hexahedron"; break; }
                case element::Type::PRISM15:
                { os << "15-node prism"; break; }
                case element::Type::PYRAMID13:
                { os << "13-node pyramid"; break; }
            }
            os << ", ";
            for (const node::Number& node : e.nodes) { os << node << " "; }
            return os;
        }

        friend bool
        operator==(const Element& lhs, const Element& rhs) {
            auto nodes = [](const std::vector<node::Number>& lh, const std::vector<node::Number>& rh) {
                if (lh.size() == rh.size()) {
                    for (std::size_t i = 0; i < lh.size(); i++) {
                        if (lh[i] != rh[i]) { return false; }
                    }
                } else {
                    return false;
                }
                return true;
            };
            return lhs.type == rhs.type && nodes(lhs.nodes, rhs.nodes);
        }

        friend bool
        operator!=(const Element& lhs, const Element& rhs) {
            return !(lhs == rhs);
        }
    };
    using Elements = std::vector<Element>;

    struct Version {
        float   version;
        int32_t format;
        int32_t size;

        friend bool
        operator==(const Version& lhs, const Version& rhs) {
            return lhs.version  == rhs.version
                && lhs.format   == rhs.format
                && lhs.size     == rhs.size
                ;
        }

        friend bool
        operator!=(const Version& lhs, const Version& rhs) {
            return lhs.version  != rhs.version
                || lhs.format   != rhs.format
                || lhs.size     != rhs.size
                ;
        }

        friend std::ostream&
        operator<<(std::ostream& os, const Version& v) {
            os  << v.version
                << " "
                << v.format
                << " "
                << v.size
                ;
            return os;
        }
    };

    struct Mesh {
        Nodes       nodes;
        Elements    elements;

        Mesh() {}
        Mesh(const Nodes& ns, const Elements& es) : nodes(ns), elements(es) {}

        template <typename Precision>
        Mesh(const ::mesh::Mesh<Precision>& source) {
            nodes.reserve(source.nodes.size());
            for (const auto& [_, n] : source.nodes) {
                nodes.push_back({n.x(), n.y(), n.z()});
            }
            elements.reserve(source.element.size());
            for (const auto& [_, e] : source.element) {
                tomos::mesh::element::Type type;
                switch (e.type) {
                    case ::mesh::element::Type::LINE2:          { type = tomos::mesh::element::Type::LINE2; break; }
                    case ::mesh::element::Type::TRIANGLE3:      { type = tomos::mesh::element::Type::TRIANGLE3; break; }
                    case ::mesh::element::Type::QUADRANGLE4:    { type = tomos::mesh::element::Type::QUADRANGLE4; break; }
                    case ::mesh::element::Type::TETRATHEDRON4:  { type = tomos::mesh::element::Type::TETRATHEDRON4; break; }
                    case ::mesh::element::Type::HEXAHEDRON8:    { type = tomos::mesh::element::Type::HEXAHEDRON8; break; }
                    case ::mesh::element::Type::PRISM6:         { type = tomos::mesh::element::Type::PRISM6; break; }
                    case ::mesh::element::Type::PYRAMID5:       { type = tomos::mesh::element::Type::PYRAMID5; break; }
                    case ::mesh::element::Type::LINE3:          { type = tomos::mesh::element::Type::LINE3; break; }
                    case ::mesh::element::Type::TRIANGLE6:      { type = tomos::mesh::element::Type::TRIANGLE6; break; }
                    case ::mesh::element::Type::QUADRANGLE9:    { type = tomos::mesh::element::Type::QUADRANGLE9; break; }
                    case ::mesh::element::Type::TETRAHEDRON10:  { type = tomos::mesh::element::Type::TETRAHEDRON10; break; }
                    case ::mesh::element::Type::HEXAHEDRON27:   { type = tomos::mesh::element::Type::HEXAHEDRON27; break; }
                    case ::mesh::element::Type::PRISM18:        { type = tomos::mesh::element::Type::PRISM18; break; }
                    case ::mesh::element::Type::PYRAMID14:      { type = tomos::mesh::element::Type::PYRAMID14; break; }
                    case ::mesh::element::Type::POINT1:         { type = tomos::mesh::element::Type::POINT1; break; }
                    case ::mesh::element::Type::QUADRANGLE8:    { type = tomos::mesh::element::Type::QUADRANGLE8; break; }
                    case ::mesh::element::Type::HEXAHEDRON20:   { type = tomos::mesh::element::Type::HEXAHEDRON20; break; }
                    case ::mesh::element::Type::PRISM15:        { type = tomos::mesh::element::Type::PRISM15; break; }
                    case ::mesh::element::Type::PYRAMID13:      { type = tomos::mesh::element::Type::PYRAMID13; break; }
                }
                tomos::mesh::node::Numbers indices;
                indices.reserve(e.nodes.size());
                for (auto index : e .nodes) { indices.push_back(index - 1); }

                elements.push_back({type, indices});
            }
        }
    };
} // namespace mesh
} // namespace tomos

BOOST_FUSION_ADAPT_STRUCT(
        tomos::mesh::Element,
        (tomos::mesh::element::Type, type)
        (tomos::mesh::node::Numbers, nodes)
        );

BOOST_FUSION_ADAPT_STRUCT(
        tomos::mesh::Version,
        (float,     version)
        (int32_t,   format)
        (int32_t,   size)
        );

BOOST_FUSION_ADAPT_STRUCT(
        tomos::mesh::Mesh,
        (tomos::mesh::Nodes,    nodes)
        (tomos::mesh::Elements, elements)
        );

namespace tomos {
namespace mesh {
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

    template <typename Iterator>
    struct elements : qi::grammar<Iterator, Elements> {
        elements() : elements::base_type(rule) {
            using boost::phoenix::ref;

            std::size_t ts  = 0;
            std::size_t es  = 0;
            auto subtract   = [](int& i) { i -= 1; };

            tags    %= qi::skip(qi::space)
                    [  qi::omit[qi::int_[ref(ts) = qi::_1]]
                    >> qi::repeat(ref(ts))[qi::int_]
                    ]
                    ;

            element %= qi::skip(qi::space)
                    [  (qi::int_( 1) >> qi::omit[tags] >> qi::repeat( 2)[qi::int_[subtract]])   // Line 2
                    |  (qi::int_( 2) >> qi::omit[tags] >> qi::repeat( 3)[qi::int_[subtract]])   // Triangle 3
                    |  (qi::int_( 3) >> qi::omit[tags] >> qi::repeat( 4)[qi::int_[subtract]])   // Quadrangle 4
                    |  (qi::int_( 4) >> qi::omit[tags] >> qi::repeat( 4)[qi::int_[subtract]])   // Tetrahedron 4
                    |  (qi::int_( 5) >> qi::omit[tags] >> qi::repeat( 8)[qi::int_[subtract]])   // Hexahedron 8
                    |  (qi::int_( 6) >> qi::omit[tags] >> qi::repeat( 6)[qi::int_[subtract]])   // Prism 6
                    |  (qi::int_( 7) >> qi::omit[tags] >> qi::repeat( 5)[qi::int_[subtract]])   // Pyramid 5
                    |  (qi::int_( 8) >> qi::omit[tags] >> qi::repeat( 3)[qi::int_[subtract]])   // Line 3
                    |  (qi::int_( 9) >> qi::omit[tags] >> qi::repeat( 6)[qi::int_[subtract]])   // Triangle 6
                    |  (qi::int_(10) >> qi::omit[tags] >> qi::repeat( 9)[qi::int_[subtract]])   // Quadrangle 9
                    |  (qi::int_(11) >> qi::omit[tags] >> qi::repeat(10)[qi::int_[subtract]])   // Tetrahedron 10
                    |  (qi::int_(12) >> qi::omit[tags] >> qi::repeat(27)[qi::int_[subtract]])   // Hexahedron 27
                    |  (qi::int_(13) >> qi::omit[tags] >> qi::repeat(18)[qi::int_[subtract]])   // Prism 18
                    |  (qi::int_(14) >> qi::omit[tags] >> qi::repeat(14)[qi::int_[subtract]])   // Pyramid 14
                    |  (qi::int_(15) >> qi::omit[tags] >> qi::repeat( 1)[qi::int_[subtract]])   // Point 1
                    |  (qi::int_(16) >> qi::omit[tags] >> qi::repeat( 8)[qi::int_[subtract]])   // Quadrangle 8
                    |  (qi::int_(17) >> qi::omit[tags] >> qi::repeat(20)[qi::int_[subtract]])   // Hexahedron 20
                    |  (qi::int_(18) >> qi::omit[tags] >> qi::repeat(15)[qi::int_[subtract]])   // Prism 15
                    |  (qi::int_(19) >> qi::omit[tags] >> qi::repeat(13)[qi::int_[subtract]])   // Pyramid 13
                    ]
                    ;
            line    %= qi::skip(qi::space)[qi::omit[qi::int_] >> element];
            rule    %= qi::skip(qi::eol)
                    [   qi::lit("$Elements")
                    >>  qi::omit[qi::int_[ref(es) = qi::_1]]
                    >>  qi::repeat(ref(es))[line]
                    >>  qi::lit("$EndElements")
                    ]
                    ;
        }

        qi::rule<Iterator, element::Tags>   tags;
        qi::rule<Iterator, Element>         element;
        qi::rule<Iterator, Element>         line;
        qi::rule<Iterator, Elements>        rule;
    };

    template <typename Iterator>
    struct version : qi::grammar<Iterator, Version> {
        version() : version::base_type(rule) {
            rule %= qi::skip(qi::eol)
                [  qi::lit("$MeshFormat")
                >> qi::skip(qi::space)[qi::float_ >> qi::int_(0) >> qi::int_(8)]
                >> qi::lit("$EndMeshFormat")
                ]
                ;
        }

        qi::rule<Iterator, Version>  rule;
    };

    template <typename Iterator>
    struct mesh : qi::grammar<Iterator, Mesh> {
        mesh() : mesh::base_type(rule) {
            rule %= qi::omit[versionp]
                >>  nodesp
                >>  elementsp
                ;
        }

        version<Iterator>           versionp;
        nodes<Iterator>             nodesp;
        elements<Iterator>          elementsp;

        qi::rule<Iterator, Mesh>    rule;
    };
} // namespace decoder

    template <typename Iterator>
    Mesh
    decode(const Iterator begin, const Iterator end) {
        tomos::mesh::Mesh msh;

        tomos::mesh::decoder::mesh<Iterator> grammar;
        if (!boost::spirit::qi::parse(begin, end, grammar, msh)) {
            throw std::runtime_error("failed to decode mesh file");
        }
        return msh;
    }
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

#include <boost/spirit/include/qi.hpp>
#include <gtest/gtest.h>
#include <mesh/mesh.hpp>
#include <tomos/tomos.hpp>

TEST(Nodes, Parse) {
    std::string ss =
        "$Nodes\n"
        "6\n"
        "1 0.0 0.0 0.0\n"
        "2 1.0 0.0 0.0\n"
        "3 1.0 1.0 0.0\n"
        "4 0.0 1.0 0.0\n"
        "5 2.0 0.0 0.0\n"
        "6 2.0 1.0 0.0\n"
        "$EndNodes"
        ;
    tomos::mesh::Nodes expected = {
          {{0.0f, 0.0f, 0.0f}}
        , {{1.0f, 0.0f, 0.0f}}
        , {{1.0f, 1.0f, 0.0f}}
        , {{0.0f, 1.0f, 0.0f}}
        , {{2.0f, 0.0f, 0.0f}}
        , {{2.0f, 1.0f, 0.0f}}
    };
    tomos::mesh::Nodes actual;

    using Iterator = std::string::iterator;

    tomos::mesh::decoder::nodes<Iterator> grammar;
    bool result = boost::spirit::qi::parse(ss.begin(), ss.end(), grammar, actual);

    ASSERT_TRUE(result);
    ASSERT_EQ(actual.size(), expected.size());
    for (std::size_t i = 0; i < actual.size(); i++) {
    for (std::size_t j = 0; j < 3; j++) {
        EXPECT_EQ(actual[i].s[j], expected[i].s[j]);
    }
    }
}

TEST(Elements, Parse) {
    std::string ss = 
        "$Elements\n"
        "2\n"
        "1 3 2 99 2 1 2 3 4\n"
        "2 3 2 99 2 2 5 6 3\n"
        "$EndElements\n"
        ;
    tomos::mesh::Elements expected = {
          {tomos::mesh::element::Type::QUADRANGLE4, {0, 1, 2, 3}}
        , {tomos::mesh::element::Type::QUADRANGLE4, {1, 4, 5, 2}}
    };
    tomos::mesh::Elements actual;

    using Iterator = std::string::iterator;

    tomos::mesh::decoder::elements<Iterator> grammar;
    bool result = boost::spirit::qi::parse(ss.begin(), ss.end(), grammar, actual);

    ASSERT_TRUE(result);
    ASSERT_EQ(actual.size(), expected.size());
    for (std::size_t i = 0; i < actual.size(); i++) {
        EXPECT_EQ(actual[i], expected[i]);
    }
}


int
main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include <boost/spirit/include/qi.hpp>
#include <gtest/gtest.h>
#include <mesh/mesh.hpp>
#include <tomos/tomos.hpp>

const mesh::Mesh<float> MESH = {
      {2.2, 0, 8}
    , mesh::node::Map<float>{
          {1, mesh::Node<float>(0.0f, 0.0f, 0.0f)}
        , {2, mesh::Node<float>(1.0f, 0.0f, 0.0f)}
        , {3, mesh::Node<float>(2.0f, 0.0f, 0.0f)}
        , {4, mesh::Node<float>(0.0f, 1.0f, 0.0f)}
        , {5, mesh::Node<float>(1.0f, 1.0f, 0.0f)}
        , {6, mesh::Node<float>(2.0f, 1.0f, 0.0f)}
        , {7, mesh::Node<float>(0.0f, 2.0f, 0.0f)}
        , {8, mesh::Node<float>(1.0f, 2.0f, 0.0f)}
        , {9, mesh::Node<float>(2.0f, 2.0f, 0.0f)}
    }
    , mesh::element::Map{
          {1, {mesh::element::Type::TRIANGLE3, {1}, {1, 5, 4}}}
        , {2, {mesh::element::Type::TRIANGLE3, {1}, {1, 2, 5}}}
        , {3, {mesh::element::Type::TRIANGLE3, {1}, {2, 3, 5}}}
        , {4, {mesh::element::Type::TRIANGLE3, {1}, {3, 6, 5}}}
        , {5, {mesh::element::Type::TRIANGLE3, {1}, {4, 5, 7}}}
        , {6, {mesh::element::Type::TRIANGLE3, {1}, {7, 5, 8}}}
        , {7, {mesh::element::Type::TRIANGLE3, {1}, {5, 9, 8}}}
        , {8, {mesh::element::Type::TRIANGLE3, {1}, {5, 6, 9}}}
    }
};

TEST(Sparse, CSR) {
    tomos::sparse::Indices cols = {
          0, 1, 3, 4
        , 0, 1, 2, 4
        , 1, 2, 4, 5
        , 0, 3, 4, 6
        , 0, 1, 2, 3, 4, 5, 6, 7, 8
        , 2, 4, 5, 8
        , 3, 4, 6, 7
        , 4, 6, 7, 8
        , 4, 5, 7, 8
    };
    tomos::sparse::Indices rows = {0, 4, 8, 12, 16, 25, 29, 33, 37, 41};
    const auto [cs, rs]         = tomos::sparse::csr(MESH);

    ASSERT_EQ(rs.size(), rows.size());
    for (std::size_t i = 0; i < rows.size(); i++) { EXPECT_EQ(rs[i], rows[i]); }

    ASSERT_EQ(cs.size(), cols.size());
    for (std::size_t i = 0; i < rows.size() - 1; i++) {
        tomos::sparse::Indices actual   = {};
        tomos::sparse::Indices expected = {};

        for (std::size_t j = rows[i]; j < rows[i + 1]; j++) {
            actual.push_back(cs[j]);
            expected.push_back(cols[j]);
        }
        std::sort(  actual.begin(),   actual.end());
        std::sort(expected.begin(), expected.end());

        for (std::size_t j = 0; j < expected.size(); j++) { EXPECT_EQ(actual[j], expected[j]); }
    }
}

int
main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

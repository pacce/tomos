#include <boost/spirit/include/qi.hpp>
#include <gtest/gtest.h>
#include <tomos/tomos.hpp>
#include <tomos/tomos-mesh.hpp>

const tomos::mesh::Mesh MESH = {
    tomos::mesh::Nodes{
          {{0.0f, 0.0f, 0.0f}}
        , {{1.0f, 0.0f, 0.0f}}
        , {{2.0f, 0.0f, 0.0f}}
        , {{0.0f, 1.0f, 0.0f}}
        , {{1.0f, 1.0f, 0.0f}}
        , {{2.0f, 1.0f, 0.0f}}
        , {{0.0f, 2.0f, 0.0f}}
        , {{1.0f, 2.0f, 0.0f}}
        , {{2.0f, 2.0f, 0.0f}}
    }
    , tomos::mesh::Elements{
          {tomos::mesh::element::Type::TRIANGLE3, {0, 4, 3}}
        , {tomos::mesh::element::Type::TRIANGLE3, {0, 1, 4}}
        , {tomos::mesh::element::Type::TRIANGLE3, {1, 2, 4}}
        , {tomos::mesh::element::Type::TRIANGLE3, {2, 5, 4}}
        , {tomos::mesh::element::Type::TRIANGLE3, {3, 4, 6}}
        , {tomos::mesh::element::Type::TRIANGLE3, {6, 4, 7}}
        , {tomos::mesh::element::Type::TRIANGLE3, {4, 8, 7}}
        , {tomos::mesh::element::Type::TRIANGLE3, {4, 5, 8}}
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

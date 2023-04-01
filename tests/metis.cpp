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

TEST(Dual, Node) {
    tomos::metis::Dual dual(MESH, tomos::metis::Common::NODE);
    tomos::metis::Adjacency actual      = dual.adjacency();
    tomos::metis::Adjacency expected    = {
          {0, {1, 2, 3, 4, 5, 6, 7}}
        , {1, {0, 2, 3, 4, 5, 6, 7}}
        , {2, {0, 1, 3, 4, 5, 6, 7}}
        , {3, {0, 1, 2, 4, 5, 6, 7}}
        , {4, {0, 1, 2, 3, 5, 6, 7}}
        , {5, {0, 1, 2, 3, 4, 6, 7}}
        , {6, {0, 1, 2, 3, 4, 5, 7}}
        , {7, {0, 1, 2, 3, 4, 5, 6}}
    };

    ASSERT_EQ(actual.size(), expected.size());

    for (auto& [k, xs] : expected) {
        tomos::metis::Neighbours& ys = actual[k];

        std::sort(xs.begin(), xs.end());
        std::sort(ys.begin(), ys.end());

        ASSERT_EQ(xs.size(), ys.size());
        for (std::size_t i = 0; i < xs.size(); i++) {
            EXPECT_EQ(xs[i], ys[i]);
        }
    }
}

TEST(Dual, Edge) {
    tomos::metis::Dual dual(MESH, tomos::metis::Common::EDGE);
    tomos::metis::Adjacency actual      = dual.adjacency();
    tomos::metis::Adjacency expected    = {
          {0, {1, 4}}
        , {1, {0, 2}}
        , {2, {1, 3}}
        , {3, {2, 7}}
        , {4, {0, 5}}
        , {5, {4, 6}}
        , {6, {5, 7}}
        , {7, {3, 6}}
    };

    ASSERT_EQ(actual.size(), expected.size());

    for (auto& [k, xs] : expected) {
        tomos::metis::Neighbours& ys = actual[k];

        std::sort(xs.begin(), xs.end());
        std::sort(ys.begin(), ys.end());

        ASSERT_EQ(xs.size(), ys.size());
        for (std::size_t i = 0; i < xs.size(); i++) {
            EXPECT_EQ(xs[i], ys[i]);
        }
    }
}

TEST(Nodal, Simple) {
    tomos::metis::Nodal nodal(MESH);
    tomos::metis::Adjacency actual      = nodal.adjacency();
    tomos::metis::Adjacency expected    = {
          {0, {1, 3, 4}}
        , {1, {0, 2, 4}}
        , {2, {1, 4, 5}}
        , {3, {0, 4, 6}}
        , {4, {0, 1, 2, 3, 5, 6, 7, 8}}
        , {5, {2, 4, 8}}
        , {6, {3, 4, 7}}
        , {7, {4, 6, 8}}
        , {8, {4, 5, 7}}
    };

    ASSERT_EQ(actual.size(), expected.size());

    for (auto& [k, xs] : expected) {
        tomos::metis::Neighbours& ys = actual[k];

        std::sort(xs.begin(), xs.end());
        std::sort(ys.begin(), ys.end());

        ASSERT_EQ(xs.size(), ys.size());
        for (std::size_t i = 0; i < xs.size(); i++) {
            EXPECT_EQ(xs[i], ys[i]);
        }
    }
}

TEST(Partition, Edge2) {
    tomos::metis::Dual dual(MESH, tomos::metis::Common::EDGE);
    tomos::metis::Partitions actual     = dual.partition(2);
    tomos::metis::Partitions expected   = {
          {0, {0, 4, 5, 6}}
        , {1, {1, 2, 3, 7}}
    };

    ASSERT_EQ(actual.size(), expected.size());

    for (auto& [k, xs] : expected) {
        const tomos::metis::Indices& ys = actual[k];
        ASSERT_EQ(xs.size(), ys.size());

        for (std::size_t i = 0; i < xs.size(); i++) { EXPECT_EQ(xs[i], ys[i]); }
    }
}

TEST(Partition, Edge4) {
    tomos::metis::Dual dual(MESH, tomos::metis::Common::EDGE);
    tomos::metis::Partitions actual     = dual.partition(4);
    tomos::metis::Partitions expected   = {
          {0, {5, 6}}
        , {1, {0, 4}}
        , {2, {3, 7}}
        , {3, {1, 2}}
    };

    ASSERT_EQ(actual.size(), expected.size());

    for (auto& [k, xs] : expected) {
        const tomos::metis::Indices& ys = actual[k];
        ASSERT_EQ(xs.size(), ys.size());

        for (std::size_t i = 0; i < xs.size(); i++) { EXPECT_EQ(xs[i], ys[i]); }
    }
}

int
main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

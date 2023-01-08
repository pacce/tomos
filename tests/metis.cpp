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

TEST(Dual, Node) {
    tomos::metis::Adjacency actual      = tomos::metis::dual(MESH, tomos::metis::Common::NODE);
    tomos::metis::Adjacency expected    = {
          {1, {2, 3, 4, 5, 6, 7, 8}}
        , {2, {1, 3, 4, 5, 6, 7, 8}}
        , {3, {1, 2, 4, 5, 6, 7, 8}}
        , {4, {1, 2, 3, 5, 6, 7, 8}}
        , {5, {1, 2, 3, 4, 6, 7, 8}}
        , {6, {1, 2, 3, 4, 5, 7, 8}}
        , {7, {1, 2, 3, 4, 5, 6, 8}}
        , {8, {1, 2, 3, 4, 5, 6, 7}}
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
    tomos::metis::Adjacency actual      = tomos::metis::dual(MESH, tomos::metis::Common::EDGE);
    tomos::metis::Adjacency expected    = {
          {1, {2, 5}}
        , {2, {1, 3}}
        , {3, {2, 4}}
        , {4, {3, 8}}
        , {5, {1, 6}}
        , {6, {5, 7}}
        , {7, {6, 8}}
        , {8, {4, 7}}
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
    tomos::metis::Adjacency actual      = tomos::metis::nodal(MESH);
    tomos::metis::Adjacency expected    = {
          {1, {2, 4, 5}}
        , {2, {1, 3, 5}}
        , {3, {2, 5, 6}}
        , {4, {1, 5, 7}}
        , {5, {1, 2, 3, 4, 6, 7, 8, 9}}
        , {6, {3, 5, 9}}
        , {7, {4, 5, 8}}
        , {8, {5, 7, 9}}
        , {9, {5, 6, 8}}
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
    tomos::metis::Partitions actual     = tomos::metis::partition(MESH, tomos::metis::Common::EDGE, 2);
    tomos::metis::Partitions expected   = {
          {1, 0}
        , {2, 1}
        , {3, 1}
        , {4, 1}
        , {5, 0}
        , {6, 0}
        , {7, 0}
        , {8, 1}
    };

    ASSERT_EQ(actual.size(), expected.size());

    for (auto& [k, x] : expected) {
        tomos::metis::Partition& y = actual[k];
        EXPECT_EQ(x, y);
    }
}

TEST(Partition, Edge4) {
    tomos::metis::Partitions actual     = tomos::metis::partition(MESH, tomos::metis::Common::EDGE, 4);
    tomos::metis::Partitions expected   = {
          {1, 1}
        , {2, 3}
        , {3, 3}
        , {4, 2}
        , {5, 1}
        , {6, 0}
        , {7, 0}
        , {8, 2}
    };

    ASSERT_EQ(actual.size(), expected.size());

    for (auto& [k, x] : expected) {
        tomos::metis::Partition& y = actual[k];
        EXPECT_EQ(x, y);
    }
}

int
main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

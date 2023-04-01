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

TEST(Partition, Valid) {
    tomos::metis::Dual dual(MESH, tomos::metis::Common::EDGE);
    tomos::metis::Partitions ps = dual.partition(2);

    EXPECT_FALSE(tomos::partition::valid(MESH, ps, 1));
    EXPECT_TRUE(tomos::partition::valid(MESH, ps, 6));
}

int
main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

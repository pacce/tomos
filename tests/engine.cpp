#include <boost/spirit/include/qi.hpp>
#include <gtest/gtest.h>
#include <tomos/tomos.hpp>
#include <tomos/tomos-mesh.hpp>

const std::filesystem::path KERNEL = {"./shaders/tomos.kernel"};

TEST(GPU, Area) {
    tomos::mesh::Mesh mesh = {
          tomos::mesh::Nodes{
              {{0.0f, 0.0f, 0.0f}}
            , {{1.0f, 0.0f, 0.0f}}
            , {{1.0f, 1.0f, 0.0f}}
            , {{2.0f, 0.0f, 0.0f}}
            , {{2.0f, 2.0f, 0.0f}}
            , {{3.0f, 0.0f, 0.0f}}
            , {{3.0f, 3.0f, 0.0f}}
        }
        , tomos::mesh::Elements{
              {tomos::mesh::element::Type::TRIANGLE3, {0, 1, 2}}
            , {tomos::mesh::element::Type::TRIANGLE3, {0, 3, 4}}
            , {tomos::mesh::element::Type::TRIANGLE3, {0, 5, 6}}
        }
    };
    tomos::Engine engine(KERNEL, mesh);
    std::vector<float> expected = {0.5, 2.0, 4.5};
    std::vector<float> actual   = engine.area();

    ASSERT_EQ(actual.size(), expected.size());
    for (std::size_t i = 0; i < actual.size(); i++) { EXPECT_EQ(actual[i], expected[i]); }
}

TEST(GPU, Centroid) {
    tomos::mesh::Mesh mesh = {
         tomos::mesh::Nodes{
              {{0.0f, 0.0f, 0.0f}}
            , {{1.0f, 0.0f, 0.0f}}
            , {{1.0f, 1.0f, 0.0f}}
            , {{2.0f, 0.0f, 0.0f}}
            , {{2.0f, 2.0f, 0.0f}}
            , {{3.0f, 0.0f, 0.0f}}
            , {{3.0f, 3.0f, 0.0f}}
        }
        , tomos::mesh::Elements{
              {tomos::mesh::element::Type::TRIANGLE3, {0, 1, 2}}
            , {tomos::mesh::element::Type::TRIANGLE3, {0, 3, 4}}
            , {tomos::mesh::element::Type::TRIANGLE3, {0, 5, 6}}
        }
    };
    tomos::Engine engine(KERNEL, mesh);
    tomos::mesh::Nodes expected = {
          {{0.666666, 0.333333, 0.000000}}
        , {{1.333333, 0.666666, 0.000000}}
        , {{2.000000, 1.000000, 0.000000}}
    };
    tomos::mesh::Nodes actual   = engine.centroid();

    ASSERT_EQ(actual.size(), expected.size());
    for (std::size_t i = 0; i < actual.size(); i++) {
        for (std::size_t j = 0; j < 3; j++) {
            float value = actual[i].s[j] - expected[i].s[j];
            EXPECT_TRUE(std::abs(value) < 1e-3);
        }
    }
}

TEST(GPU, Normal) {
    tomos::mesh::Mesh mesh = {
         tomos::mesh::Nodes{
              {{0.0f, 0.0f, 0.0f}}
            , {{1.0f, 0.0f, 0.0f}}
            , {{1.0f, 1.0f, 0.0f}}
            , {{2.0f, 0.0f, 0.0f}}
            , {{2.0f, 2.0f, 0.0f}}
            , {{3.0f, 0.0f, 0.0f}}
            , {{3.0f, 3.0f, 0.0f}}
        }
        , tomos::mesh::Elements{
              {tomos::mesh::element::Type::TRIANGLE3, {0, 1, 2}}
            , {tomos::mesh::element::Type::TRIANGLE3, {0, 3, 4}}
            , {tomos::mesh::element::Type::TRIANGLE3, {0, 5, 6}}
        }
    };
    tomos::Engine engine(KERNEL, mesh);
    tomos::mesh::Nodes expected = {
          {{0.0f, 0.0f, 1.0f}}
        , {{0.0f, 0.0f, 4.0f}}
        , {{0.0f, 0.0f, 9.0f}}
    };
    tomos::mesh::Nodes actual   = engine.normal();

    ASSERT_EQ(actual.size(), expected.size());
    for (std::size_t i = 0; i < actual.size(); i++) {
        for (std::size_t j = 0; j < 3; j++) {
            EXPECT_FLOAT_EQ(actual[i].s[j], expected[i].s[j]);
        }
    }
}


TEST(Stiffness, Oracle) {
    const tomos::mesh::Mesh mesh = {
        tomos::mesh::Nodes{
              {{-0.99703213, -0.07698669, 0.0}}
            , {{-0.91614308, -0.04714896, 0.0}}
            , {{-1.00000000, -0.00000000, 0.0}}
        }
        , tomos::mesh::Elements{
            {{tomos::mesh::element::Type::TRIANGLE3, {0, 1, 2}}}
        }
    };
    tomos::Engine engine(KERNEL, mesh);
    std::vector<float> expected = {
           0.73267143964767456
        , -0.30705833435058594
        , -0.42561310529708862
        ,  0.46990343928337097
        , -0.30705833435058594
        , -0.16284510493278503
        ,  0.58845824003219604
        , -0.42561310529708862
        , -0.16284510493278503
    };
    std::vector<float> actual   = engine.color();
    ASSERT_EQ(actual.size(), expected.size());

    for (std::size_t i = 0; i < actual.size(); i++) {
        EXPECT_FLOAT_EQ(actual[i], expected[i]);
    }
}

TEST(Stiffness, Square) {
    const tomos::mesh::Mesh mesh = {
        tomos::mesh::Nodes{
              {{0.0, 0.0, 0.0}}
            , {{1.0, 0.0, 0.0}}
            , {{1.0, 1.0, 0.0}}
            , {{0.0, 1.0, 0.0}}
        }
        , tomos::mesh::Elements{
              {tomos::mesh::element::Type::TRIANGLE3, {0, 1, 2}}
            , {tomos::mesh::element::Type::TRIANGLE3, {0, 2, 3}}
        }
    };
    tomos::Engine engine(KERNEL, mesh);
    std::vector<float> expected = {
           1.0  // (1, 1) -  0
        , -0.5  // (1, 2) -  1
        ,  0.0  // (1, 3) -  2
        , -0.5  // (1, 4) -  3
        ,  1.0  // (2, 2) -  4
        , -0.5  // (2, 1) -  5
        , -0.5  // (2, 3) -  6
        ,  1.0  // (3, 3) -  7
        ,  0.0  // (3, 1) -  8
        , -0.5  // (3, 2) -  9
        , -0.5  // (3, 4) - 10
        ,  1.0  // (4, 4) - 11
        , -0.5  // (4, 1) - 12
        , -0.5  // (4, 3) - 13
    };
    std::vector<float> actual = engine.color();
    ASSERT_EQ(actual.size(), expected.size());

    for (std::size_t i = 0; i < actual.size(); i++) {
        EXPECT_FLOAT_EQ(actual[i], expected[i]);
    }
}


int
main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

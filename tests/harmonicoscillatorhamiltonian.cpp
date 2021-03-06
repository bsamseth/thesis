#include "definitions.hpp"
#include "harmonicoscillator.hpp"
#include "importancesampler.hpp"
#include "metropolissampler.hpp"
#include "simplegaussian.hpp"
#include "system.hpp"

#include <gtest/gtest.h>
#include <random>

namespace
{
std::default_random_engine           rand_gen(12345);
std::uniform_real_distribution<Real> rand_dist(-1, 1);
std::uniform_int_distribution<int>   rand_dim(1, 3);

double double_gen()
{
    return rand_dist(rand_gen);
}
int dim_gen()
{
    return rand_dim(rand_gen);
}
}  // namespace

class HarmonicOscillatorTest : public ::testing::Test
{
protected:
    System*            s;
    HarmonicOscillator H_1 {1, 1};
    HarmonicOscillator H_2 {1, 2.8};

    virtual void SetUp()
    {
        s = new System(4, 3);
        // Random values.
        s->row(0) << 0.50833829, 0.07732213, 0.69294646;
        s->row(1) << 0.63837196, 0.48833327, 0.17570063;
        s->row(2) << 0.72436579, 0.36970369, 0.49771584;
        s->row(3) << 0.42984966, 0.72519657, 0.30454728;
    }
    virtual void TearDown() { delete s; }
};

TEST_F(HarmonicOscillatorTest, potential)
{
    // Calculated by hand/calculator.
    EXPECT_DOUBLE_EQ(4.4791622360462391, H_2.external_potential(*s));
    EXPECT_DOUBLE_EQ(1.5669788465930243, H_1.external_potential(*s));
    EXPECT_DOUBLE_EQ(0, H_2.internal_potential(*s));
    EXPECT_DOUBLE_EQ(0, H_1.internal_potential(*s));
}

TEST_F(HarmonicOscillatorTest, potential2D)
{
    System s(2, 1);
    s.col(0) << 0.75, -1.5;

    // Check potential for 2D case. Should be agnotic of
    // omega_z value.

    // Calculated by hand/calculator.
    EXPECT_DOUBLE_EQ(1.40625, H_2.external_potential(s));
    EXPECT_DOUBLE_EQ(1.40625, H_1.external_potential(s));
    EXPECT_DOUBLE_EQ(0, H_2.internal_potential(s));
    EXPECT_DOUBLE_EQ(0, H_1.internal_potential(s));
}

TEST_F(HarmonicOscillatorTest, localEnergyAlphaBetaOmegaZ)
{
    SimpleGaussian psi_1(0.6, 1);
    SimpleGaussian psi_2(0.6, 2.8);

    // Calculated by hand/calculator.
    EXPECT_DOUBLE_EQ(6.5105293074990698, H_1.local_energy(*s, psi_1));
    EXPECT_DOUBLE_EQ(9.5491686161396530, H_2.local_energy(*s, psi_2));
}

/*
 * For the ideal alpha value, we have another expression for the local energy.
 *
 *  E_L = 0.5 * dims * N
 *
 * This works when alpha = 0.5, and is independent of position.
 * Therefore, we can run randomized tests, as the expression used to check
 * against is not the same as the one we use the generate our answer.
 */
TEST_F(HarmonicOscillatorTest, local_energy_simple)
{
    const Real     alpha = 0.5;
    SimpleGaussian psi(alpha);

    // 1000, why not?
    for (int runs = 0; runs < 1000; ++runs)
    {
        int    dims      = dim_gen();
        int    particles = dim_gen() * dim_gen() * dim_gen() * dim_gen();
        System s(dims, particles);
        for (int j = 0; j < s.rows(); ++j)
        {
            for (int i = 0; i < s.cols(); ++i)
            {
                s(j, i) = double_gen();
            }
        }

        Real expected = alpha * s.rows() * s.cols();
        ASSERT_NEAR(expected, H_1.local_energy(s, psi), expected * 1e-15);
        ASSERT_NEAR(expected, H_1.local_energy_numeric(s, psi), expected * 1e-6);
    }
}

TEST(HamiltonianBase, local_energy_array)
{
    const HarmonicOscillator H;
    SimpleGaussian           psi(0.5001);
    System                   system(1, 1);
    ImportanceSampler        sampler(system, psi, 0.1);
    std::vector<int>         sizes {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 1000};

    for (int size : sizes)
    {
        auto E_L = H.local_energy_array(sampler, psi, size);
        ASSERT_EQ(size, E_L.size());
    }
}

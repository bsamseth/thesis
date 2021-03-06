import numpy as np

from qflow.wavefunctions import FixedWavefunction, SimpleGaussian, WavefunctionProduct


def test_consistent_with_original():
    orig = SimpleGaussian()
    fixed = FixedWavefunction(orig)

    for _ in range(100):
        n, d = np.random.randint(100), np.random.randint(1, 3 + 1)
        s = np.random.randn(n, d)

        assert np.isclose(orig(s), fixed(s))
        assert np.isclose(orig.laplacian(s), fixed.laplacian(s))
        np.testing.assert_allclose(orig.drift_force(s), fixed.drift_force(s))
        np.testing.assert_array_equal(orig.parameters, fixed.parameters)

        # Gradient is special, should return zeros for each parameter as they are fixed.
        np.testing.assert_array_equal(orig.gradient(s) * 0, fixed.gradient(s))


def test_set_parameters_is_noop():
    orig = SimpleGaussian()
    fixed = FixedWavefunction(orig)

    # This should do noting to the parameters.
    fixed.parameters = orig.parameters + 1

    np.testing.assert_array_equal(orig.parameters, fixed.parameters)


def test_set_parameters_is_noop_in_wavefunction_product():
    orig1 = SimpleGaussian(0.4)
    fixed = FixedWavefunction(orig1)
    orig2 = SimpleGaussian(0.6)
    prod = WavefunctionProduct(fixed, orig2)

    # Only the params related to orig2 should change after this.
    # Also, orig2's beta should not change because it is const by def.
    prod.parameters = [0.5, 1.1, 123, 321]

    np.testing.assert_array_equal(prod.parameters, [0.4, 1, 123, 1])

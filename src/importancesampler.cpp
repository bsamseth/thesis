#include "definitions.hpp"
#include "system.hpp"
#include "wavefunction.hpp"
#include "hamiltonian.hpp"
#include "importancesampler.hpp"

ImportanceSampler::ImportanceSampler(const System &system,
                                     const Wavefunction &wavefunction,
                                     Real step)
    : Sampler(system, wavefunction, step)
{
    initialize_system();
}

void ImportanceSampler::initialize_system() {
    for (int i = 0; i < _system_old.cols(); ++i) {
        for (int d = 0; d < _system_old.rows(); ++d) {
            _system_old(d, i) = rnorm(rand_gen) * std::sqrt(_step);
        }
    }
    _system_new = _system_old;
    _psi_old = (*_wavefunction)(_system_old);
}

void ImportanceSampler::perturb_system() {
    for (int d = 0; d < _system_new.rows(); ++d) {
        _system_new(d, _particle_to_move) += rnorm(rand_gen) * std::sqrt(_step)
                   + 0.5 * _step * _wavefunction->drift_force(_system_new, _particle_to_move, d);
    }

    _psi_new = (*_wavefunction)(_system_new);
}

Real ImportanceSampler::acceptance_probability() const {
    const Vector &r_old = _system_old.col(_particle_to_move);
    const Vector &r_new = _system_new.col(_particle_to_move);

    Real green1 = 0, green2 = 0;
    for (int d = 0; d < r_old.size(); ++d) {
        green1 += square(r_old[d] - r_new[d] - 0.5 * _step * _wavefunction->drift_force(_system_new, _particle_to_move, d));
        green2 += square(r_new[d] - r_old[d] - 0.5 * _step * _wavefunction->drift_force(_system_old, _particle_to_move, d));
    }

    // Ratio = exp(-green1/(4*D*step)) / exp(-green2/(4*D*step))
    Real ratio = std::exp( (green2 - green1) / (2 * _step) );

    return ratio * square(_psi_new) / square(_psi_old);
}
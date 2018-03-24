#include "definitions.hpp"
#include "system.hpp"
#include "wavefunction.hpp"
#include "hamiltonian.hpp"
#include "importancesampler.hpp"

ImportanceSampler::ImportanceSampler(const System &system,
                                     const Wavefunction &wavefunction,
                                     Real step)
    : Sampler(system, wavefunction, step), _q_force_old(0), _q_force_new(0)
{
    initialize_system();
}

void ImportanceSampler::initialize_system() {
    for (int i = 0; i < _system_old.get_n_bosons(); ++i) {
        for (int d = 0; d < _system_old.get_dimensions(); ++d) {
            _system_old[i][d] = rnorm(rand_gen) * std::sqrt(_step);
        }
    }
    _system_new = _system_old;
}

void ImportanceSampler::perturb_system() {
    Boson &boson = _system_new[_particle_to_move];
    _q_force_old = (*_wavefunction).drift_force(_system_old, _particle_to_move);

    for (int d = 0; d < boson.get_dimensions(); ++d) {
        boson[d] += rnorm(rand_gen) * std::sqrt(_step)
                   + 0.5 * _step * _q_force_old[d];
    }

    _psi_new = (*_wavefunction)(_system_new);
    _q_force_new = (*_wavefunction).drift_force(_system_new, _particle_to_move);
}

Real ImportanceSampler::acceptance_probability() const {
    const Boson &r_old = _system_old[_particle_to_move];
    const Boson &r_new = _system_new[_particle_to_move];

    Real exponent = 0;
    for (int d = 0; d < _system_new.get_dimensions(); ++d) {
        exponent += (_q_force_old[d] + _q_force_new[d]) *
            (0.5 * _step * (_q_force_old[d] - _q_force_new[d]) + 2 * ( r_old[d] - r_new[d] ) );
    }

    return std::exp(exponent) * square(_psi_new) / square(_psi_old);
}
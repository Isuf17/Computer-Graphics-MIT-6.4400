#ifndef FORWARD_TRAPEZOIDAL_INTEGRATOR_H_
#define FORWARD_TRAPEZOIDAL_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class ForwardTrapezoidalIntegrator : public IntegratorBase<TSystem, TState> {
  TState Integrate(const TSystem& system,
                   const TState& state,
                   float start_time,
                   float dt) const override {

    TState f_0 = system.ComputeTimeDerivative(state, start_time);
    TState f_1 = system.ComputeTimeDerivative(state + dt*f_0, start_time + dt);
    return state + (dt/2.0)*(f_0 + f_1); /* trapezoidal integration */
  }
};
}  // namespace GLOO

#endif
#ifndef FORWARD_RK4_INTEGRATOR_H_
#define FORWARD_RK4_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class ForwardRK4Integrator : public IntegratorBase<TSystem, TState> {
  TState Integrate(const TSystem& system,
                   const TState& state,
                   float start_time,
                   float dt) const override {

    TState k1 = system.ComputeTimeDerivative(state, start_time);
    TState k2 = system.ComputeTimeDerivative(state + (dt/2.0)*k1, start_time + (dt/2.0));
    TState k3 = system.ComputeTimeDerivative(state + (dt/2.0)*k2, start_time + (dt/2.0));
    TState k4 = system.ComputeTimeDerivative(state + dt*k3, start_time + dt);

    TState f = state + (dt/6.0) * (k1 + 2*k2 + 2*k3 + k4);
    return f;
  }
};
}  // namespace GLOO

#endif
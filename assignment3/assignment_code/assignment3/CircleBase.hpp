#ifndef CIRCLE_BASE_H_
#define CIRCLE_BASE_H_

#include "ParticleState.hpp"
#include "ParticleSystemBase.hpp"

namespace GLOO {
class CircleBase : public ParticleSystemBase {
 public:
 
    ParticleState ComputeTimeDerivative(const ParticleState& state,
                                              float time) const {
                                                ParticleState f;
                                                f.positions.push_back(glm::vec3(0, 0, 0));
                                                f.velocities.push_back(glm::vec3(0, 0, 0));
                                                f.positions[0].x = -1*state.positions[0].y;
                                                f.positions[0].y = state.positions[0].x;
                                                f.positions[0].z = 0;
                                                return f; /* the derivative for circular motion */
                                              }
};
}  // namespace GLOO

#endif

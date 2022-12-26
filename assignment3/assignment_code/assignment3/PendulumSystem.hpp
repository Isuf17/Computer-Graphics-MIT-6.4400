#ifndef PENDULUM_SYSTEM_H
#define PENDULUM_SYSTEM_H

#include "ParticleSystemBase.hpp"
#include "ParticleState.hpp"
#include "Spring.hpp"

#include "glm/gtx/string_cast.hpp" /* to print vectors, matrices */

namespace GLOO {
class PendulumSystem : public ParticleSystemBase { 
  public:

  std::vector<float> Masses;
  std::vector<Spring> Springs;
  std::vector<int> Fixed;
  float drag_const;

  ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const {
    ParticleState f;
    std::vector<glm::vec3> Forces; /* temporary vector to keep the total force on each particle */
    std::vector<glm::vec3> Accelerations; /* temporary vector to keep acceleration of each particle */

    for (int i = 0; i < Masses.size(); i++) { 
        float w = Masses[i]*9.81;
        glm::vec3 weight = glm::vec3(0, -w, 0); /* gravity acts in y direction only */
        glm::vec3 drag = -drag_const*state.velocities[i]; /* viscous drag force */
        Forces.push_back(weight + drag);
    }

    for (int i = 0; i < Springs.size(); i++) {
        int sphere_index1 = Springs[i].sphere_indices[0];
        int sphere_index2 = Springs[i].sphere_indices[1];

        float spring_const = Springs[i].spring_const;
        float rest_length = Springs[i].rest_length;

        glm::vec3 d = state.positions[sphere_index1] - state.positions[sphere_index2];
        float norm_d = glm::length(d);

        glm::vec3 spring_force;
        if (norm_d == 0) {
            spring_force = glm::vec3(0, 0, 0);
        }
        else {
            spring_force = -spring_const*(norm_d - rest_length)*(d/norm_d);
        }

        Forces[sphere_index1] += spring_force;
        Forces[sphere_index2] -= spring_force;
    }

    for (int i = 0; i < Fixed.size(); i++) {
        int fixed_index = Fixed[i];
        Forces[fixed_index] = glm::vec3(0, 0, 0); /* fix some spheres so they don't just fall away */
    }

    for (int i = 0; i < Forces.size(); i++) {
        Accelerations.push_back(Forces[i]/Masses[i]); /* N2L to calculate acceleration */
    }

    f.positions = state.velocities;
    f.velocities = Accelerations; /* how to compute derivative for pendulum systems */
    
    return f;
  }

  void AddMass(float mass) { 
    Masses.push_back(mass);
  }

  void AddSpring(int sphere_index1, int sphere_index2, float spring_const, float rest_length) {
    auto spring = make_unique<Spring>();
    spring->sphere_indices = {sphere_index1, sphere_index2};
    spring->spring_const = spring_const;
    spring->rest_length = rest_length;
    Springs.push_back(*spring);
  }

  void Fix(int i) { 
    Fixed.push_back(i);
  } 

  void SetDragConst(float drag) {
    drag_const = drag;
  }
};
} // namespace GLOO
#endif
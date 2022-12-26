#ifndef PENDULUM_NODE_H_
#define PENDULUM_NODE_H_

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"
#include "../common/helpers.hpp"

#include "ParticleState.hpp"
#include "IntegratorBase.hpp"
#include "PendulumSystem.hpp"

namespace GLOO {
class PendulumNode : public SceneNode {
 public:
  PendulumNode(ParticleState state, IntegratorType type, float step, bool cloth);
  void Update(double delta_time) override;  

 private:
  void InitializePendulum();
  void InitializeCloth();
  int IndexOf(int i, int j);

  std::vector<SceneNode*> spheres_;
  std::shared_ptr<VertexObject> sphere_mesh_;  
  std::shared_ptr<VertexObject> segments_;
  std::shared_ptr<ShaderProgram> shader_;
  

  ParticleState reset_;
  ParticleState state_;
  float step_;
  float time_;
  std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator_;
  PendulumSystem system_;
};
}  // namespace GLOO

#include "PendulumNode.cpp"
#endif
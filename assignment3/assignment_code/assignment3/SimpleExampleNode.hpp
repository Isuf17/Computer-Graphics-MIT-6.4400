#ifndef SIMPLE_EXAMPLE_NODE_H_
#define SIMPLE_EXAMPLE_NODE_H_

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"

#include "ParticleState.hpp"
#include "IntegratorBase.hpp"
#include "CircleBase.hpp"

namespace GLOO {
class SimpleExampleNode : public SceneNode {
 public:
  SimpleExampleNode(ParticleState state, IntegratorType type, float step);
  void Update(double delta_time) override;  

 private:
  void InitializeSphere();
  SceneNode* sphere_node_;

  std::shared_ptr<VertexObject> sphere_mesh_;  
  std::shared_ptr<ShaderProgram> shader_;

  ParticleState state_;
  float step_;
  float time_;
  std::unique_ptr<IntegratorBase<CircleBase, ParticleState>> integrator_;
  CircleBase system_;
};
}  // namespace GLOO

#include "SimpleExampleNode.cpp"
#endif
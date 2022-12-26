#include "SimulationApp.hpp"
#include "SimpleExampleNode.hpp"
#include "PendulumNode.hpp"

#include "glm/gtx/string_cast.hpp"

#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/debug/AxisNode.hpp"


namespace GLOO {
SimulationApp::SimulationApp(const std::string& app_name,
                             glm::ivec2 window_size,
                             IntegratorType integrator_type,
                             float integration_step)
    : Application(app_name, window_size),
      integrator_type_(integrator_type),
      integration_step_(integration_step) {
}

void SimulationApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  auto camera_node = make_unique<ArcBallCameraNode>(45.f, 0.75f, 5.0f);
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  root.AddChild(make_unique<AxisNode>('A'));

  auto ambient_light = std::make_shared<AmbientLight>();
  ambient_light->SetAmbientColor(glm::vec3(0.2f));
  root.CreateComponent<LightComponent>(ambient_light);

  auto point_light = std::make_shared<PointLight>();
  point_light->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
  point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
  auto point_light_node = make_unique<SceneNode>();
  point_light_node->CreateComponent<LightComponent>(point_light);
  point_light_node->GetTransform().SetPosition(glm::vec3(0.0f, 2.0f, 4.f));
  root.AddChild(std::move(point_light_node));

  // Simple example set up
  
  auto state = ParticleState();
  state.positions.push_back(glm::vec3(1.0, 0.0, 0.0)); /* give particle valid position */
  state.velocities.push_back(glm::vec3(0.0, 0.0, 0.0)); /* velocity does not matter, so we can give all 0s */
  
  auto simple_example = make_unique<SimpleExampleNode>(state, integrator_type_, integration_step_);
  root.AddChild(std::move(simple_example)); /* add simple example to scene */

  // Pendulum set up
  
  auto pendulum_state = ParticleState();

  std::vector<glm::vec3> pendulum_positions_ = {glm::vec3(0.5, 0.0, 0), glm::vec3(1.0, 0.0, 0), glm::vec3(1.5, 0.0, 0), glm::vec3(2.0, 0.0, 0)};
  std::vector<glm::vec3> pendulum_velocities_ = {glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0)};

  pendulum_state.positions = pendulum_positions_;
  pendulum_state.velocities = pendulum_velocities_;

  auto pendulum = make_unique<PendulumNode>(pendulum_state, integrator_type_, integration_step_, false);
  root.AddChild(std::move(pendulum)); /* add pendulum to scene */

  // Cloth set up
  
  auto cloth_state = ParticleState();
  glm::vec3 origin = glm::vec3(-8.0, 0, 0);

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      glm::vec3 offset = origin + glm::vec3(j*1.0, 0, 0) + glm::vec3(0, -i*1.0, 0);
      cloth_state.positions.push_back(offset);
      cloth_state.velocities.push_back(glm::vec3(0, 0, 0));
    }
  }

  auto cloth = make_unique<PendulumNode>(cloth_state, integrator_type_, integration_step_, true);
  root.AddChild(std::move(cloth)); /* add cloth to scene */

}
}  // namespace GLOO

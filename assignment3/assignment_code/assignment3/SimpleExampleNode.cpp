#include "SimpleExampleNode.hpp"
#include "IntegratorFactory.hpp"

#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"

namespace GLOO { 
SimpleExampleNode::SimpleExampleNode(ParticleState state, IntegratorType type, float step)
        : SceneNode() {
    sphere_mesh_ = PrimitiveFactory::CreateSphere(0.05f, 25, 25);
    shader_ = std::make_shared<PhongShader>();

    state_ = state;
    step_ = step;
    time_ = 0.0;
    integrator_ = IntegratorFactory::CreateIntegrator<CircleBase, ParticleState>(type);
    system_ = CircleBase();

    InitializeSphere(); /* add sphere to scene */
}

void SimpleExampleNode::InitializeSphere() {
    auto sphere = make_unique<SceneNode>();
    sphere->CreateComponent<RenderingComponent>(sphere_mesh_);
    
    auto material_ = std::make_shared<Material>(Material::GetDefault());
    sphere->CreateComponent<MaterialComponent>(material_);

    sphere->CreateComponent<ShadingComponent>(shader_);

    sphere_node_ = sphere.get();
    AddChild(std::move(sphere));
}

void SimpleExampleNode::Update(double delta_time) {
    double num_steps = delta_time/step_;
    if (step_ <= delta_time) {
      for (int i = 0; i < num_steps; i++) {
         state_ = integrator_->Integrate(system_, state_, time_, step_); /* if the step_ is small enough, we can update position a number of times*/
         sphere_node_->GetTransform().SetPosition(state_.positions[0]);
         time_ += step_;
      }
    }
    else {
        state_ = integrator_->Integrate(system_, state_, time_, delta_time);
        sphere_node_->GetTransform().SetPosition(state_.positions[0]);
        time_ += float(delta_time);
    }
}
}  // namespace GLO
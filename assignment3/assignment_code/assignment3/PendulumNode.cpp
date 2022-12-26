#include "PendulumNode.hpp"
#include "IntegratorFactory.hpp"
#include "../common/helpers.hpp"

#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/InputManager.hpp"


namespace GLOO { 
PendulumNode::PendulumNode(ParticleState state, IntegratorType type, float step, bool cloth)
        : SceneNode() {
    sphere_mesh_ = PrimitiveFactory::CreateSphere(0.05f, 25, 25);
    shader_ = std::make_shared<PhongShader>();
    segments_ = std::make_shared<VertexObject>();
    reset_ = state;
    state_ = state;
    step_ = step;
    time_ = 0.0;
    integrator_ = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(type);
    system_ = PendulumSystem();

    if (cloth) {
        InitializeCloth(); /* add cloth to scene */
    }
    else {
        InitializePendulum(); /* add pendulum to scene */
    }
}

void PendulumNode::InitializePendulum() {
    for (int i = 0; i < state_.positions.size(); i++) {
        auto sphere = make_unique<SceneNode>();
        sphere->CreateComponent<RenderingComponent>(sphere_mesh_);
        
        auto material_ = std::make_shared<Material>(Material::GetDefault());
        sphere->CreateComponent<MaterialComponent>(material_);

        sphere->CreateComponent<ShadingComponent>(shader_);

        spheres_.push_back(sphere.get());

        system_.AddMass(5.0); /* add sphere to system with mass */

        AddChild(std::move(sphere));
    }
    system_.SetDragConst(1.5);

    for (int i = 0; i < state_.positions.size() - 1; i ++) {
        system_.AddSpring(i, i + 1, 200.0, 0.25);
    }
    system_.Fix(0);
    // system_.Fix(3);
}


void PendulumNode::InitializeCloth() { 
    for (int i = 0; i < state_.positions.size(); i++) {
        auto sphere = make_unique<SceneNode>();
        sphere->CreateComponent<RenderingComponent>(sphere_mesh_);
        
        auto material_ = std::make_shared<Material>(Material::GetDefault());
        sphere->CreateComponent<MaterialComponent>(material_);

        sphere->CreateComponent<ShadingComponent>(shader_);

        spheres_.push_back(sphere.get());

        system_.AddMass(5.0); /* add sphere to system with mass */

        AddChild(std::move(sphere));
    }
    system_.SetDragConst(70.0);
    system_.Fix(0);
    system_.Fix(7); /* fix corners to create draping effect */

    float structural_spring_const = 10000.0;
    float shear_spring_const = 100.0;
    float flex_spring_const = 200.0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i != 7) {
                system_.AddSpring(IndexOf(i, j), IndexOf(i + 1, j), structural_spring_const, 1.0);
            }
             if (j != 7) {
                system_.AddSpring(IndexOf(i, j), IndexOf(i, j + 1), structural_spring_const, 1.0); /* add structural springs */
            }
            if (i > 0 && j < 7) {
                system_.AddSpring(IndexOf(i - 1, j + 1), IndexOf(i, j), shear_spring_const, 1.4);
            }
            if (i < 7 && j < 7) {
                system_.AddSpring(IndexOf(i, j), IndexOf(i + 1, j + 1), shear_spring_const, 1.4); /* add shear springs */
            }
            if (i < 6) {
                system_.AddSpring(IndexOf(i, j), IndexOf(i + 2, j), flex_spring_const, 2.0);
            }
             if (j < 6) {
                system_.AddSpring(IndexOf(i, j), IndexOf(i, j + 2), flex_spring_const, 2.0); /* add flex springs */
            }
        }
    }

    auto lines = make_unique<SceneNode>(); /* draw lines */
    auto positions = make_unique<PositionArray>();
    auto indices = make_unique<IndexArray>();

    for (int i = 0; i < system_.Springs.size(); i++) {
        int index1 = system_.Springs[i].sphere_indices[0];
        int index2 = system_.Springs[i].sphere_indices[1];

        positions->push_back(state_.positions[index1]);
        positions->push_back(state_.positions[index2]);

        indices->push_back(positions->size() - 2);
        indices->push_back(positions->size() - 1);
    }
    auto normals = CalculateNormals(*positions, *indices);

    segments_->UpdateNormals(std::move(normals));
    segments_->UpdatePositions(std::move(positions));
    segments_->UpdateIndices(std::move(indices));

    auto material_ = CreateComponent<MaterialComponent>(std::make_shared<Material>());
    material_.GetMaterial().SetAmbientColor(glm::vec3(255, 255, 255));

    
    lines->CreateComponent<MaterialComponent>(material_);
    lines->CreateComponent<ShadingComponent>(shader_);

    auto& rc = lines->CreateComponent<RenderingComponent>(segments_);
    rc.SetDrawMode(DrawMode::Lines);

    AddChild(std::move(lines));
}

void PendulumNode::Update(double delta_time) {
    if (InputManager::GetInstance().IsKeyPressed('R')) {
        state_ = reset_; /* reset state to initial if R is pressed */
    }
    double num_steps = delta_time/step_;
    if (step_ <= delta_time) {
      for (int i = 0; i < num_steps; i++) {
        state_ = integrator_->Integrate(system_, state_, time_, step_); /* if the step_ is small enough, we can update position a number of times*/
        for (int j = 0; j < state_.positions.size(); j++) {
            auto sphere = spheres_[j];
            sphere->GetTransform().SetPosition(state_.positions[j]);
        }
        time_ += step_;
      }
    }
    else {
        state_ = integrator_->Integrate(system_, state_, time_, delta_time);
        for (int j = 0; j < state_.positions.size(); j++) {
            auto sphere = spheres_[j];
            sphere->GetTransform().SetPosition(state_.positions[j]);
        }
        time_ += delta_time;
    }

    auto positions = make_unique<PositionArray>();
    auto indices = make_unique<IndexArray>();

    for (int i = 0; i < system_.Springs.size(); i++) {
        int index1 = system_.Springs[i].sphere_indices[0];
        int index2 = system_.Springs[i].sphere_indices[1];

        positions->push_back(state_.positions[index1]);
        positions->push_back(state_.positions[index2]);

        indices->push_back(positions->size() - 2);
        indices->push_back(positions->size() - 1);
    }
    auto normals = CalculateNormals(*positions, *indices);

    segments_->UpdateNormals(std::move(normals));
    segments_->UpdatePositions(std::move(positions)); /* update the positions and normals of the springs every update */
}

int PendulumNode::IndexOf(int i, int j) {
    return j*8 + i; /* returns the correct indices of the spheres we are connecting a spring between */
}
}  // namespace GLOO
#include "SkeletonNode.hpp"

#include "gloo/utils.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/MeshLoader.hpp"

#include <fstream> /* include to read files */
#include <string> /* have strings */
#include "glm/gtx/string_cast.hpp" /* to print vectors, matrices */

#include "gloo/utils.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"


namespace GLOO {
SkeletonNode::SkeletonNode(const std::string& filename)
    : SceneNode(), draw_mode_(DrawMode::Skeleton) {
  
  shader_ = std::make_shared<PhongShader>();
  sphere_mesh_ = PrimitiveFactory::CreateSphere(0.02f, 12, 12);
  cylinder_mesh_ = PrimitiveFactory::CreateCylinder(0.01f, 1.f, 25); /* set appropriate meshes */
  
  LoadAllFiles(filename);

  for (int i = 0; i < Joints.size(); i++) {
    B_.push_back(glm::inverse(Joints[i]->GetTransform().GetLocalToWorldMatrix()));
    T_.push_back(Joints[i]->GetTransform().GetLocalToWorldMatrix()); /* save Bs and Ts as described */
  }

  DecorateTree();

  // Force initial update.
  OnJointChanged();
}

void SkeletonNode::ToggleDrawMode() {
  draw_mode_ =
      draw_mode_ == DrawMode::Skeleton ? DrawMode::SSD : DrawMode::Skeleton;
  if (draw_mode_ == DrawMode::SSD) {
    SSD->SetActive(true);
      for (int i = 0; i < Joints.size(); i++) {
      Joints[i]->SetActive(false);
      sphere_node_ptrs_[i]->SetActive(false);
    }
    for (int i = 0; i < Bones.size(); i++) { 
      Bones[i]->SetActive(false);
    }
  }
  else {
    SSD->SetActive(false); 
    for (int i = 0; i < Joints.size(); i++) {
      Joints[i]->SetActive(true);
      sphere_node_ptrs_[i]->SetActive(true);
    }
    for (int i = 0; i < Bones.size(); i++) { 
      Bones[i]->SetActive(true);
    }
  }
  return;
}

void SkeletonNode::DecorateTree() {
  auto material_ = std::make_shared<Material>(Material::GetDefault()); /* get appropriate material with colour for bones and joints */

  for (int i = 0; i < Joints.size(); i++) {
    auto joint = Joints[i];
    auto sphere_node = make_unique<SceneNode>();
    sphere_node->CreateComponent<ShadingComponent>(shader_);
    sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
    sphere_node->CreateComponent<MaterialComponent>(material_); /* give joints appropriate shapes and colours */

    if (joint->GetParentPtr() != Joints[0] && i > 0) { /* only add bones to joints with non root parent */
      auto bone_node = make_unique<SceneNode>();
      bone_node->CreateComponent<ShadingComponent>(shader_);
      bone_node->CreateComponent<RenderingComponent>(cylinder_mesh_);
      bone_node->CreateComponent<MaterialComponent>(material_);
      Bones.push_back(bone_node.get());
      joint->GetParentPtr()->AddChild(std::move(bone_node)); /* add bone to parent frame */
    }
    sphere_node_ptrs_.push_back(sphere_node.get());
    joint->AddChild(std::move(sphere_node)); /* add spheres to joints */
  }

  for (int i = 0; i < Bones.size(); i++) {
    auto bone = Bones[i];
    auto parent_joint = bone->GetParentPtr();

    if (parent_joint == Joints[0]) {
      continue; /* root does not need bones */
    }
    glm::vec3 parent_to_child = parent_joint->GetChild(0).GetTransform().GetWorldPosition() - parent_joint->GetTransform().GetWorldPosition(); /* vector from parent to child */
    auto l = glm::length(parent_to_child);
    bone->GetTransform().SetScale(glm::vec3(1.f, l, 1.f));
    
    glm::vec3 up = glm::normalize(parent_joint->GetTransform().GetWorldUp());
    glm::vec3 axis = glm::normalize(glm::cross(up, parent_to_child)); /* align parent to child */
    float theta = glm::acos(glm::dot(up, glm::normalize(parent_to_child))); 
    bone->GetTransform().SetRotation(axis, theta); /* set appropriate rotation */
  }

  auto SSD_ptr = make_unique<SceneNode>(); /* create the ssd node */
  SSD = SSD_ptr.get();
  SSD->SetActive(false); /* initally set to false, as specified */

  for (int i = 0; i < skin->GetPositions().size(); i++) {
    positions_.push_back(skin->GetPositions().at(i)); /* save all initial positions */
  }

  for (int i = 0; i < skin->GetIndices().size() - 2; i+=3) {
    int vert1 = skin->GetIndices().at(i);
    int vert2 = skin->GetIndices().at(i + 1);
    int vert3 = skin->GetIndices().at(i + 2);

    glm::vec3 edge1 = skin->GetPositions().at(vert2) - skin->GetPositions().at(vert1);
    glm::vec3 edge2 = skin->GetPositions().at(vert3) - skin->GetPositions().at(vert1);
    glm::vec3 normal = glm::cross(edge1, edge2); /* compute per face normal */

    weighted_norms[vert1].push_back(normal);
    weighted_norms[vert2].push_back(normal);
    weighted_norms[vert3].push_back(normal);
  }
  auto v_normals = make_unique<NormalArray>();
  for (int j = 0; j < skin->GetIndices().size(); j++) {
    int vertex = skin->GetIndices().at(j);
    std::vector<glm::vec3> adj_norms = weighted_norms[vertex];
    glm::vec3 norm;
    for (int k = 0; k < adj_norms.size(); k++) {
      norm += adj_norms[k]; /* once normalised, will be the weighted normal */
    }
    v_normals->push_back(glm::normalize(norm));
    normals_.push_back(glm::normalize(norm)); /* save initial normals */
  }
  skin->UpdateNormals(std::move(v_normals));
  SSD->CreateComponent<ShadingComponent>(shader_);
  SSD->CreateComponent<RenderingComponent>(skin);
  auto material = CreateComponent<MaterialComponent>(std::make_shared<Material>());
  material.GetMaterial().SetAmbientColor(glm::vec3(1, 2, 3));
  SSD->CreateComponent<MaterialComponent>(material);

  AddChild(std::move(SSD_ptr));
}

void SkeletonNode::Update(double delta_time) {
  // Prevent multiple toggle.
  static bool prev_released = true;
  if (InputManager::GetInstance().IsKeyPressed('S')) {
    if (prev_released) {
      ToggleDrawMode();
    }
    prev_released = false;
  } else if (InputManager::GetInstance().IsKeyReleased('S')) {
    prev_released = true;
  }
}

void SkeletonNode::OnJointChanged() {
  for (int i = 0; i < linked_angles_.size(); i++) {
    auto euler_angle = linked_angles_[i];
    auto joint = Joints[i];
    glm::quat Q = glm::quat(glm::vec3(euler_angle->rx, euler_angle->ry, euler_angle->rz)); /* create quaternion of rotations*/
    joint->GetTransform().SetRotation(Q); /* update joints */
  }

  for (int i = 0; i < Joints.size(); i++) {
    T_[i] = (Joints[i]->GetTransform().GetLocalToWorldMatrix()); /* update T after rotations */
  }

  auto new_positions = make_unique<PositionArray>();
  auto new_normals = make_unique<NormalArray>();

  for (int i = 0; i < skin->GetPositions().size(); i++) {
    glm::vec3 p = positions_[i];
    glm::vec4 p_new = glm::vec4(0);
    glm::vec3 normal = normals_[i];
    glm::mat4 M = glm::mat4(0);

    for (int j = 0; j < Joints.size(); j++) {
      p_new = p_new + Weights[i][j] * T_[j] * B_[j] * glm::vec4(p[0], p[1], p[2], 1); /* build new point */
      M = M + Weights[i][j] * T_[j] * B_[j]; /* build M */
    }
    glm::vec3 P = glm::vec3(p_new[0], p_new[1], p_new[2]);
    new_positions->push_back(P); /* get new positions */

    M = glm::inverse(glm::transpose(M));
    glm::vec4 new_normal = M * glm::vec4(normal[0], normal[1], normal[2], 0);;
    glm::vec3 N = glm::vec3(new_normal[0], new_normal[1], new_normal[2]);
    new_normals->push_back(N); /* get new normals */
  }
  skin->UpdatePositions(std::move(new_positions));
  skin->UpdateNormals(std::move(new_normals));
}

void SkeletonNode::LinkRotationControl(const std::vector<EulerAngle*>& angles) {
  linked_angles_ = angles;
}

void SkeletonNode::LoadSkeletonFile(const std::string& path) {
  std::string line; /* create empty string to use */
  std::fstream file(path); /* open file */
  while (std::getline(file, line)) { /* read line by line till none left */
    std::istringstream s(line); 
    float x;
    float y;
    float z;
    int parent_;
    s >> x >> y >> z >> parent_; /* put information of line into x, y, z and parent */

    auto joint = make_unique<SceneNode>(); /* create joint */
    joint->GetTransform().SetPosition(glm::vec3(x, y, z)); /* give joint correct position */
    Joints.push_back(joint.get());
    
    if (parent_ == -1) {
      AddChild(std::move(joint)); /* add root directly to scene */
    }
    else {
      auto parent_joint = Joints[parent_];
      parent_joint->AddChild(std::move(joint)); /* add children joints to parent joints */
    }
  }
}

void SkeletonNode::LoadMeshFile(const std::string& filename) {
  std::shared_ptr<VertexObject> vtx_obj =
      MeshLoader::Import(filename).vertex_obj;
  skin = vtx_obj; /* call mesh skin to make sense */
}

void SkeletonNode::LoadAttachmentWeights(const std::string& path) {
  std::string line;
  std::fstream file(path);
  while (std::getline(file, line)) {
    std::stringstream s(line);
    std::vector<float> weight_vector;
    weight_vector.push_back(0); /* push 0 so root has no effect */
    float w;
    while (s >> w) {
      weight_vector.push_back(w);
    }
    Weights.push_back(weight_vector);
  }
}

void SkeletonNode::LoadAllFiles(const std::string& prefix) {
  std::string prefix_full = GetAssetDir() + prefix;
  LoadSkeletonFile(prefix_full + ".skel");
  LoadMeshFile(prefix + ".obj");
  LoadAttachmentWeights(prefix_full + ".attach");
}
}  // namespace GLOO
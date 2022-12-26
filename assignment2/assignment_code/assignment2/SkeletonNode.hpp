#ifndef SKELETON_NODE_H_
#define SKELETON_NODE_H_

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"
#include "gloo/shaders/PhongShader.hpp"

#include <fstream> /* to read files */

#include <string>
#include <vector>
#include <map>

namespace GLOO {
class SkeletonNode : public SceneNode {
 public:
  enum class DrawMode { Skeleton, SSD };
  struct EulerAngle {
    float rx, ry, rz;
  };

  SkeletonNode(const std::string& filename);
  void LinkRotationControl(const std::vector<EulerAngle*>& angles);
  void Update(double delta_time) override;
  void OnJointChanged();

 private:
  void LoadAllFiles(const std::string& prefix);
  void LoadSkeletonFile(const std::string& path);
  void LoadMeshFile(const std::string& filename);
  void LoadAttachmentWeights(const std::string& path);
  void ToggleDrawMode();
  void DecorateTree();

  std::vector<SceneNode*> Joints;
  std::vector<SceneNode*> sphere_node_ptrs_;
  std::vector<SceneNode*> Bones;
  std::vector<std::vector<float>> Weights;

  std::vector<glm::mat4> B_;
  std::vector<glm::mat4> T_;
  std::vector<glm::vec3> positions_;
  std::map<int, std::vector<glm::vec3>> weighted_norms;
  std::vector<glm::vec3> normals_;

  SceneNode* SSD;
  
  std::shared_ptr<ShaderProgram> shader_;
  std::shared_ptr<VertexObject> sphere_mesh_;
  std::shared_ptr<VertexObject> cylinder_mesh_;
  std::shared_ptr<VertexObject> skin;

  DrawMode draw_mode_;
  // Euler angles of the UI sliders.
  std::vector<EulerAngle*> linked_angles_;
};
}  // namespace GLOO

#endif

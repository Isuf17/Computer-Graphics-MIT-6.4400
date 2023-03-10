#ifndef PATCH_NODE_H_
#define PATCH_NODE_H_

#include <string>
#include <vector>

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"

#include "CurveNode.hpp"

namespace GLOO {
struct PatchPoint {
  glm::vec3 P;
  glm::vec3 N;
};

class PatchNode : public SceneNode {
 public:
  PatchNode(std::vector<glm::vec3> control_points, SplineBasis spline_basis);

 private:
  PatchPoint EvalPatch(float u, float v);
  void PlotPatch();
  void InitPatch();
  
  std::vector<glm::mat4> Gs_;
  SplineBasis spline_basis_;

  glm::mat4 BBezier;
  glm::mat4 BBSpline;
  glm::mat4 TBezier;
  glm::mat4 TBSpline;

  std::shared_ptr<VertexObject> patch_mesh_;
  std::shared_ptr<ShaderProgram> shader_;

  const int N_SUBDIV_ = 50;
};
}  // namespace GLOO

#endif

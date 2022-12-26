#include "PatchNode.hpp"

#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"

namespace GLOO {
PatchNode::PatchNode(std::vector<glm::vec3> control_points, SplineBasis spline_basis) {
  shader_ = std::make_shared<PhongShader>();
  patch_mesh_ = std::make_shared<VertexObject>();

  // TODO: this node should represent a single tensor product patch.
  // Think carefully about what data defines a patch and how you can
  // render it.

  spline_basis_ = spline_basis;
  BBezier = glm::mat4(1, 0, 0, 0, -3, 3, 0, 0, 3, -6, 3, 0, -1, 3, -3, 1);

  BBSpline = (1.0f / 6.0f) * glm::mat4(1, 4, 1, 0, -3, 0, 3, 0, 3, -6, 3, 0, -1, 3, -3, 1);

  TBezier = glm::mat4(-3, 3, 0, 0, 6, -12, 6, 0, -3, 9, -9, 3, 0, 0, 0, 0);

  TBSpline = (1.0f / 6.0f) * glm::mat4(-3, 0, 3, 0, 6, -12, 6, 0, -3, 9, -9, 3, 0, 0, 0, 0);

  Gs_.resize(3);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      int index = i*4 + j;
      Gs_[0][i][j] = control_points[index].x;
      Gs_[1][i][j] = control_points[index].y;
      Gs_[2][i][j] = control_points[index].z;
    }
  }

  PlotPatch();
  InitPatch();

}
PatchPoint PatchNode::EvalPatch(float u, float v) {
	const glm::mat4 B = (spline_basis_ == SplineBasis::Bezier) ? BBezier : BBSpline;
	const glm::mat4 TB = (spline_basis_ == SplineBasis::Bezier) ? TBezier : TBSpline;

	glm::vec4 MonU(1.0, u, pow(u, 2.0), pow(u, 3.0));
	glm::vec4 MonV(1.0, v, pow(v, 2.0), pow(v, 3.0));

	float p_x = glm::dot(MonU * glm::transpose(B), Gs_[0] * B * MonV);
	float p_y = glm::dot(MonU * glm::transpose(B), Gs_[1] * B * MonV);
	float p_z = glm::dot(MonU * glm::transpose(B), Gs_[2] * B * MonV);
	

	float dU_x = glm::dot(MonU * glm::transpose(TB), Gs_[0] * B * MonV);
	float dU_y = glm::dot(MonU * glm::transpose(TB), Gs_[1] * B * MonV);
	float dU_z = glm::dot(MonU * glm::transpose(TB), Gs_[2] * B * MonV);
	

	float dV_x = glm::dot(MonU * glm::transpose(B), Gs_[0] * TB * MonV);
	float dV_y = glm::dot(MonU * glm::transpose(B), Gs_[1] * TB * MonV);
	float dV_z = glm::dot(MonU * glm::transpose(B), Gs_[2] * TB * MonV);
	

	glm::vec3 point{ p_x, p_y, p_z };
  glm::vec3 dU{ dU_x, dU_y, dU_z };
  glm::vec3 dV{ dV_x, dV_y, dV_z };

	glm::vec3 N = glm::cross(dU, dV);
	glm::vec3 normN = glm::normalize(N);

	auto patch_point_ = PatchPoint();
	patch_point_.P = point;
	patch_point_.N = normN;

	return patch_point_;
}

void PatchNode::InitPatch() {
	CreateComponent<ShadingComponent>(shader_);
	CreateComponent<RenderingComponent>(patch_mesh_);
	CreateComponent<MaterialComponent>(std::make_shared<Material>(Material::GetDefault()));
}
void PatchNode::PlotPatch() {
  auto positions = make_unique<PositionArray>();
  auto normals = make_unique<NormalArray>();
  auto indices = make_unique<IndexArray>();

  float w = 1.0f / N_SUBDIV_;
	for (int i = 0; i < N_SUBDIV_; i++) {
		for (int j = 0; j < N_SUBDIV_; j++) {
			float u = (i + 1) * w;
			float v = j * w;
			PatchPoint p1 = EvalPatch(u, v);
			PatchPoint p2 = EvalPatch(u, v + w);
			PatchPoint p3 = EvalPatch(u - w, v);
			PatchPoint p4 = EvalPatch(u -  w, v + w);
			positions->push_back(p1.P);
			positions->push_back(p2.P);
			positions->push_back(p3.P);
			positions->push_back(p4.P);

			normals->push_back(p1.N);
			normals->push_back(p2.N);
			normals->push_back(p3.N);
			normals->push_back(p4.N);

			int index = (i*N_SUBDIV_ + j) * 4;
			indices->push_back(index);
			indices->push_back(index + 1);
			indices->push_back(index + 2);
			indices->push_back(index + 2);
			indices->push_back(index + 1);
			indices->push_back(index + 3);
		}
	}

  patch_mesh_->UpdatePositions(std::move(positions));
  patch_mesh_->UpdateNormals(std::move(normals));
  patch_mesh_->UpdateIndices(std::move(indices));
}
}
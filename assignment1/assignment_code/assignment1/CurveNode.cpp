#include "CurveNode.hpp"

#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/InputManager.hpp"

namespace GLOO {
CurveNode::CurveNode(const glm::mat4x3 ControlPoints, SplineBasis TypeOfCurve, bool toggle) {
  // TODO: this node should represent a single spline curve.
  // Think carefully about what data defines a curve and how you can
  // render it.

  BezierMatrix = glm::mat4(1.0, 0.0, 0.0, 0.0, -3.0, 3.0, 0.0, 0.0, 3.0, -6.0, 3.0, 0.0, -1.0, 3.0, -3.0, 1.0);
  BSplineMatrix  = glm::mat4(1.0/6.0, 4.0/6.0, 1.0/6.0, 0.0, -3.0/6.0, 0.0, 3.0/6.0, 0.0, 3.0/6.0, -6.0/6.0,
                         3.0/6.0, 0, -1.0/6.0, 3.0/6.0, -3.0/6.0, 1.0/6.0);
  // Initialize the VertexObjects and shaders used to render the control points,
  // the curve, and the tangent line.
  sphere_mesh_ = PrimitiveFactory::CreateSphere(0.015f, 25, 25);
  curve_polyline_ = std::make_shared<VertexObject>();
  tangent_line_ = std::make_shared<VertexObject>();
  shader_ = std::make_shared<PhongShader>();
  polyline_shader_ = std::make_shared<SimpleShader>();

  control_points_ = ControlPoints;
  spline_basis_ = TypeOfCurve;
  toggle_ = toggle;

  PlotCurve();
  InitCurve();
  PlotControlPoints();
  PlotTangentLine();
}

void CurveNode::Update(double delta_time) {

  if (toggle_) {
  
  // Prevent multiple toggle.
  static bool prev_released = true;

  if (InputManager::GetInstance().IsKeyPressed('T')) {
    if (prev_released) {
      // TODO: implement toggling spline bases.
        ToggleSplineBasis();
    }
    prev_released = false;
  } else if (InputManager::GetInstance().IsKeyPressed('B')) {
    if (prev_released) {
      // TODO: implement converting conrol point geometry from Bezier to
      // B-Spline basis.
        ConvertGeometry(SplineBasis::BSpline);
        PlotControlPoints();
        PlotCurve();
        PlotTangentLine();
    }
    prev_released = false;
  } else if (InputManager::GetInstance().IsKeyPressed('Z')) {
    if (prev_released) {
      // TODO: implement converting conrol point geometry from B-Spline to
      // Bezier basis.
        ConvertGeometry(SplineBasis::Bezier);
        PlotControlPoints();
        PlotCurve();
        PlotTangentLine();
    }
    prev_released = false;
  } else {
    prev_released = true;
  }
}
}

void CurveNode::ToggleSplineBasis() {
  // TODO: implement toggling between Bezier and B-Spline bases.
  if (spline_basis_ == SplineBasis::Bezier) {
    spline_basis_ = SplineBasis::BSpline;
  }
  else {
    spline_basis_ = SplineBasis::Bezier;
  }
  PlotControlPoints();
  PlotCurve();
  PlotTangentLine();
}

void CurveNode::ConvertGeometry(SplineBasis basis) {
  // TODO: implement converting the control points between bases.
  glm::mat4 B = (basis == SplineBasis::Bezier) ?  BSplineMatrix : BezierMatrix;
  glm::mat4 Binv = (basis == SplineBasis::Bezier) ? inverse(BezierMatrix) : inverse(BSplineMatrix);
  control_points_ = control_points_*B*Binv;
}

CurvePoint CurveNode::EvalCurve(float t) {
  // TODO: implement evaluating the spline curve at parameter value t.
  auto curve_point_ = CurvePoint();
  auto Monomial = glm::vec4 (1.0, t, pow(t, 2.0), pow(t, 3.0));
  auto Velocity = glm::vec4 (0, 1.0, 2.0*t, 3*pow(t, 2.0));

  curve_point_.P = (spline_basis_ == SplineBasis::Bezier) ? control_points_*(BezierMatrix*Monomial) : control_points_*(BSplineMatrix*Monomial);
  curve_point_.T = (spline_basis_ == SplineBasis::Bezier) ? control_points_*(BezierMatrix*Velocity) : control_points_*(BSplineMatrix*Velocity);

  return curve_point_;
}

void CurveNode::InitCurve() {
  // TODO: create all of the  nodes and components necessary for rendering the
  // curve, its control points, and its tangent line. You will want to use the
  // VertexObjects and shaders that are initialized in the class constructor.
  
  for (int i = 0; i < 4; i++) {
    auto control_point_node = make_unique<SceneNode>();
    control_point_node->CreateComponent<ShadingComponent>(shader_);
    control_point_node->CreateComponent<RenderingComponent>(sphere_mesh_);
    // control_point_node->GetComponentPtr<RenderingComponent>()->SetDrawMode(DrawMode::Triangles);
    if (spline_basis_ == SplineBasis::Bezier) {
        glm::vec3 color(1.0f, 0, 0);
        auto material = std::make_shared<Material>(color, color, color, 0);
        control_point_node->CreateComponent<MaterialComponent>(material);
    }
    else {
        glm::vec3 color(0, 1.0f, 0);
        auto material = std::make_shared<Material>(color, color, color, 0);
        control_point_node->CreateComponent<MaterialComponent>(material);
    }
    position_.push_back(control_point_node.get());
    AddChild(std::move(control_point_node));
  }

  auto line_node = make_unique<SceneNode>();
  line_node->CreateComponent<ShadingComponent>(polyline_shader_);

  auto& rc = line_node->CreateComponent<RenderingComponent>(curve_polyline_);
  rc.SetDrawMode(DrawMode::Lines);

  glm::vec3 color(1.f, 1.f, 0);
  auto material = std::make_shared<Material>(color, color, color, 0);
  line_node->CreateComponent<MaterialComponent>(material);
  AddChild(std::move(line_node));  

}

void CurveNode::PlotCurve() {
  // TODO: plot the curve by updating the positions of its VertexObject.
  auto positions = make_unique<PositionArray>();
  auto indices = make_unique<IndexArray>();

  float i = 0;
  while (i < N_SUBDIV_) {
    CurvePoint point = EvalCurve(i/N_SUBDIV_);
    positions->push_back(point.P);
    indices->push_back(i);
    indices->push_back(i+1);
    i++;
  }

  CurvePoint final = EvalCurve(1.0);
  positions->push_back(final.P);

  curve_polyline_->UpdatePositions(std::move(positions));
  curve_polyline_->UpdateIndices(std::move(indices));
}

void CurveNode::PlotControlPoints() {
  // TODO: plot the curve control points.
  for (int i = 0; i < 4; i ++) {
    if (spline_basis_ == SplineBasis::Bezier) {
        position_[i]->GetComponentPtr<MaterialComponent>()->GetMaterial().SetAmbientColor(glm::vec3(1.0f, 0, 0));
    }
    else { 
        position_[i]->GetComponentPtr<MaterialComponent>()->GetMaterial().SetAmbientColor(glm::vec3(0, 1.0f, 0));
    }
    position_[i]->GetTransform().SetPosition(control_points_[i]);
  }
}

void CurveNode::PlotTangentLine() {
  // TODO: implement plotting a line tangent to the curve.

  CurvePoint intersect = EvalCurve(0.5f);
  glm::vec3 before = intersect.P - 0.2f*intersect.T;
  glm::vec3 after = intersect.P + 0.2f*intersect.T;

  auto positions = make_unique<PositionArray>();
  positions->push_back(before);
  positions->push_back(after);

  auto indices = make_unique<IndexArray>();
  indices->push_back(0);
  indices->push_back(1);

  tangent_line_->UpdatePositions(std::move(positions));
  tangent_line_->UpdateIndices(std::move(indices));

  auto shader = std::make_shared<SimpleShader>();

  auto line_node = make_unique<SceneNode>();
  line_node->CreateComponent<ShadingComponent>(shader);

  auto& rc = line_node->CreateComponent<RenderingComponent>(tangent_line_);
  rc.SetDrawMode(DrawMode::Lines);

  glm::vec3 color(1.f, 1.f, 1.f);
  auto material = std::make_shared<Material>(color, color, color, 0);
  line_node->CreateComponent<MaterialComponent>(material);

  AddChild(std::move(line_node));
}
}  // namespace GLOO

#include "Plane.hpp"

namespace GLOO {
Plane::Plane(const glm::vec3& normal, float d) {
  d_ = -d;
  normal_ = normal;
}

bool Plane::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  float t = -1.0*(d_ + glm::dot(normal_, ray.GetOrigin()))/glm::dot(normal_, ray.GetDirection());

  if (t > t_min && t <= record.time) {
    record.time = t;
    record.normal = normal_;
    return true;
  }
  return false;
}
}  // namespace GLOO

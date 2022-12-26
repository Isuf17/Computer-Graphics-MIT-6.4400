#include "Triangle.hpp"

#include <iostream>
#include <stdexcept>

#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Plane.hpp"

namespace GLOO {
Triangle::Triangle(const glm::vec3& p0,
                   const glm::vec3& p1,
                   const glm::vec3& p2,
                   const glm::vec3& n0,
                   const glm::vec3& n1,
                   const glm::vec3& n2) {
                    positions_.push_back(p0);
                    positions_.push_back(p1);
                    positions_.push_back(p2);
                    normals_.push_back(n0);
                    normals_.push_back(n1);
                    normals_.push_back(n2);
}

Triangle::Triangle(const std::vector<glm::vec3>& positions,
                   const std::vector<glm::vec3>& normals) {
                    positions_ = positions;
                    normals_ = normals;
}

bool Triangle::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  float A_11 = GetPosition(0).x - GetPosition(1).x;
  float A_12 = GetPosition(0).x - GetPosition(2).x;
  float A_13 = ray.GetDirection().x;
  float A_21 = GetPosition(0).y - GetPosition(1).y;
  float A_22 = GetPosition(0).y - GetPosition(2).y;
  float A_23 = ray.GetDirection().y;
  float A_31 = GetPosition(0).z - GetPosition(1).z;
  float A_32 = GetPosition(0).z - GetPosition(2).z;
  float A_33 = ray.GetDirection().z;

  glm::mat3 A(A_11, A_12, A_13,
              A_21, A_22, A_23,
              A_31, A_32, A_33);

  glm::vec3 b = GetPosition(0) - ray.GetOrigin();	
  glm::vec3 x = b*glm::inverse(A);
  float beta = x.x;
  float gamma = x.y;
  float t = x.z;

  if (beta + gamma > 1 || beta < 0 || gamma < 0) {
    return false;
  }  

  if (t >= t_min && t < record.time) {
    record.time = t;
    record.normal = glm::normalize((1.0f - beta - gamma)*GetNormal(0) + beta*GetNormal(1) + gamma*GetNormal(2));
    return true;
  }
  return false;
}
}  // namespace GLOO

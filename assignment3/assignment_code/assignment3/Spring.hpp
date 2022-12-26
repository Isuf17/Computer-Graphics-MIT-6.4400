#ifndef SPRING_H
#define SPRING_H

namespace GLOO {
struct Spring { 
  std::vector<int> sphere_indices; /* indices of the spheres that the spring is attached to */  
  float spring_const;
  float rest_length;
};
} // namespace GLOO

#endif

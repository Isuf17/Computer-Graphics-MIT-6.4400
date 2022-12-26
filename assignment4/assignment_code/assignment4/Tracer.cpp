#include "Tracer.hpp"

#include <glm/gtx/string_cast.hpp>
#include <stdexcept>
#include <algorithm>

#include "gloo/Transform.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/lights/AmbientLight.hpp"

#include "gloo/Image.hpp"
#include "Illuminator.hpp"

namespace GLOO {
void Tracer::Render(const Scene& scene, const std::string& output_file) {
  scene_ptr_ = &scene;

  auto& root = scene_ptr_->GetRootNode();
  tracing_components_ = root.GetComponentPtrsInChildren<TracingComponent>();
  light_components_ = root.GetComponentPtrsInChildren<LightComponent>();


  Image image(image_size_.x, image_size_.y);

  for (size_t y = 0; y < image_size_.y; y++) {
    for (size_t x = 0; x < image_size_.x; x++) {

      float i = (2*float(x)/(image_size_.x - 1)) - 1;
      float j = (2*float(y)/(image_size_.y - 1)) - 1;

      glm::vec2 coord = glm::vec2(i, j);
      Ray ray = camera_.GenerateRay(coord);
      HitRecord record;

      glm::vec3 colour = TraceRay(ray, max_bounces_, record);
      // std::cout << x << std::endl;
      image.SetPixel(x, y, colour);
    }
  }

  if (output_file.size())
    image.SavePNG(output_file);
}


glm::vec3 Tracer::TraceRay(const Ray& ray,
                           size_t bounces,
                           HitRecord& record) const {
  int closest_index = -1;

  for (int i = 0; i < tracing_components_.size(); i++) {
    glm::mat4 transform = tracing_components_[i]->GetNodePtr()->GetTransform().GetLocalToWorldMatrix();
    
    Ray ray_ = ray;
    ray_.ApplyTransform(glm::inverse(transform));
    bool interesect = tracing_components_[i]->GetHittable().Intersect(ray_, camera_.GetTMin(), record);

    if (interesect) {
      closest_index = i;
      glm::vec4 mod_norm = glm::transpose(glm::inverse(transform)) * glm::vec4(record.normal, 0.f);
      glm::vec3 new_norm = glm::vec3(mod_norm.x, mod_norm.y, mod_norm.z);
      glm::vec3 n_norm = glm::normalize(new_norm);
      record.normal = n_norm;
    }
  }

  if (closest_index == -1) {
    return GetBackgroundColor(ray.GetDirection());
  }
  else {
      glm::vec3 diffuse_ = tracing_components_[closest_index]->GetNodePtr()->GetComponentPtr<MaterialComponent>()->GetMaterial().GetDiffuseColor();
      glm::vec3 specular_ = tracing_components_[closest_index]->GetNodePtr()->GetComponentPtr<MaterialComponent>()->GetMaterial().GetSpecularColor();
      float shininess = tracing_components_[closest_index]->GetNodePtr()->GetComponentPtr<MaterialComponent>()->GetMaterial().GetShininess();

      glm::vec3 colour = glm::vec3(0, 0, 0);

      glm::vec3 hit_position = ray.At(record.time);
      glm::vec3 direction_to_light;
      glm::vec3 illumination_intensity;
      float distance_to_light;

      glm::vec3 reflected_ray_eye = ray.GetDirection() - (2*glm::dot(ray.GetDirection(), record.normal) * record.normal);

      for (int i = 0; i < light_components_.size(); i ++) {
        glm::vec3 ambient_component = glm::vec3(0, 0, 0);
        glm::vec3 diffuse_component = glm::vec3(0, 0, 0);
        glm::vec3 specular_component = glm::vec3(0, 0, 0);

        if (light_components_[i]->GetLightPtr()->GetType() == LightType::Ambient) {
          glm::vec3 ambient_light = static_cast<AmbientLight*>(light_components_[i]->GetLightPtr())->GetAmbientColor();
          ambient_component = ambient_light*diffuse_;
        }

        else { 
          Illuminator::GetIllumination(*light_components_[i], hit_position, direction_to_light, illumination_intensity, distance_to_light);

          if (glm::dot(direction_to_light, record.normal) > 0) {
            diffuse_component = glm::dot(direction_to_light, record.normal)*illumination_intensity*diffuse_;
          }

          if (glm::dot(direction_to_light, reflected_ray_eye) > 0) {
            auto value = pow(glm::dot(direction_to_light, reflected_ray_eye), shininess);
            specular_component = value*illumination_intensity*specular_;
          }
          else {
            if (shininess == 0) {
              specular_component = illumination_intensity*specular_;
            }
          }

          if (Tracer::shadows_enabled_) {
            glm::vec3 surface_point = hit_position + 0.001f*direction_to_light;
            Ray shadow_ray = Ray(surface_point, direction_to_light);
            HitRecord shadow_record;

            for (int i = 0; i < tracing_components_.size(); i++) {
              glm::mat4 transform = tracing_components_[i]->GetNodePtr()->GetTransform().GetLocalToWorldMatrix();
              Ray shadow_ray_c = shadow_ray;
              shadow_ray_c.ApplyTransform(glm::inverse(transform));
              bool intersect_ = tracing_components_[i]->GetHittable().Intersect(shadow_ray_c, camera_.GetTMin(), shadow_record);

              if (intersect_) {
                if (shadow_record.time < distance_to_light) {
                  diffuse_component = glm::vec3(0, 0, 0);
                  specular_component = glm::vec3(0, 0, 0);
                }
              }
            }
          }
        }
        
        colour += ambient_component + diffuse_component + specular_component;
      }

      if (bounces > 0) {
        glm::vec3 intersection = hit_position + 0.001f*reflected_ray_eye;
        Ray perfect = Ray(intersection, reflected_ray_eye);
        HitRecord new_record;
        colour += specular_*TraceRay(perfect, bounces - 1, new_record);
      }

      return colour;
  }
}


glm::vec3 Tracer::GetBackgroundColor(const glm::vec3& direction) const {
  if (cube_map_ != nullptr) {
    return cube_map_->GetTexel(direction);
  } else
    return background_color_;
}
}  // namespace GLOO

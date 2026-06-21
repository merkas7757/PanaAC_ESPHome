#pragma once
#include <string>
namespace esphome {
class EntityBase {
 public:
  EntityBase() = default;
  std::string name;
  std::string object_id;
  std::string unique_id;
  bool disabled_by_default{false};
};
}  // namespace esphome
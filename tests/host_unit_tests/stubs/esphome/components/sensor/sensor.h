#pragma once
#include "esphome/core/entity_base.h"
namespace esphome::sensor {
class Sensor : public EntityBase {
 public:
  Sensor() = default;
  float state{0};
  template<typename F> void add_on_state_callback(F &&) {}
};
}  // namespace esphome::sensor
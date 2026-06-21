#pragma once
#include <string>
#include <cstddef>
#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"
#include "esphome/components/select/select_traits.h"
namespace esphome::select {
class Select : public EntityBase {
 public:
  Select() = default;
  virtual ~Select() = default;
  void publish_state(const std::string &) {}   // no-op on host
  void publish_state(const char *) {}
  void publish_state(size_t) {}
  virtual void control(const std::string &value) {}
  SelectTraits traits;
  std::string state;
};
}  // namespace esphome::select
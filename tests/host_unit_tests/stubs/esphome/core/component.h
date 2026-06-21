#pragma once
#include <cstdint>
namespace esphome {
class Component {
 public:
  Component() = default;
  virtual ~Component() = default;
  virtual void setup() {}
  virtual void loop() {}
  virtual void dump_config() {}
  virtual void call_setup() { this->setup(); }
  virtual void call_loop() { this->loop(); }
  uint32_t get_component_state() const { return 0; }
  void set_component_state(uint32_t) {}
};
}  // namespace esphome
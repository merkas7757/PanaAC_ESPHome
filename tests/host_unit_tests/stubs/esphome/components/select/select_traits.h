#pragma once
#include <initializer_list>
#include "esphome/core/helpers.h"
namespace esphome::select {
class SelectTraits {
 public:
  void set_options(const std::initializer_list<const char *> &options) {
    this->options_.init(options.size());
    for (const char *o : options) this->options_.push_back(o);
  }
  void set_options(const FixedVector<const char *> &options) {
    this->options_.init(options.size());
    for (size_t i = 0; i < options.size(); i++) this->options_.push_back(options[i]);
  }
  const FixedVector<const char *> &get_options() const { return this->options_; }
 protected:
  FixedVector<const char *> options_;
};
}  // namespace esphome::select
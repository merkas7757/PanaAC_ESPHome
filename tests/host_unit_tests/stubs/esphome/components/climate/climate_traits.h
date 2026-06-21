#pragma once
#include <cstdint>
#include <initializer_list>
#include "esphome/components/climate/climate_mode.h"

namespace esphome::climate {

// Minimal bitmask mask type supporting initializer-list construction and insert(),
// matching the API surface panaac uses (ClimateIR ctor + ClimateTraits setters).
template<typename Enum> class FiniteSetMask {
 public:
  FiniteSetMask() = default;
  FiniteSetMask(std::initializer_list<Enum> il) {
    for (Enum e : il) this->insert(e);
  }
  void insert(Enum e) { this->bits_ |= (uint32_t{1} << static_cast<uint32_t>(e)); }
  bool has(Enum e) const { return (this->bits_ >> static_cast<uint32_t>(e)) & 1; }
  uint32_t raw() const { return this->bits_; }
 private:
  uint32_t bits_{0};
};

using ClimateModeMask = FiniteSetMask<ClimateMode>;
using ClimateFanModeMask = FiniteSetMask<ClimateFanMode>;
using ClimateSwingModeMask = FiniteSetMask<ClimateSwingMode>;
using ClimatePresetMask = FiniteSetMask<ClimatePreset>;

class ClimateTraits {
 public:
  ClimateTraits() = default;

  void add_feature_flags(uint32_t f) { this->feature_flags_ |= f; }
  void clear_feature_flags(uint32_t f) { this->feature_flags_ &= ~f; }
  uint32_t get_feature_flags() const { return this->feature_flags_; }

  void set_supported_modes(ClimateModeMask m) { this->supported_modes_ = m; }
  void add_supported_mode(ClimateMode m) { this->supported_modes_.insert(m); }
  void set_supported_fan_modes(ClimateFanModeMask m) { this->supported_fan_modes_ = m; }
  void add_supported_fan_mode(ClimateFanMode m) { this->supported_fan_modes_.insert(m); }
  void set_supported_swing_modes(ClimateSwingModeMask m) { this->supported_swing_modes_ = m; }
  void add_supported_swing_mode(ClimateSwingMode m) { this->supported_swing_modes_.insert(m); }
  void set_supported_presets(ClimatePresetMask m) { this->supported_presets_ = m; }

  void set_visual_min_temperature(float v) { this->visual_min_ = v; }
  void set_visual_max_temperature(float v) { this->visual_max_ = v; }
  void set_visual_temperature_step(float v) { this->visual_step_ = v; }
  void set_visual_target_temperature_step(float v) {}
  void set_visual_current_temperature_step(float v) {}

 private:
  uint32_t feature_flags_{0};
  ClimateModeMask supported_modes_{};
  ClimateFanModeMask supported_fan_modes_{};
  ClimateSwingModeMask supported_swing_modes_{};
  ClimatePresetMask supported_presets_{};
  float visual_min_{0}, visual_max_{0}, visual_step_{1};
};

}  // namespace esphome::climate
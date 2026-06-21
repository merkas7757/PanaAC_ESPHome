#pragma once
#include <cmath>
#include <string>
#include "esphome/core/optional.h"
#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "climate_mode.h"
#include "climate_traits.h"

namespace esphome::climate {

struct ClimateDeviceRestoreState {};  // opaque; test never uses restore

class Climate : public EntityBase {
 public:
  Climate() = default;
  virtual ~Climate() = default;

  void publish_state() {}  // no-op on host

  // Restore used only by ClimateIR::setup(), which the test never calls.
  optional<ClimateDeviceRestoreState> restore_state_() { return optional<ClimateDeviceRestoreState>(); }

  virtual ClimateTraits traits() { return ClimateTraits(); }

  ClimateMode mode{CLIMATE_MODE_OFF};
  float target_temperature{NAN};
  optional<ClimateFanMode> fan_mode{};
  ClimateSwingMode swing_mode{CLIMATE_SWING_OFF};
  float current_temperature{NAN};
};

}  // namespace esphome::climate
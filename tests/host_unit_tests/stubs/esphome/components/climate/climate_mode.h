#pragma once
#include <cstdint>
#include "esphome/core/log.h"
// Verbatim enum values from esphome/components/climate/climate_mode.h so the component
// under test sees identical enum names/values as on the real platform.
namespace esphome::climate {

enum ClimateMode : uint8_t {
  CLIMATE_MODE_OFF = 0,
  CLIMATE_MODE_HEAT_COOL = 1,
  CLIMATE_MODE_COOL = 2,
  CLIMATE_MODE_HEAT = 3,
  CLIMATE_MODE_FAN_ONLY = 4,
  CLIMATE_MODE_DRY = 5,
  CLIMATE_MODE_AUTO = 6,
};

enum ClimateAction : uint8_t {
  CLIMATE_ACTION_OFF = 0, CLIMATE_ACTION_COOLING = 2, CLIMATE_ACTION_HEATING = 3,
  CLIMATE_ACTION_IDLE = 4, CLIMATE_ACTION_DRYING = 5, CLIMATE_ACTION_FAN = 6,
  CLIMATE_ACTION_DEFROSTING = 7,
};

enum ClimateFanMode : uint8_t {
  CLIMATE_FAN_ON = 0, CLIMATE_FAN_OFF = 1, CLIMATE_FAN_AUTO = 2, CLIMATE_FAN_LOW = 3,
  CLIMATE_FAN_MEDIUM = 4, CLIMATE_FAN_HIGH = 5, CLIMATE_FAN_MIDDLE = 6, CLIMATE_FAN_FOCUS = 7,
  CLIMATE_FAN_DIFFUSE = 8, CLIMATE_FAN_QUIET = 9,
};

enum ClimateSwingMode : uint8_t {
  CLIMATE_SWING_OFF = 0, CLIMATE_SWING_BOTH = 1,
  CLIMATE_SWING_VERTICAL = 2, CLIMATE_SWING_HORIZONTAL = 3,
};

enum ClimatePreset : uint8_t {
  CLIMATE_PRESET_NONE = 0, CLIMATE_PRESET_HOME = 1, CLIMATE_PRESET_AWAY = 2,
  CLIMATE_PRESET_BOOST = 3, CLIMATE_PRESET_COMFORT = 4, CLIMATE_PRESET_ECO = 5,
  CLIMATE_PRESET_SLEEP = 6, CLIMATE_PRESET_ACTIVITY = 7,
};

enum ClimateFeature : uint32_t {
  CLIMATE_SUPPORTS_CURRENT_TEMPERATURE = 1 << 0,
  CLIMATE_SUPPORTS_TWO_POINT_TARGET_TEMPERATURE = 1 << 1,
  CLIMATE_REQUIRES_TWO_POINT_TARGET_TEMPERATURE = 1 << 2,
  CLIMATE_SUPPORTS_CURRENT_HUMIDITY = 1 << 3,
  CLIMATE_SUPPORTS_TARGET_HUMIDITY = 1 << 4,
  CLIMATE_SUPPORTS_ACTION = 1 << 5,
};

const LogString *climate_mode_to_string(ClimateMode mode);
const LogString *climate_fan_mode_to_string(ClimateFanMode mode);
const LogString *climate_swing_mode_to_string(ClimateSwingMode mode);
const LogString *climate_preset_to_string(ClimatePreset preset);

}  // namespace esphome::climate
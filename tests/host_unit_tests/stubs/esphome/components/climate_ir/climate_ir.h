#pragma once
#include "esphome/components/climate/climate.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/log.h"

namespace esphome::climate_ir {

class ClimateIR : public Component,
                  public climate::Climate,
                  public remote_base::RemoteReceiverListener,
                  public remote_base::RemoteTransmittable {
 public:
  ClimateIR(float minimum_temperature, float maximum_temperature, float temperature_step = 1.0f,
            bool supports_dry = false, bool supports_fan_only = false,
            climate::ClimateFanModeMask fan_modes = climate::ClimateFanModeMask(),
            climate::ClimateSwingModeMask swing_modes = climate::ClimateSwingModeMask(),
            climate::ClimatePresetMask presets = climate::ClimatePresetMask()) {
    this->minimum_temperature_ = minimum_temperature;
    this->maximum_temperature_ = maximum_temperature;
    this->temperature_step_ = temperature_step;
    this->supports_dry_ = supports_dry;
    this->supports_fan_only_ = supports_fan_only;
    this->fan_modes_ = fan_modes;
    this->swing_modes_ = swing_modes;
    this->presets_ = presets;
  }

  void setup() override {}  // host stub; test does not rely on restore-from-flash
  void dump_config() override {}
  climate::ClimateTraits traits() override { return climate::ClimateTraits(); }
  void set_supports_cool(bool v) { this->supports_cool_ = v; }
  void set_supports_heat(bool v) { this->supports_heat_ = v; }
  void set_sensor(sensor::Sensor *s) { this->sensor_ = s; }
  void set_humidity_sensor(sensor::Sensor *s) { this->humidity_sensor_ = s; }

  virtual void transmit_state() = 0;
  bool on_receive(remote_base::RemoteReceiveData data) override { return false; }

 protected:
  float minimum_temperature_{}, maximum_temperature_{}, temperature_step_{};
  bool supports_cool_{true};
  bool supports_heat_{true};
  bool supports_dry_{false};
  bool supports_fan_only_{false};
  climate::ClimateFanModeMask fan_modes_{};
  climate::ClimateSwingModeMask swing_modes_{};
  climate::ClimatePresetMask presets_{};
  sensor::Sensor *sensor_{nullptr};
  sensor::Sensor *humidity_sensor_{nullptr};
};

}  // namespace esphome::climate_ir
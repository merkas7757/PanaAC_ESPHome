// Host unit tests for the panaac IR protocol logic. Compiles the UNMODIFIED component
// (panaac.cpp + extra.cpp) against minimal ESPHome stubs and exercises encode/decode via
// public + exposed-protected methods. No ESP device, no network, no PlatformIO.
#include "panaac.h"
#include "test_framework.h"
#include <vector>
#include <cstdint>
#include <cmath>
#include <string>

using namespace esphome::panaac;
using esphome::remote_base::RemoteReceiveData;
using esphome::remote_base::RemoteTransmitterBase;
using esphome::remote_base::RawTimings;
using esphome::remote_base::TOLERANCE_MODE_PERCENTAGE;
using esphome::climate::ClimateMode;
using esphome::climate::ClimateFanMode;
using esphome::climate::ClimateSwingMode;

// Test-only subclass that re-exposes protected methods as public (no component change).
struct TestClimate : public PanaACClimate {
  using PanaACClimate::transmit_state;
  using PanaACClimate::on_receive;
  using PanaACClimate::decode_data;
  using PanaACClimate::decode_state;
};

struct Harness {
  TestClimate c;
  RemoteTransmitterBase tx;
  PanaACFanLevel fl;
  PanaACSwingV sv;
  PanaACSwingH sh;
  explicit Harness(bool quiet, bool fan5, bool swing_h, bool ir_ctrl) {
    c.set_supports_quiet(quiet);
    c.set_fan_5level(fan5);
    c.set_swing_horizontal(swing_h);
    c.set_ir_control(ir_ctrl);
    c.set_supports_fan_only(true);
    c.set_supports_heat(true);
    c.set_supports_cool(true);
    fl.set_parent_climate(&c);
    c.set_fanlevel(&fl);
    sv.set_parent_climate(&c);
    c.set_swingv(&sv);
    if (swing_h) {
      sh.set_parent_climate(&c);
      c.set_swingh(&sh);
    }
    c.set_transmitter(&tx);
  }
};

static ClimateState snapshot(const TestClimate &h) { return h.ac_state; }

// Independent minimal raw→bytes parser for the second (19-byte) frame, used to assert the
// encoder produced spec-correct bytes. Marks positive, spaces negative; bit=1 when space==-1200.
static std::vector<uint8_t> parse_second_frame(const RawTimings &raw) {
  // find the 2nd header: the first FRAME_END (-10000) is the end of frame 1; the next +3650
  // mark that follows is the 2nd header mark.
  size_t i = 0;
  // skip past first frame's trailing FRAME_END space
  bool saw_frame_end = false;
  for (; i + 1 < raw.size(); i++) {
    if (raw[i] > 0 && raw[i + 1] == -PANAAC_FRAME_END) { saw_frame_end = true; break; }
  }
  if (!saw_frame_end) return {};
  i += 2;  // past the frame-end (mark+space)
  // expect 2nd header: mark +3650, space -1600
  if (i + 1 >= raw.size() || raw[i] != PANAAC_HEADER_MARK || raw[i + 1] != -PANAAC_HEADER_SPACE)
    return {};
  i += 2;  // past 2nd header
  std::vector<uint8_t> bytes;
  for (int b = 0; b < 19; b++) {
    uint8_t byte = 0;
    for (int bit = 0; bit < 8; bit++) {
      // raw[i] = bit mark (+550), raw[i+1] = space (-1200 one / -350 zero)
      if (raw[i + 1] == -PANAAC_ONE_SPACE) byte |= (1 << bit);
      i += 2;
    }
    bytes.push_back(byte);
  }
  return bytes;
}

static bool decode_via_on_receive(TestClimate &c, const RawTimings &raw) {
  RemoteReceiveData data(raw, 55, TOLERANCE_MODE_PERCENTAGE);
  return c.on_receive(data);
}

// ---------------------------------------------------------------------------
// TEST 1 — encode produces a full 27-byte (440-entry) raw frame
// ---------------------------------------------------------------------------
static void test_encode_raw_size() {
  g_current_case = "encode_raw_size";
  Harness h(false, false, false, false);
  h.c.ac_state.mode = ClimateMode::CLIMATE_MODE_COOL;
  h.c.ac_state.temp = 24.0f;
  h.c.ac_state.fan_mode = ClimateFanMode::CLIMATE_FAN_AUTO;
  h.c.ac_state.fan_level = PANAAC_FAN_AUTO;
  h.c.ac_state.swing_mode = ClimateSwingMode::CLIMATE_SWING_VERTICAL;
  h.c.ac_state.swing_v_pos = PANAAC_SWINGV_AUTO;
  h.c.ac_state.swing_h_pos = PANAAC_SWINGH_NONE;
  h.c.transmit_data();
  CHECK_EQ(h.tx.temp_.get_data().size(), 440);
}

// ---------------------------------------------------------------------------
// TEST 2 — encode byte correctness against the protocol spec (independent parser)
//   COOL, 24C, fan AUTO, swingV AUTO, swingH NONE
// ---------------------------------------------------------------------------
static void test_encode_spec_bytes() {
  g_current_case = "encode_spec_bytes";
  Harness h(false, false, false, false);
  h.c.ac_state.mode = ClimateMode::CLIMATE_MODE_COOL;
  h.c.ac_state.temp = 24.0f;
  h.c.ac_state.fan_mode = ClimateFanMode::CLIMATE_FAN_AUTO;
  h.c.ac_state.fan_level = PANAAC_FAN_AUTO;
  h.c.ac_state.swing_mode = ClimateSwingMode::CLIMATE_SWING_VERTICAL;
  h.c.ac_state.swing_v_pos = PANAAC_SWINGV_AUTO;
  h.c.ac_state.swing_h_pos = PANAAC_SWINGH_NONE;
  h.c.transmit_data();
  auto bytes = parse_second_frame(h.tx.temp_.get_data());
  CHECK_EQ(bytes.size(), 19u);
  // protocol header (bytes 0..4)
  CHECK_EQ(bytes[0], 0x02);
  CHECK_EQ(bytes[1], 0x20);
  CHECK_EQ(bytes[2], 0xE0);
  CHECK_EQ(bytes[3], 0x04);
  CHECK_EQ(bytes[4], 0x00);
  // byte5: power ON (0x01) | mode COOL (0x30) = 0x31
  CHECK_EQ(bytes[5], uint8_t(PANAAC_POWER_ON | PANAAC_MODE_COOL));
  // byte6: 0x20 | ((24-16)<<1) = 0x20 | 0x10 = 0x30
  CHECK_EQ(bytes[6], uint8_t(0x20 | (((24 - PANAAC_TEMP_MIN) << 1) & 0x1E)));
  // byte8: fan AUTO (0xA0) | swingV AUTO (0x0F) = 0xAF
  CHECK_EQ(bytes[8], uint8_t((int) PANAAC_FAN_AUTO | (int) PANAAC_SWINGV_AUTO));
  // checksum: sum of bytes[0..17] == bytes[18]
  uint8_t sum = 0;
  for (int i = 0; i < 18; i++) sum += bytes[i];
  CHECK_EQ(bytes[18], sum);
}

// ---------------------------------------------------------------------------
// TEST 3 — round-trip: encode -> on_receive -> decode reproduces the encoded state
// ---------------------------------------------------------------------------
static void test_round_trip() {
  g_current_case = "round_trip";
  Harness h(true, true, true, false);  // quiet + 5-level + horizontal
  // a representative state incl. half-degree temp and a non-AUTO swing position
  h.c.ac_state.mode = ClimateMode::CLIMATE_MODE_HEAT;
  h.c.ac_state.temp = 26.5f;
  h.c.ac_state.fan_mode = ClimateFanMode::CLIMATE_FAN_MEDIUM;
  h.c.ac_state.fan_level = PANAAC_FAN_LEVEL_4;  // MEDIUM can be L3 or L4
  h.c.ac_state.swing_mode = ClimateSwingMode::CLIMATE_SWING_OFF;
  h.c.ac_state.swing_v_pos = PANAAC_SWINGV_HIGH;
  h.c.ac_state.swing_h_pos = PANAAC_SWINGH_LEFT;
  h.c.ac_state.last_swing_v_pos = PANAAC_SWINGV_HIGH;
  h.c.ac_state.last_swing_h_pos = PANAAC_SWINGH_LEFT;
  h.c.transmit_data();
  ClimateState before = snapshot(h.c);  // what was actually encoded
  CHECK_TRUE(decode_via_on_receive(h.c, h.tx.temp_.get_data()));
  CHECK_EQ(h.c.ac_state.mode, before.mode);
  CHECK_EQ(h.c.ac_state.fan_level, before.fan_level);
  CHECK_EQ(h.c.ac_state.swing_v_pos, before.swing_v_pos);
  CHECK_EQ(h.c.ac_state.swing_h_pos, before.swing_h_pos);
  // temp encodes to 0.5 resolution; compare with half-degree tolerance
  CHECK_TRUE(fabs(h.c.ac_state.temp - before.temp) < 0.01f);
}

// ---------------------------------------------------------------------------
// TEST 4 — select-driven fan level: "Level 2" is preserved through update_state
//   (the select -> update_state -> transmit_data path keeps L2, unlike the
//   climate transmit_state path tested below)
// ---------------------------------------------------------------------------
static void test_select_keeps_level2() {
  g_current_case = "select_keeps_level2";
  Harness h(false, true, false, false);  // 5-level
  h.fl.control(STR_FAN_L2);  // sets ac_state.fan_level=L2, fan_mode=LOW, calls update_state
  CHECK_EQ(h.c.ac_state.fan_level, PANAAC_FAN_LEVEL_2);
  CHECK_EQ(h.c.ac_state.fan_mode, ClimateFanMode::CLIMATE_FAN_LOW);
  // round-trip the just-transmitted frame; decode must give back L2
  CHECK_TRUE(decode_via_on_receive(h.c, h.tx.temp_.get_data()));
  CHECK_EQ(h.c.ac_state.fan_level, PANAAC_FAN_LEVEL_2);
}

// ---------------------------------------------------------------------------
// TEST 5 — characterization of the L2-loss bug via transmit_state (climate path)
//   transmit_state() collapses CLIMATE_FAN_LOW -> PANAAC_FAN_LEVEL_1, dropping a
//   previously-selected L2. This ASSERTS THE CURRENT (buggy) behavior so the suite
//   stays green; flip the expectation to LEVEL_2 once the bug is fixed.
// ---------------------------------------------------------------------------
static void test_transmit_state_loses_level2() {
  g_current_case = "transmit_state_loses_level2";
  Harness h(false, true, false, false);
  // simulate: user picked L2 via select, then drives the climate entity (temp change)
  h.c.ac_state.fan_level = PANAAC_FAN_LEVEL_2;
  h.c.ac_state.fan_mode = ClimateFanMode::CLIMATE_FAN_LOW;
  h.c.fan_mode = ClimateFanMode::CLIMATE_FAN_LOW;
  h.c.mode = ClimateMode::CLIMATE_MODE_COOL;
  h.c.target_temperature = 24.0f;
  h.c.swing_mode = ClimateSwingMode::CLIMATE_SWING_VERTICAL;
  h.c.transmit_state();
  // BUG: transmit_state maps LOW -> L1, discarding the L2 the user selected.
  CHECK_EQ(h.c.ac_state.fan_level, PANAAC_FAN_LEVEL_1);
}

// ---------------------------------------------------------------------------
// TEST 6 — decode rejects malformed frames
// ---------------------------------------------------------------------------
static void test_decode_rejects_bad_frames() {
  g_current_case = "decode_rejects_bad";
  Harness h(false, false, false, false);
  ClimateState out{};
  // wrong length (18 bytes)
  std::vector<uint8_t> short18(18, 0x00);
  CHECK_FALSE(h.c.decode_state(short18, out));
  // right length, wrong protocol header
  std::vector<uint8_t> badhdr(19, 0x00);
  badhdr[0] = 0xFF;
  CHECK_FALSE(h.c.decode_state(badhdr, out));
  // right header, wrong checksum
  std::vector<uint8_t> badck(19, 0x00);
  badck[0] = 0x02; badck[1] = 0x20; badck[2] = 0xE0; badck[3] = 0x04; badck[4] = 0x00;
  badck[5] = 0x31; badck[6] = 0x30; badck[8] = 0xAF;
  // checksum (sum 0..17) intentionally not placed in [18]
  uint8_t sum = 0; for (int i = 0; i < 18; i++) sum += badck[i];
  badck[18] = uint8_t(sum + 1);  // off-by-one -> invalid
  CHECK_FALSE(h.c.decode_state(badck, out));
  // valid checksum -> accepted
  badck[18] = sum;
  CHECK_TRUE(h.c.decode_state(badck, out));
}

// ---------------------------------------------------------------------------
// runner
// ---------------------------------------------------------------------------
int main() {
  test_encode_raw_size();
  test_encode_spec_bytes();
  test_round_trip();
  test_select_keeps_level2();
  test_transmit_state_loses_level2();
  test_decode_rejects_bad_frames();
  return report_results();
}
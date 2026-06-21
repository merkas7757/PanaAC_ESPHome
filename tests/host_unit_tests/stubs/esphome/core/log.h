#pragma once
#include <cstdint>
// Minimal log stub: all ESP_LOG* are no-ops, and ESPHOME_LOG_LEVEL is set to NONE so the
// verbose hex-dump blocks in panaac.cpp (guarded by #if ESPHOME_LOG_LEVEL >= VERBOSE) are
// compiled out, avoiding std::string heap churn in the host test.
namespace esphome {
struct LogString;  // incomplete; only used as `const LogString *` in climate_mode.h decls
}

#define ESP_LOGE(tag, ...) do {} while (0)
#define ESP_LOGW(tag, ...) do {} while (0)
#define ESP_LOGI(tag, ...) do {} while (0)
#define ESP_LOGD(tag, ...) do {} while (0)
#define ESP_LOGC(tag, ...) do {} while (0)
#define ESP_LOGCONFIG(tag, ...) do {} while (0)
#define ESP_LOGV(tag, ...) do {} while (0)
#define ESP_LOGVV(tag, ...) do {} while (0)

#define LOG_CLIMATE(prefix, name, obj) do {} while (0)
#define LOG_SELECT(prefix, name, obj) do {} while (0)

#define YESNO(x) ((x) ? "YES" : "NO")

#define ESPHOME_LOG_LEVEL_NONE 0
#define ESPHOME_LOG_LEVEL_ERROR 1
#define ESPHOME_LOG_LEVEL_WARN 2
#define ESPHOME_LOG_LEVEL_INFO 3
#define ESPHOME_LOG_LEVEL_DEBUG 4
#define ESPHOME_LOG_LEVEL_VERBOSE 5
#define ESPHOME_LOG_LEVEL_VERY_VERBOSE 6

// Compile out all verbose logging blocks in the component under test.
#define ESPHOME_LOG_LEVEL ESPHOME_LOG_LEVEL_NONE
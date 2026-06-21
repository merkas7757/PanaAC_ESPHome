#pragma once
#include <cstdint>
#include <vector>
#include "esphome/core/log.h"

namespace esphome::remote_base {

using RawTimings = std::vector<int32_t>;

enum ToleranceMode {
  TOLERANCE_MODE_TIME,
  TOLERANCE_MODE_PERCENTAGE,
};

class RemoteTransmitData {
 public:
  void mark(uint32_t length) { this->data_.push_back(static_cast<int32_t>(length)); }
  void space(uint32_t length) { this->data_.push_back(-static_cast<int32_t>(length)); }
  void set_carrier_frequency(uint32_t f) { this->carrier_frequency_ = f; }
  const RawTimings &get_data() const { return this->data_; }
  RawTimings data_{};
  uint32_t carrier_frequency_{0};
};

// Faithful reimplementation of esphome::remote_base::RemoteReceiveData so the real
// panaac decode logic runs unchanged on the host. Marks are positive, spaces negative.
class RemoteReceiveData {
 public:
  explicit RemoteReceiveData(const RawTimings &data, uint32_t tolerance, ToleranceMode mode)
      : data_(data), index_(0), tolerance_(tolerance), tolerance_mode_(mode) {}

  const RawTimings &get_raw_data() const { return this->data_; }
  uint32_t get_index() const { return this->index_; }
  int32_t size() const { return static_cast<int32_t>(this->data_.size()); }
  int32_t operator[](uint32_t i) const { return this->data_[i]; }
  bool is_valid(uint32_t offset = 0) const { return this->index_ + offset < this->data_.size(); }
  int32_t peek(uint32_t offset = 0) const { return this->data_[this->index_ + offset]; }
  void advance(uint32_t amount = 1) { this->index_ += amount; }
  void reset() { this->index_ = 0; }

  int32_t lower_bound_(uint32_t length) const {
    if (this->tolerance_mode_ == TOLERANCE_MODE_TIME) return int32_t(length - this->tolerance_);
    if (this->tolerance_mode_ == TOLERANCE_MODE_PERCENTAGE) return int32_t(100 - this->tolerance_) * length / 100U;
    return 0;
  }
  int32_t upper_bound_(uint32_t length) const {
    if (this->tolerance_mode_ == TOLERANCE_MODE_TIME) return int32_t(length + this->tolerance_);
    if (this->tolerance_mode_ == TOLERANCE_MODE_PERCENTAGE) return int32_t(100 + this->tolerance_) * length / 100U;
    return 0;
  }

  bool peek_mark(uint32_t length, uint32_t offset = 0) const {
    if (!this->is_valid(offset)) return false;
    int32_t value = this->peek(offset);
    int32_t lo = this->lower_bound_(length), hi = this->upper_bound_(length);
    return value >= 0 && lo <= value && value <= hi;
  }
  bool peek_space(uint32_t length, uint32_t offset = 0) const {
    if (!this->is_valid(offset)) return false;
    int32_t value = this->peek(offset);
    int32_t lo = this->lower_bound_(length), hi = this->upper_bound_(length);
    return value <= 0 && lo <= -value && -value <= hi;
  }
  bool peek_item(uint32_t mark, uint32_t space, uint32_t offset = 0) const {
    return this->peek_space(space, offset + 1) && this->peek_mark(mark, offset);
  }
  bool expect_mark(uint32_t length) {
    if (!this->peek_mark(length)) return false;
    this->advance(); return true;
  }
  bool expect_space(uint32_t length) {
    if (!this->peek_space(length)) return false;
    this->advance(); return true;
  }
  bool expect_item(uint32_t mark, uint32_t space) {
    if (!this->peek_item(mark, space)) return false;
    this->advance(2); return true;
  }

 protected:
  const RawTimings &data_;
  uint32_t index_;
  uint32_t tolerance_;
  ToleranceMode tolerance_mode_;
};

class RemoteTransmitterBase;

class RemoteTransmittable {
 public:
  RemoteTransmittable() = default;
  explicit RemoteTransmittable(RemoteTransmitterBase *tx) : transmitter_(tx) {}
  void set_transmitter(RemoteTransmitterBase *tx) { this->transmitter_ = tx; }
 protected:
  RemoteTransmitterBase *transmitter_{nullptr};
};

class RemoteReceiverListener {
 public:
  virtual ~RemoteReceiverListener() = default;
  virtual bool on_receive(RemoteReceiveData data) = 0;
};

class RemoteTransmitterBase {
 public:
  virtual ~RemoteTransmitterBase() = default;
  RemoteTransmitData temp_;  // written to by transmit().get_data()
  class Call {
   public:
    explicit Call(RemoteTransmitterBase *p) : parent_(p) {}
    RemoteTransmitData *get_data() { return &this->parent_->temp_; }
    void perform() {}
   protected:
    RemoteTransmitterBase *parent_;
  };
  virtual Call transmit() { return Call(this); }
};

}  // namespace esphome::remote_base
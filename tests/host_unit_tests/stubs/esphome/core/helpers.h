#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <initializer_list>

namespace esphome {

// Minimal FixedVector matching the API surface panaac uses:
// default ctor, init(n), push_back, size(), operator[], begin/end.
template<typename T> class FixedVector {
 public:
  FixedVector() = default;
  ~FixedVector() { delete[] data_; }
  void init(size_t n) {
    delete[] data_;
    data_ = n ? new T[n]() : nullptr;
    size_ = 0;
    capacity_ = n;
  }
  void push_back(const T &v) {
    if (size_ < capacity_) {
      data_[size_] = v;
      ++size_;
    }
  }
  size_t size() const { return size_; }
  const T &operator[](size_t i) const { return data_[i]; }
  T &operator[](size_t i) { return data_[i]; }
  const T *begin() const { return data_; }
  const T *end() const { return data_ + size_; }
 private:
  T *data_{nullptr};
  size_t size_{0};
  size_t capacity_{0};
};

}  // namespace esphome
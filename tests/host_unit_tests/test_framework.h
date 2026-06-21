#pragma once
// Tiny test framework (no external deps). CHECK/REQUIRE print on failure; REQUIRE aborts.
#include <cstdio>
#include <cstdlib>
#include <string>

static int g_tests_run = 0, g_tests_failed = 0;
static const char *g_current_case = "";

#define CHECK_EQ(a, b) check_eq_impl((a), (b), #a, #b, __FILE__, __LINE__)
#define CHECK_TRUE(c) check_true_impl((c), #c, __FILE__, __LINE__)
#define CHECK_FALSE(c) check_true_impl(!(c), "!(" #c ")", __FILE__, __LINE__)

template<typename A, typename B>
static void check_eq_impl(const A &a, const B &b, const char *ea, const char *eb,
                          const char *file, int line) {
  ++g_tests_run;
  if (!(a == b)) {
    ++g_tests_failed;
    std::printf("  FAIL [%s] %s:%d: %s != %s\n", g_current_case, file, line, ea, eb);
  }
}
static void check_true_impl(bool c, const char *ec, const char *file, int line) {
  ++g_tests_run;
  if (!c) {
    ++g_tests_failed;
    std::printf("  FAIL [%s] %s:%d: %s is false\n", g_current_case, file, line, ec);
  }
}

static int report_results() {
  std::printf("----\nrun=%d failed=%d\n", g_tests_run, g_tests_failed);
  return g_tests_failed ? 1 : 0;
}
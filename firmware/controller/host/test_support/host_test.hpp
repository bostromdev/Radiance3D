#pragma once

// A tiny host-only Unity compatibility layer.  ESP-IDF component tests use
// ESP-IDF's Unity component; portable CTest builds avoid a network dependency.

#include <cmath>
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace radiance3d_host_test {

class AssertionFailure final : public std::runtime_error {
 public:
  explicit AssertionFailure(const std::string& message) : std::runtime_error(message) {}
};

inline int& failures() {
  static int value = 0;
  return value;
}

inline int begin() { return 0; }

inline int end() { return failures(); }

[[noreturn]] inline void fail(const char* expression, const char* file, int line,
                              const std::string& detail = "") {
  std::ostringstream output;
  output << file << ':' << line << ": assertion failed: " << expression;
  if (!detail.empty()) {
    output << " (" << detail << ')';
  }
  throw AssertionFailure(output.str());
}

template <typename Expected, typename Actual>
inline void equal(const Expected& expected, const Actual& actual, const char* expression,
                  const char* file, int line) {
  if (!(expected == actual)) {
    fail(expression, file, line);
  }
}

template <typename Expected, typename Actual>
inline void not_equal(const Expected& expected, const Actual& actual,
                      const char* expression, const char* file, int line) {
  if (expected == actual) {
    fail(expression, file, line);
  }
}

template <typename Expected, typename Actual>
inline void float_within(const Expected& delta, const Actual& expected, const Actual& actual,
                         const char* expression, const char* file, int line) {
  if (std::fabs(static_cast<double>(expected) - static_cast<double>(actual)) >
      static_cast<double>(delta)) {
    fail(expression, file, line);
  }
}

template <typename Function>
inline void run(const char* name, Function&& function) {
  try {
    function();
  } catch (const std::exception& error) {
    ++failures();
    std::cerr << "FAILED " << name << ": " << error.what() << '\n';
  } catch (...) {
    ++failures();
    std::cerr << "FAILED " << name << ": unknown exception\n";
  }
}

}  // namespace radiance3d_host_test

#define UNITY_BEGIN() ::radiance3d_host_test::begin()
#define UNITY_END() ::radiance3d_host_test::end()
#define RUN_TEST(function)                                                        \
  ::radiance3d_host_test::run(#function, [] {                                    \
    setUp();                                                                      \
    try {                                                                         \
      function();                                                                 \
    } catch (...) {                                                               \
      tearDown();                                                                 \
      throw;                                                                      \
    }                                                                             \
    tearDown();                                                                   \
  })

#define TEST_ASSERT_TRUE(condition)                                               \
  do {                                                                            \
    if (!(condition)) {                                                           \
      ::radiance3d_host_test::fail(#condition, __FILE__, __LINE__);              \
    }                                                                             \
  } while (false)
#define TEST_ASSERT_FALSE(condition) TEST_ASSERT_TRUE(!(condition))
#define TEST_ASSERT_EQUAL(expected, actual)                                      \
  ::radiance3d_host_test::equal((expected), (actual), #expected " == " #actual, \
                                __FILE__, __LINE__)
#define TEST_ASSERT_NOT_EQUAL(expected, actual)                                  \
  ::radiance3d_host_test::not_equal((expected), (actual), #expected " != " #actual, \
                                    __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_INT(expected, actual) TEST_ASSERT_EQUAL((expected), (actual))
#define TEST_ASSERT_EQUAL_INT64(expected, actual) TEST_ASSERT_EQUAL((expected), (actual))
#define TEST_ASSERT_EQUAL_UINT8(expected, actual) TEST_ASSERT_EQUAL((expected), (actual))
#define TEST_ASSERT_EQUAL_UINT16(expected, actual) TEST_ASSERT_EQUAL((expected), (actual))
#define TEST_ASSERT_EQUAL_UINT32(expected, actual) TEST_ASSERT_EQUAL((expected), (actual))
#define TEST_ASSERT_NOT_EQUAL_INT64(expected, actual) TEST_ASSERT_NOT_EQUAL((expected), (actual))
#define TEST_ASSERT_EQUAL_STRING(expected, actual)                               \
  TEST_ASSERT_TRUE(std::strcmp((expected), (actual)) == 0)
#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)                        \
  ::radiance3d_host_test::float_within((delta), (expected), (actual),            \
                                       #expected " ~= " #actual, __FILE__, __LINE__)

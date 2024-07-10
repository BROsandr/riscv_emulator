#define CATCH_CONFIG_MAIN

#include "memory.hpp"

#include "catch2/catch_test_macros.hpp"

TEST_CASE("Directed test vectors", "[direct]") {
  SECTION("all sequential") {
    Rf rf{};
    for (unsigned int i{0}; i < 32; ++i) {
      rf.write(i, i);
    }
    for (unsigned int i{0}; i < 32; ++i) {
      REQUIRE(rf.read(i) == i);
    }
  }
}

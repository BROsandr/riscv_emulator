#include "exception.hpp"
#include "instr_mem.hpp"

#define CATCH_CONFIG_MAIN

#include "catch2/catch_test_macros.hpp"

TEST_CASE("instr mem", "[INSTR_MEM]") {
  std::vector<Uxlen> instr_container(2);

  Instr_mem instr_mem{instr_container.begin(), instr_container.end()};

  SECTION("uninitialized_read") {
    REQUIRE(instr_mem.read(0) == 0);
    REQUIRE(instr_mem.read(1) == 0);
  }

  SECTION("out_of_range_read") {
    REQUIRE_THROWS_AS(instr_mem.read(2), Errors::Illegal_addr);
  }

  SECTION("simple") {
    instr_container[0] = 1;
    instr_container[1] = 2;

    REQUIRE(instr_mem.read(0) == 1);
    REQUIRE(instr_mem.read(1) == 2);
  }

  SECTION("exception") {
    REQUIRE_THROWS_AS(instr_mem.write(0, 3), Errors::Read_only);
  }
}

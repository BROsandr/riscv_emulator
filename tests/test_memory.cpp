#include "exception.hpp"
#include "instr_mem.hpp"

#define CATCH_CONFIG_MAIN

#include "catch2/catch_test_macros.hpp"

TEST_CASE("instr mem", "[INSTR_MEM]") {
  std::vector<Uxlen> instr_container{};

  Instr_mem_wrap instr_mem{instr_container};

  SECTION("uninitialized_read") {
    REQUIRE_THROWS_AS(instr_mem.read(0), Errors::Illegal_addr);
  }

  SECTION("out_of_range_read") {
    instr_container.push_back({});
    REQUIRE_NOTHROW(instr_mem.read(0));
    REQUIRE_THROWS_AS(instr_mem.read(4), Errors::Illegal_addr);
  }

  SECTION("simple 2 reads") {
    const std::vector<Uxlen> data{0x01020304, 0x05060708};
    instr_container.push_back(data.at(0));
    instr_container.push_back(data.at(1));

    REQUIRE(instr_mem.read(0) == data.at(0));
    REQUIRE(instr_mem.read(4) == data.at(1));

    REQUIRE(instr_container == data);
  }

  SECTION("exception") {
    REQUIRE_THROWS_AS(instr_mem.write(0, 0), Errors::Read_only);
  }
}

#include "exception.hpp"
#include "instr_mem.hpp"
#include "data_mem.hpp"

#define CATCH_CONFIG_MAIN

#include "catch2/catch_test_macros.hpp"

TEST_CASE("instr mem", "[INSTR_MEM]") {
  std::vector<Uxlen> instr_container{};

  Instr_mem_view instr_mem{instr_container};

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

using Byte = std::byte;
using Cont = std::vector<Byte>;

TEST_CASE("data_mem", "[DATA_MEM]") {
  Cont container{};

  Data_mem_view data_mem{container};

  SECTION("uninitialized_write") {
    REQUIRE_THROWS_AS(data_mem.write(0, 0), Errors::Illegal_addr);
  }

  SECTION("uninitialized_read") {
    REQUIRE_THROWS_AS(data_mem.read(0, 0x1), Errors::Illegal_addr);
  }

  SECTION("out_of_range_read") {
    container.push_back({});
    REQUIRE_NOTHROW(data_mem.read(0, 0x1));
    REQUIRE_THROWS_AS(data_mem.read(4), Errors::Illegal_addr);
  }

  SECTION("simple 2 instr") {
    const Cont data{
        Byte{0x1},
        Byte{0x2},
        Byte{0x3},
        Byte{0x4},

        Byte{0x5},
        Byte{0x6},
        Byte{0x7},
        Byte{0x8},
    };
    container.push_back(data.at(0));
    container.push_back(data.at(1));
    container.push_back(data.at(2));
    container.push_back(data.at(3));

    container.push_back(data.at(4));
    container.push_back(data.at(5));
    container.push_back(data.at(6));
    container.push_back(data.at(7));

    SECTION("write") {
      SECTION("be=0b0001") {
        Cont new_data{data};
        new_data[0] = Byte{0xf};
        const unsigned int byte_en{0b0001};
        data_mem.write(0, 0xf, byte_en);
        REQUIRE(container == new_data);
      }
      SECTION("be=0b0010") {
        Cont new_data{data};
        new_data[1] = Byte{0xf};
        const unsigned int byte_en{0b0010};
        data_mem.write(0, 0xf << CHAR_BIT, byte_en);
        REQUIRE(container == new_data);
      }
    }
  }
}

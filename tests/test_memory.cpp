#include "exception.hpp"
#include "instr_mem.hpp"
#include "data_mem.hpp"
#include "riscv_algos.hpp"

#include <climits>

#include <map>

#define CATCH_CONFIG_MAIN

#include "catch2/catch_test_macros.hpp"

TEST_CASE("instr mem", "[INSTR_MEM]") {
  std::vector<Uxlen> instr_container{};

  Instr_mem_view instr_mem{instr_container};

  SECTION("uninitialized_read") {
    REQUIRE_THROWS_AS(instr_mem.read(0), Errors::Illegal_addr);
    REQUIRE(instr_container.empty());
  }

  SECTION("out_of_range_read") {
    instr_container.push_back({});
    REQUIRE_NOTHROW(instr_mem.read(0));
    REQUIRE_THROWS_AS(instr_mem.read(4), Errors::Illegal_addr);
    REQUIRE(instr_container.size() == 1);
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
    REQUIRE(instr_container.empty());
  }
}

using Byte = std::byte;

TEST_CASE("data_mem", "[DATA_MEM]") {
  using Cont = std::map<std::size_t, Byte>;
  Cont container{};

  Data_mem_view data_mem{container};

  SECTION("uninitialized_write") {
    REQUIRE_NOTHROW(data_mem.write(0, 0));
    REQUIRE(container.size() == 4);
  }

  SECTION("uninitialized_read") {
    REQUIRE_THROWS_AS(data_mem.read(0, 0x1), Errors::Illegal_addr);
    REQUIRE(container.empty());
  }

  SECTION("out_of_range_read") {
    container[0] = {};
    REQUIRE_NOTHROW(data_mem.read(0, 0x1));
    REQUIRE_THROWS_AS(data_mem.read(4), Errors::Illegal_addr);
    REQUIRE(container.size() == 1);
  }

  SECTION("misalignment") {
    data_mem.m_assured_aligment = true;
    container[0] = {};
    container[1] = {};
    container[2] = {};
    container[3] = {};
    REQUIRE_THROWS_AS(data_mem.read(0, 0b10000), Errors::Misalignment);
    REQUIRE(container.size() == 4);
  }

  SECTION("simple 2 instr") {
    Cont data{};
    data[0] = Byte{0x1},
    data[1] = Byte{0x2},
    data[2] = Byte{0x3},
    data[3] = Byte{0x4},

    data[4] = Byte{0x5},
    data[5] = Byte{0x6},
    data[6] = Byte{0x7},
    data[7] = Byte{0x8},

    container[0] = data.at(0);
    container[1] = data.at(1);
    container[2] = data.at(2);
    container[3] = data.at(3);

    container[4] = data.at(4);
    container[5] = data.at(5);
    container[6] = data.at(6);
    container[7] = data.at(7);

    SECTION("write") {
      for (unsigned int byte_en{1}; byte_en < (0xf + 1); ++byte_en) {
        SECTION("be=" + std::to_string(byte_en)) {
          Cont new_data{data};
          Uxlen write_word{};
          std::vector<Byte> test_data{Byte{0xf},Byte{0xf}, Byte{0xf}, Byte{0xf}};
          if (extract_bits(byte_en, 0)) {
            new_data[0] = test_data[0];
            write_word |= static_cast<Uxlen>(test_data[0]);
          }
          if (extract_bits(byte_en, 1)) {
            new_data[1] = test_data[1];
            write_word |= static_cast<Uxlen>(test_data[1]) << CHAR_BIT;
          }
          if (extract_bits(byte_en, 2)) {
            new_data[2] = test_data[2];
            write_word |= static_cast<Uxlen>(test_data[2]) << 2 * CHAR_BIT;
          }
          if (extract_bits(byte_en, 3)) {
            new_data[3] = test_data[3];
            write_word |= static_cast<Uxlen>(test_data[3]) << 3 * CHAR_BIT;
          }
          data_mem.write(0, write_word, byte_en);
          REQUIRE(container == new_data);
        }
      }
    }

    SECTION("read") {
      for (unsigned int byte_en{1}; byte_en < (0xf + 1); ++byte_en) {
        SECTION("be=" + std::to_string(byte_en)) {
          Uxlen read_word{0};
          if (extract_bits(byte_en, 0)) {
            read_word |= static_cast<Uxlen>(data[0]);
          }
          if (extract_bits(byte_en, 1)) {
            read_word |= static_cast<Uxlen>(data[1]) << CHAR_BIT;
          }
          if (extract_bits(byte_en, 2)) {
            read_word |= static_cast<Uxlen>(data[2]) << 2 * CHAR_BIT;
          }
          if (extract_bits(byte_en, 3)) {
            read_word |= static_cast<Uxlen>(data[3]) << 3 * CHAR_BIT;
          }
          REQUIRE(data_mem.read(0, byte_en) == read_word);
          REQUIRE(container == data);
        }
      }
    }
    SECTION("write-read") {
      for (unsigned int byte_en{1}; byte_en < (0xf + 1); ++byte_en) {
        SECTION("be=" + std::to_string(byte_en)) {
          Cont new_data{data};
          Uxlen write_word{};
          std::vector<Byte> test_data{Byte{0xf},Byte{0xf}, Byte{0xf}, Byte{0xf}};
          if (extract_bits(byte_en, 0)) {
            new_data[0] = test_data[0];
            write_word |= static_cast<Uxlen>(test_data[0]);
          }
          if (extract_bits(byte_en, 1)) {
            new_data[1] = test_data[1];
            write_word |= static_cast<Uxlen>(test_data[1]) << CHAR_BIT;
          }
          if (extract_bits(byte_en, 2)) {
            new_data[2] = test_data[2];
            write_word |= static_cast<Uxlen>(test_data[2]) << 2 * CHAR_BIT;
          }
          if (extract_bits(byte_en, 3)) {
            new_data[3] = test_data[3];
            write_word |= static_cast<Uxlen>(test_data[3]) << 3 * CHAR_BIT;
          }
          data_mem.write(0, write_word, byte_en);
                    Uxlen read_word{0};

          if (extract_bits(byte_en, 0)) {
            read_word |= static_cast<Uxlen>(new_data[0]);
          }
          if (extract_bits(byte_en, 1)) {
            read_word |= static_cast<Uxlen>(new_data[1]) << CHAR_BIT;
          }
          if (extract_bits(byte_en, 2)) {
            read_word |= static_cast<Uxlen>(new_data[2]) << 2 * CHAR_BIT;
          }
          if (extract_bits(byte_en, 3)) {
            read_word |= static_cast<Uxlen>(new_data[3]) << 3 * CHAR_BIT;
          }
          REQUIRE(data_mem.read(0, byte_en) == read_word);
          REQUIRE(container == new_data);
        }
      }
    }
  }
}

TEST_CASE("ranged view", "[RANGED_VIEW]") {
  using Cont = std::map<std::size_t, Byte>;
  Cont container{};

  Data_mem_view data_mem{container};

  Ranged_mem_view ranged_cont{data_mem, 4, 11 - 4 + 1};

  SECTION("uninitialized_write") {
    REQUIRE_NOTHROW(ranged_cont.write(4, 0));
    REQUIRE(container.size() == 4);
  }

  SECTION("uninitialized_read") {
    REQUIRE_THROWS_AS(ranged_cont.read(4, 0x1), Errors::Illegal_addr);
    REQUIRE(container.empty());
  }

  SECTION("out_of_range") {
    container[0] = Byte{1};
    container[1] = Byte{2};
    container[2] = Byte{3};
    container[3] = Byte{4};
    container[4] = Byte{5};
    container[5] = Byte{6};
    container[6] = Byte{7};
    container[7] = Byte{8};
    REQUIRE_THROWS_AS(ranged_cont.read(0, 0xf), Errors::Illegal_addr);
    REQUIRE_THROWS_AS(ranged_cont.read(12, 0xf), Errors::Illegal_addr);
    REQUIRE(ranged_cont.read(4) == 0x04030201);
    REQUIRE(ranged_cont.read(8) == 0x08070605);
    REQUIRE(container.size() == 8);
  }

  SECTION("throw_when_read_before_write") {
    container[0] = Byte{1};
    container[1] = Byte{2};
    container[2] = Byte{3};
    container[3] = Byte{4};

    REQUIRE_THROWS_AS(ranged_cont.read(8, 0xf), Errors::Illegal_addr);
    ranged_cont.write(8, 0x08070605, 0xf);
    REQUIRE(ranged_cont.read(8, 0xf) == 0x08070605);
    REQUIRE(container[0] == Byte{1});
    REQUIRE(container[1] == Byte{2});
    REQUIRE(container[2] == Byte{3});
    REQUIRE(container[3] == Byte{4});
    REQUIRE(container[4] == Byte{5});
    REQUIRE(container[5] == Byte{6});
    REQUIRE(container[6] == Byte{7});
    REQUIRE(container[7] == Byte{8});
    REQUIRE(container.size() == 8);
  }

  SECTION("misalignment") {
    data_mem.m_assured_aligment = true;
    container[0] = {};
    container[1] = {};
    container[2] = {};
    container[3] = {};
    REQUIRE_THROWS_AS(ranged_cont.read(4, 0b10000), Errors::Misalignment);
    REQUIRE(container.size() == 4);
  }
}

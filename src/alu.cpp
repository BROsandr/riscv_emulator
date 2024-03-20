#include "alu.hpp"

#include <string>
#include <cassert>

namespace {
  using enum Alu::Op;

  constexpr std::pair<Uxlen, bool> calculate(Alu::Op op, Uxlen a, Uxlen b) {
      const Sxlen a_signed{static_cast<Sxlen>(a)};
      const Sxlen b_signed{static_cast<Sxlen>(b)};

      const auto get_shamt = [b]() constexpr {
        constexpr Uxlen shamt_mask{(sizeof(Uxlen) * 8) - 1};
        assert((b & shamt_mask) == b);
        return b;
      };

      Uxlen result{};
      bool  flag  {};

      switch (op) {
        case ADD : result = a + b                                      ; break;
        case SUB : result = a - b                                      ; break;
        case XOR : result = a ^ b                                      ; break;
        case OR  : result = a | b                                      ; break;
        case AND : result = a & b                                      ; break;
        case SRA : result = static_cast<Uxlen>(a_signed >> get_shamt()); break;
        case SRL : result = a >> get_shamt()                           ; break;
        case SLL : result = a << get_shamt()                           ; break;
        case SLTS: result = a_signed < b_signed                        ; break;
        case SLTU: result = a < b                                      ; break;
        case LTS : flag   = a_signed < b_signed                        ; break;
        case LTU : flag   = a < b                                      ; break;
        case GES : flag   = a_signed >= b_signed                       ; break;
        case GEU : flag   = a >= b                                     ; break;
        case EQ  : flag   = a == b                                     ; break;
        case NE  : flag   = a != b                                     ; break;
        default  : assert((void("Unknown_alu_op " + std::to_string(op)), 0));
      }

      return {result, flag};
  }
}

namespace Alu {
  Uxlen calc_result(Alu::Op op, Uxlen a, Uxlen b) {
    const auto [result, flag] = calculate(op, a, b);
    return result;
  }

  bool calc_flag(Alu::Op op, Uxlen a, Uxlen b) {
    const auto [result, flag] = calculate(op, a, b);
    return flag;
  }
}

#ifdef UNIT_TEST
#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

#include <limits>

TEST_CASE("Alu_ADD", "[ADD]") {
  SECTION("0 + 0") {
    auto [result, flag] = calculate(Alu::Op::ADD, 0, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == 0);
  }

  SECTION("-1 + 1") {
    auto [result, flag] = calculate(Alu::Op::ADD, -1u, 1);
    REQUIRE(result == 0);
    REQUIRE(flag   == 0);
  }
  SECTION("1 + 2") {
    auto [result, flag] = calculate(Alu::Op::ADD, 1, 2);
    REQUIRE(result == 3);
    REQUIRE(flag == 0);
  }

  SECTION("1 + (-2)") {
    auto [result, flag] = calculate(Alu::Op::ADD, 1, -2u);
    REQUIRE(result == -1u);
    REQUIRE(flag ==  0);
  }

  SECTION("-1 + 2") {
    auto [result, flag] = calculate(Alu::Op::ADD, -1u, 2);
    REQUIRE(result == 1);
    REQUIRE(flag == 0);
  }

  SECTION("-1 + (-2)") {
    auto [result, flag] = calculate(Alu::Op::ADD, -1u, -2u);
    REQUIRE(result == -3u);
    REQUIRE(flag ==  0);
  }

  SECTION("max_xlen + 1") {
    auto [result, flag] = calculate(Alu::Op::ADD, std::numeric_limits<Uxlen>::max(), 1);
    REQUIRE(result == 0);
    REQUIRE(flag == 0);
  }

  SECTION("min_xlen + (-1)") {
    auto [result, flag] = calculate(Alu::Op::ADD, 0, -1u);
    REQUIRE(result == std::numeric_limits<Uxlen>::max());
    REQUIRE(flag == 0);
  }
}

TEST_CASE("Alu_SUB", "[SUB]") {
  SECTION("0 - 0") {
    auto [result, flag] = calculate(Alu::Op::SUB, 0, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == 0);
  }

  SECTION("-1 - 1") {
    auto [result, flag] = calculate(Alu::Op::SUB, -1u, 1u);
    REQUIRE(result == -2u);
    REQUIRE(flag   ==  0 );
  }
  SECTION("1 - 2") {
    auto [result, flag] = calculate(Alu::Op::SUB, 1, 2);
    REQUIRE(result == -1u);
    REQUIRE(flag == 0);
  }

  SECTION("1 - (-2)") {
    auto [result, flag] = calculate(Alu::Op::SUB, 1, -2u);
    REQUIRE(result == 3u);
    REQUIRE(flag ==  0);
  }

  SECTION("-1 - 2") {
    auto [result, flag] = calculate(Alu::Op::SUB, -1u, 2);
    REQUIRE(result == -3u);
    REQUIRE(flag == 0);
  }

  SECTION("-1 - (-2)") {
    auto [result, flag] = calculate(Alu::Op::SUB, -1u, -2u);
    REQUIRE(result == 1u);
    REQUIRE(flag ==  0);
  }

  SECTION("min_len - 1") {
    auto [result, flag] = calculate(Alu::Op::SUB, 0, 1);
    REQUIRE(result == std::numeric_limits<Uxlen>::max());
    REQUIRE(flag == 0);
  }

  SECTION("max_xlen - (-1)") {
    auto [result, flag] = calculate(Alu::Op::SUB, std::numeric_limits<Uxlen>::max(), -1u);
    REQUIRE(result == 0);
    REQUIRE(flag == 0);
  }
}

TEST_CASE("Alu_XOR", "[XOR]") {
  SECTION("11 ^ 10") {
    auto [result, flag] = calculate(Alu::Op::XOR, 0b11, 0b10);
    REQUIRE(result == 0b01);
    REQUIRE(flag   == 0);
  }
}

TEST_CASE("Alu_OR", "[OR]") {
  SECTION("11 | 10") {
    auto [result, flag] = calculate(Alu::Op::OR, 0b11, 0b10);
    REQUIRE(result == 0b11);
    REQUIRE(flag   == 0);
  }
}

TEST_CASE("Alu_AND", "[AND]") {
  SECTION("11 & 10") {
    auto [result, flag] = calculate(Alu::Op::AND, 0b11, 0b10);
    REQUIRE(result == 0b10);
    REQUIRE(flag   == 0);
  }
}

TEST_CASE("Alu_SRA", "[SRA]") {
  SECTION("-2 >>> 1") {
    auto [result, flag] = calculate(Alu::Op::SRA, -2u, 1);
    REQUIRE(result == -1u);
    REQUIRE(flag   == 0);
  }

  SECTION("-2 >>> 0") {
    auto [result, flag] = calculate(Alu::Op::SRA, -2u, 0);
    REQUIRE(result == -2u);
    REQUIRE(flag   == 0);
  }

  SECTION("-2 >>> 0") {
    auto [result, flag] = calculate(Alu::Op::SRA, -2u, 0);
    REQUIRE(result == -2u);
    REQUIRE(flag   == 0);
  }

  SECTION("2 >>> 1") {
    auto [result, flag] = calculate(Alu::Op::SRA, 2, 1);
    REQUIRE(result == 1);
    REQUIRE(flag   == 0);
  }
}

TEST_CASE("Alu_SRL", "[SRL]") {
  SECTION("-2 >> 1") {
    auto [result, flag] = calculate(Alu::Op::SRL, -2u, 1);
    REQUIRE(result == 0x7FFFFFFF);
    REQUIRE(flag   == 0);
  }

  SECTION("4 >> 2") {
    auto [result, flag] = calculate(Alu::Op::SRL, 4, 2);
    REQUIRE(result == 1);
    REQUIRE(flag   == 0);
  }
}

TEST_CASE("Alu_SLL", "[SLL]") {
  SECTION("-2 << 1") {
    auto [result, flag] = calculate(Alu::Op::SLL, -2u, 1);
    REQUIRE(result == 0xFFFFFFFC);
    REQUIRE(flag   == 0);
  }

  SECTION("3 << 2") {
    auto [result, flag] = calculate(Alu::Op::SLL, 3, 2);
    REQUIRE(result == 0xc);
    REQUIRE(flag   == 0);
  }
}

TEST_CASE("Alu_SLTS", "[SLTS]") {
  SECTION("1 < 0") {
    auto [result, flag] = calculate(Alu::Op::SLTS, 1, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == 0);
  }
  SECTION("0 < 1") {
    auto [result, flag] = calculate(Alu::Op::SLTS, 0, 1);
    REQUIRE(result == 1);
    REQUIRE(flag   == 0);
  }
  SECTION("0 < 0") {
    auto [result, flag] = calculate(Alu::Op::SLTS, 0, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == 0);
  }
  SECTION("-2 < -1") {
    auto [result, flag] = calculate(Alu::Op::SLTS, -2u, -1u);
    REQUIRE(result == 1);
    REQUIRE(flag   == 0);
  }
  SECTION("-2 < 1") {
    auto [result, flag] = calculate(Alu::Op::SLTS, -2u, 1u);
    REQUIRE(result == 1);
    REQUIRE(flag   == 0);
  }
  SECTION("-1 < -2") {
    auto [result, flag] = calculate(Alu::Op::SLTS, -1u, -2u);
    REQUIRE(result == 0);
    REQUIRE(flag   == 0);
  }
}

TEST_CASE("Alu_SLTU", "[SLTU]") {
  SECTION("1 < 0") {
    auto [result, flag] = calculate(Alu::Op::SLTU, 1, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == 0);
  }
  SECTION("0 < 1") {
    auto [result, flag] = calculate(Alu::Op::SLTU, 0, 1);
    REQUIRE(result == 1);
    REQUIRE(flag   == 0);
  }
  SECTION("0 < 0") {
    auto [result, flag] = calculate(Alu::Op::SLTU, 0, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == 0);
  }
  SECTION("-2 < -1") {
    auto [result, flag] = calculate(Alu::Op::SLTU, -2u, -1u);
    REQUIRE(result == 1);
    REQUIRE(flag   == 0);
  }
  SECTION("-2 < 1") {
    auto [result, flag] = calculate(Alu::Op::SLTU, -2u, 1u);
    REQUIRE(result == 0);
    REQUIRE(flag   == 0);
  }
  SECTION("-1 < -2") {
    auto [result, flag] = calculate(Alu::Op::SLTU, -1u, -2u);
    REQUIRE(result == 0);
    REQUIRE(flag   == 0);
  }
}

TEST_CASE("Alu_LTS", "[LTS]") {
  SECTION("1 < 0") {
    auto [result, flag] = calculate(Alu::Op::LTS, 1, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("0 < 1") {
    auto [result, flag] = calculate(Alu::Op::LTS, 0, 1);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("0 < 0") {
    auto [result, flag] = calculate(Alu::Op::LTS, 0, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("-2 < -1") {
    auto [result, flag] = calculate(Alu::Op::LTS, -2u, -1u);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("-2 < 1") {
    auto [result, flag] = calculate(Alu::Op::LTS, -2u, 1u);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("-1 < -2") {
    auto [result, flag] = calculate(Alu::Op::LTS, -1u, -2u);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
}

TEST_CASE("Alu_LTU", "[LTU]") {
  SECTION("1 < 0") {
    auto [result, flag] = calculate(Alu::Op::LTU, 1, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("0 < 1") {
    auto [result, flag] = calculate(Alu::Op::LTU, 0, 1);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("0 < 0") {
    auto [result, flag] = calculate(Alu::Op::LTU, 0, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("-2 < -1") {
    auto [result, flag] = calculate(Alu::Op::LTU, -2u, -1u);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("-2 < 1") {
    auto [result, flag] = calculate(Alu::Op::LTU, -2u, 1u);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("-1 < -2") {
    auto [result, flag] = calculate(Alu::Op::LTU, -1u, -2u);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
}

TEST_CASE("Alu_GES", "[GES]") {
  SECTION("1 >= 0") {
    auto [result, flag] = calculate(Alu::Op::GES, 1, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("0 >= 1") {
    auto [result, flag] = calculate(Alu::Op::GES, 0, 1);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("0 >= 0") {
    auto [result, flag] = calculate(Alu::Op::GES, 0, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("-2 >= -1") {
    auto [result, flag] = calculate(Alu::Op::GES, -2u, -1u);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("-2 >= 1") {
    auto [result, flag] = calculate(Alu::Op::GES, -2u, 1u);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("-1 >= -2") {
    auto [result, flag] = calculate(Alu::Op::GES, -1u, -2u);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
}

TEST_CASE("Alu_GEU", "[GEU]") {
  SECTION("1 >= 0") {
    auto [result, flag] = calculate(Alu::Op::GEU, 1, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("0 >= 1") {
    auto [result, flag] = calculate(Alu::Op::GEU, 0, 1);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("0 >= 0") {
    auto [result, flag] = calculate(Alu::Op::GEU, 0, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("-2 >= -1") {
    auto [result, flag] = calculate(Alu::Op::GEU, -2u, -1u);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("-2 >= 1") {
    auto [result, flag] = calculate(Alu::Op::GEU, -2u, 1u);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("-1 >= -2") {
    auto [result, flag] = calculate(Alu::Op::GEU, -1u, -2u);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
}

TEST_CASE("Alu_EQ", "[EQ]") {
  SECTION("1 == 0") {
    auto [result, flag] = calculate(Alu::Op::EQ, 1, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("0 == 1") {
    auto [result, flag] = calculate(Alu::Op::EQ, 0, 1);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("0 == 0") {
    auto [result, flag] = calculate(Alu::Op::EQ, 0, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("33 == 33") {
    auto [result, flag] = calculate(Alu::Op::EQ, -33u, -33u);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("-2 == -2") {
    auto [result, flag] = calculate(Alu::Op::EQ, -2u, -2u);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
}

TEST_CASE("Alu_NE", "[NE]") {
  SECTION("1 != 0") {
    auto [result, flag] = calculate(Alu::Op::NE, 1, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("0 != 1") {
    auto [result, flag] = calculate(Alu::Op::NE, 0, 1);
    REQUIRE(result == 0);
    REQUIRE(flag   == true);
  }
  SECTION("0 != 0") {
    auto [result, flag] = calculate(Alu::Op::NE, 0, 0);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("33 != 33") {
    auto [result, flag] = calculate(Alu::Op::NE, -33u, -33u);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
  SECTION("-2 != -2") {
    auto [result, flag] = calculate(Alu::Op::NE, -2u, -2u);
    REQUIRE(result == 0);
    REQUIRE(flag   == false);
  }
}

TEST_CASE("Alu class", "[class]") {

  SECTION("2 + 3") {
    REQUIRE(Alu::calc_result(Alu::Op::ADD, 2, 3) == 5);
    REQUIRE(Alu::calc_flag  (Alu::Op::ADD, 2, 3) == 0);
  }
  SECTION("2 == 2") {
    REQUIRE(Alu::calc_result(Alu::Op::EQ, 2, 2) == 0);
    REQUIRE(Alu::calc_flag  (Alu::Op::EQ, 2, 2) == 1);
  }
}
#endif

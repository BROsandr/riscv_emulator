#include "alu.hpp"

#include "exception.hpp"
#include <limits>

namespace {
  using enum Alu::Op;

  std::pair<Uxlen, bool> calculate(Alu::Op op, Uxlen a, Uxlen b) {
      const Uxlen shamt{b & 0b11111};
      Sxlen a_signed{static_cast<Sxlen>(a)};
      Sxlen b_signed{static_cast<Sxlen>(b)};

      Uxlen result{};
      bool  flag  {};

      switch (op) {
        case ADD : result = a + b                                ; break;
        case SUB : result = a - b                                ; break;
        case XOR : result = a ^ b                                ; break;
        case OR  : result = a | b                                ; break;
        case AND : result = a & b                                ; break;
        case SRA : result = static_cast<Uxlen>(a_signed >> shamt); break;
        case SRL : result = a >> shamt                           ; break;
        case SLL : result = a << shamt                           ; break;
        case SLTS: result = a_signed < b_signed                  ; break;
        case SLTU: result = a < b                                ; break;
        case LTS : flag   = a_signed < b_signed                  ; break;
        case LTU : flag   = a < b                                ; break;
        case GES : flag   = a_signed >= b_signed                 ; break;
        case GEU : flag   = a >= b                               ; break;
        case EQ  : flag   = a == b                               ; break;
        case NE  : flag   = a != b                               ; break;
        default  :
          throw Errors::Unknown_alu_op(
              std::to_string(static_cast<std::underlying_type_t<typename Alu::Op>>(op))
          );
      }

      return {result, flag};
  }
}

Alu::Alu(Op op, Uxlen a, Uxlen b) {
  auto out = calculate(op, a, b);

  this->result = out.first;
  this->flag   = out.second;
}

#define UNIT_TEST

#ifdef UNIT_TEST
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

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
#endif

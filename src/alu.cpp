#include "alu.hpp"
#include "exception.hpp"
#include <limits>

void Alu::calculate(Op op, Uxlen a, Uxlen b) {
    const Uxlen shamt{b & 0b11111};
    Sxlen a_signed{static_cast<Sxlen>(a)};
    Sxlen b_signed{static_cast<Sxlen>(b)};

    switch (op) {
      case ADD : this->result = a + b                                ; break;
      case SUB : this->result = a - b                                ; break;
      case XOR : this->result = a ^ b                                ; break;
      case OR  : this->result = a | b                                ; break;
      case AND : this->result = a & b                                ; break;
      case SRA : this->result = static_cast<Uxlen>(a_signed >> shamt); break;
      case SRL : this->result = a >> shamt                           ; break;
      case SLL : this->result = a << shamt                           ; break;
      case SLTS: this->result = a_signed < b_signed                  ; break;
      case SLTU: this->result = a < b                                ; break;
      case LTS : this->flag   = a_signed < b_signed                  ; break;
      case LTU : this->flag   = a < b                                ; break;
      case GES : this->flag   = a_signed >= b_signed                 ; break;
      case GEU : this->flag   = a >= b                               ; break;
      case EQ  : this->flag   = a == b                               ; break;
      case NE  : this->flag   = a != b                               ; break;
      default  :
        throw Errors::Unknown_alu_op(std::to_string(static_cast<std::underlying_type_t<Op>>(op)));
    }
}

#define UNIT_TEST

#ifdef UNIT_TEST
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("Alu_ADD", "[ADD]") {
  Alu alu{};
  SECTION("0 + 0") {
    alu.calculate(Alu::Op::ADD, 0, 0);
    REQUIRE(alu.get_result() == 0);
    REQUIRE(alu.get_flag()   == 0);
  }

  SECTION("-1 + 1") {
    alu.calculate(Alu::Op::ADD, -1u, 1);
    REQUIRE(alu.get_result() == 0);
    REQUIRE(alu.get_flag()   == 0);
  }
  SECTION("1 + 2") {
    alu.calculate(Alu::Op::ADD, 1, 2);
    REQUIRE(alu.get_result() == 3);
    REQUIRE(alu.get_flag  () == 0);
  }

  SECTION("1 + (-2)") {
    alu.calculate(Alu::Op::ADD, 1, -2u);
    REQUIRE(alu.get_result() == -1u);
    REQUIRE(alu.get_flag  () ==  0);
  }

  SECTION("-1 + 2") {
    alu.calculate(Alu::Op::ADD, -1u, 2);
    REQUIRE(alu.get_result() == 1);
    REQUIRE(alu.get_flag  () == 0);
  }

  SECTION("-1 + (-2)") {
    alu.calculate(Alu::Op::ADD, -1u, -2u);
    REQUIRE(alu.get_result() == -3u);
    REQUIRE(alu.get_flag  () ==  0);
  }

  SECTION("max_xlen + 1") {
    alu.calculate(Alu::Op::ADD, std::numeric_limits<Uxlen>::max(), 1);
    REQUIRE(alu.get_result() == 0);
    REQUIRE(alu.get_flag  () == 0);
  }

  SECTION("min_xlen + (-1)") {
    alu.calculate(Alu::Op::ADD, 0, -1u);
    REQUIRE(alu.get_result() == std::numeric_limits<Uxlen>::max());
    REQUIRE(alu.get_flag  () == 0);
  }
}
#endif

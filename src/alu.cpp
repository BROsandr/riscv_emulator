#include "alu.hpp"

void Alu::calculate(Op op, Xlen a, Xlen b) {
    decltype(Xlen) a_signed;
    signed Xlen b_signed;

    switch (op) {
      ALU_ADD : this->result = a + b; break;
      ALU_SUB : this->result = a - b; break;
      ALU_XOR : this->result = a ^ b; break;
      ALU_OR  : this->result = a | b; break;
      ALU_AND : this->result = a & b; break;
      ALU_SRA : {
        
        this->result = $signed(a) >>> b[4:0];
      } break;
      ALU_SRL : this->result = a >> b[4:0];
      ALU_SLL : this->result = a << b[4:0];
      ALU_SLTS: this->result = $signed(a) < $signed(b);
      ALU_SLTU: this->result = a < b;
    }

    this->result = 0;
    this->flag   = 0;
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
    alu.calculate(Alu::Op::ADD, -1, 1);
    REQUIRE(alu.get_result() == 0);
    REQUIRE(alu.get_flag()   == 0);
  }
  SECTION("1 + 2") {
    alu.calculate(Alu::Op::ADD, 1, 2);
    REQUIRE(alu.get_result() == 3);
    REQUIRE(alu.get_flag  () == 0);
  }

  SECTION("1 + (-2)") {
    alu.calculate(Alu::Op::ADD, 1, -2);
    REQUIRE(alu.get_result() == -1);
    REQUIRE(alu.get_flag  () ==  0);
  }

  SECTION("-1 + 2") {
    alu.calculate(Alu::Op::ADD, -1, 2);
    REQUIRE(alu.get_result() == 1);
    REQUIRE(alu.get_flag  () == 0);
  }

  SECTION("-1 + (-2)") {
    alu.calculate(Alu::Op::ADD, -1, -2);
    REQUIRE(alu.get_result() == -3);
    REQUIRE(alu.get_flag  () ==  0);
  }
}
#endif

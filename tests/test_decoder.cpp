#define CATCH_CONFIG_MAIN

#include "decoder.hpp"

#include "exception.hpp"

#include "catch2/catch_test_macros.hpp"

TEST_CASE("Decoder add", "[ADD]") {
  Decoder decoder{};
  SECTION("add x1, x2, x3") {
    Decoder::Instruction_info info{decoder.decode(0x003100b3)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_add);
    REQUIRE(info.type == Decoder::Instruction_type::r);
    REQUIRE(info.rd   == 1);
    REQUIRE(info.rs1  == 2);
    REQUIRE(info.rs2  == 3);
  }
}

TEST_CASE("Decoder addi", "[ADDI]") {
  Decoder decoder{};
  SECTION("addi x1, x2, 256") {
    Decoder::Instruction_info info{decoder.decode(0x10010093)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_addi);
    REQUIRE(info.type == Decoder::Instruction_type::i);
    REQUIRE(info.rd   == 1);
    REQUIRE(info.rs1  == 2);
    REQUIRE(info.imm  == 256);
  }
}

TEST_CASE("Decoder csrw", "[CSRW]") {
  SECTION("enabled zicsr") {
    Decoder decoder{Isa_extension::isa_zicsr};
    SECTION("nothrow") {
      REQUIRE_NOTHROW(decoder.decode(0x30529073));
    }
    SECTION("csrw mtvec, x5") {
      Decoder::Instruction_info info{decoder.decode(0x30529073)};
      REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_csrrw);
      REQUIRE(info.type == Decoder::Instruction_type::i);
      REQUIRE(info.rs1  == 5);
      REQUIRE(info.imm  == 0x305);
    }
  }
  SECTION("disabled zicsr") {
    Decoder decoder{};
    SECTION("throws") {
      REQUIRE_THROWS(decoder.decode(0x30529073));
      try {
        decoder.decode(0x30529073);
      } catch (const Errors::Illegal_instruction &exception) {
        REQUIRE(exception.m_instruction == 0x30529073);
      }
    }
  }
}

TEST_CASE("Decoder lui", "[LUI]") {
  Decoder decoder{};
  SECTION("lui x27, 50") {
    Decoder::Instruction_info info{decoder.decode(0x00032db7)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_lui);
    REQUIRE(info.type == Decoder::Instruction_type::u);
    REQUIRE(info.rd   == 27);
    REQUIRE(info.imm  == 50);
  }
}

TEST_CASE("Decoder auipc", "[AUIPC]") {
  Decoder decoder{};
  SECTION("auipc x7, 379") {
    Decoder::Instruction_info info{decoder.decode(0x0017b397)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_auipc);
    REQUIRE(info.type == Decoder::Instruction_type::u);
    REQUIRE(info.rd   == 7);
    REQUIRE(info.imm  == 379);
  }
}

TEST_CASE("Decoder slti", "[SLTI]") {
  Decoder decoder{};
  SECTION("slti x12, x23, 413") {
    Decoder::Instruction_info info{decoder.decode(0x19dba613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_slti);
    REQUIRE(info.type == Decoder::Instruction_type::i);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 413);
  }
}

TEST_CASE("Decoder sltiu", "[SLTIU]") {
  Decoder decoder{};
  SECTION("sltiu x12, x23, 413") {
    Decoder::Instruction_info info{decoder.decode(0x19dbb613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_sltiu);
    REQUIRE(info.type == Decoder::Instruction_type::i);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 413);
  }
}

TEST_CASE("Decoder xori", "[XORI]") {
  Decoder decoder{};
  SECTION("xori x12, x23, 413") {
    Decoder::Instruction_info info{decoder.decode(0x19dbc613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_xori);
    REQUIRE(info.type == Decoder::Instruction_type::i);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 413);
  }
}

TEST_CASE("Decoder ori", "[ORI]") {
  Decoder decoder{};
  SECTION("ori x12, x23, 413") {
    Decoder::Instruction_info info{decoder.decode(0x19dbe613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_ori);
    REQUIRE(info.type == Decoder::Instruction_type::i);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 413);
  }
}

TEST_CASE("Decoder andi", "[ANDI]") {
  Decoder decoder{};
  SECTION("andi x12, x23, 413") {
    Decoder::Instruction_info info{decoder.decode(0x19dbf613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_andi);
    REQUIRE(info.type == Decoder::Instruction_type::i);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 413);
  }
}

TEST_CASE("Decoder slli", "[SLLI]") {
  Decoder decoder{};
  SECTION("slli x12, x23, 31") {
    Decoder::Instruction_info info{decoder.decode(0x01fb9613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_slli);
    REQUIRE(info.type == Decoder::Instruction_type::i_sh5);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 31);
  }
}

TEST_CASE("Decoder srli", "[SRLI]") {
  Decoder decoder{};
  SECTION("srli x12, x23, 31") {
    Decoder::Instruction_info info{decoder.decode(0x01fbd613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_srli);
    REQUIRE(info.type == Decoder::Instruction_type::i_sh5);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 31);
  }
}

TEST_CASE("Decoder srai", "[SRAI]") {
  Decoder decoder{};
  SECTION("srai x12, x23, 31") {
    Decoder::Instruction_info info{decoder.decode(0x41fbd613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_srai);
    REQUIRE(info.type == Decoder::Instruction_type::i_sh5);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 31);
  }
}

TEST_CASE("Decoder sub", "[SUB]") {
  Decoder decoder{};
  SECTION("sub x12, x23, x15") {
    Decoder::Instruction_info info{decoder.decode(0x40fb8633)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_sub);
    REQUIRE(info.type == Decoder::Instruction_type::r);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.rs2  == 15);
  }
}

TEST_CASE("Decoder sll", "[SLL]") {
  Decoder decoder{};
  SECTION("sll x12, x23, x15") {
    Decoder::Instruction_info info{decoder.decode(0x00fb9633)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_sll);
    REQUIRE(info.type == Decoder::Instruction_type::r);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.rs2  == 15);
  }
}

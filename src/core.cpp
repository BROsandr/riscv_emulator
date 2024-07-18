#include "core.hpp"

#include "alu.hpp"
#include "csr.hpp"
#include "decoder.hpp"
#include "configurator.hpp"
#include "exception.hpp"
#include "lsu.hpp"
#include "riscv.hpp"

#include <cassert>

namespace {
  constexpr Alu::Op to_alu_op(Decoder::Concrete_instruction instr) {
    using enum Decoder::Concrete_instruction;
    switch (instr) {
      case instr_add :
      case instr_addi: return Alu::ADD ;

      case instr_sub : return Alu::SUB ;

      case instr_xor :
      case instr_xori: return Alu::XOR ;

      case instr_ori :
      case instr_or  : return Alu::OR  ;

      case instr_andi:
      case instr_and : return Alu::AND ;

      case instr_srai:
      case instr_sra : return Alu::SRA ;

      case instr_srli:
      case instr_srl : return Alu::SRL ;

      case instr_slli:
      case instr_sll : return Alu::SLL ;

      case instr_slti:
      case instr_slt : return Alu::SLTS;

      case instr_sltiu:
      case instr_sltu : return Alu::SLTU;

      case instr_blt : return Alu::LTS ;
      case instr_bltu: return Alu::LTU ;
      case instr_bge : return Alu::GES ;
      case instr_bgeu: return Alu::GEU ;
      case instr_beq : return Alu::EQ  ;
      case instr_bne : return Alu::NE  ;

      default: assert(0 && "Invalid instr2alu_op conversion");
    }
  }

  enum class Csr_op {
    CSR_RW  = 0b001,
    CSR_RS  = 0b010,
    CSR_RC  = 0b011,
    CSR_RWI = 0b101,
    CSR_RSI = 0b110,
    CSR_RCI = 0b111
  };

  constexpr Csr_op to_csr_op(Decoder::Concrete_instruction instr) {
    using enum Decoder::Concrete_instruction;
    using enum Csr_op;
    switch (instr) {
      case instr_csrrw : return CSR_RW;
      case instr_csrrs : return CSR_RS;
      case instr_csrrc : return CSR_RC;
      case instr_csrrwi: return CSR_RWI;
      case instr_csrrsi: return CSR_RSI;
      case instr_csrrci: return CSR_RCI;
      default: assert(0 && "Invalid instr2csr_op conversion.");
    }
  }

  constexpr Lsu::Op to_lsu_op(Decoder::Concrete_instruction instr) {
    using enum Decoder::Concrete_instruction;
    using enum Lsu::Op;
    switch (instr) {
      case instr_sb:
      case instr_lb: return b;

      case instr_sh:
      case instr_lh: return h;

      case instr_sw:
      case instr_lw: return w;

      case instr_lbu: return bu;
      case instr_lhu: return hu;

      default: assert(0 && "Invalid instr2lsu_op conversion");
    }
  }

  enum class Handler_type {
    type_calc_reg,
    type_calc_imm,
    type_load,
    type_store,
    type_branch,
    type_csr_imm,
    type_csr_reg,
    type_jal,
    type_jalr,
    type_lui,
    type_auipc,
    type_mret,
    type_fence,
  };

  constexpr Handler_type to_handler_type(Decoder::Concrete_instruction instr) {
    using enum Decoder::Concrete_instruction;
    using enum Handler_type;
    switch (instr) {
      case instr_lb    :
      case instr_lh    :
      case instr_lw    :
      case instr_lbu   :
      case instr_lhu   : return type_load;

      case instr_sb    :
      case instr_sh    :
      case instr_sw    : return type_store;

      case instr_beq   :
      case instr_bne   :
      case instr_blt   :
      case instr_bge   :
      case instr_bltu  :
      case instr_bgeu  : return type_branch;

      case instr_addi  :
      case instr_slti  :
      case instr_sltiu :
      case instr_xori  :
      case instr_ori   :
      case instr_andi  :
      case instr_slli  :
      case instr_srli  :
      case instr_srai  : return type_calc_imm;

      case instr_add   :
      case instr_sub   :
      case instr_sll   :
      case instr_slt   :
      case instr_sltu  :
      case instr_xor   :
      case instr_srl   :
      case instr_sra   :
      case instr_or    :
      case instr_and   : return type_calc_reg;

      case instr_csrrw :
      case instr_csrrs :
      case instr_csrrc : return type_csr_reg;

      case instr_csrrwi:
      case instr_csrrsi:
      case instr_csrrci: return type_csr_imm;

      case instr_jal : return type_jal;
      case instr_jalr: return type_jalr;

      case instr_auipc: return type_auipc;
      case instr_lui  : return type_lui;

      case instr_fence: return type_fence;
      case instr_mret : return type_mret;
    }

    assert(0 && "Invalid instr2handler_type conversion");
  }

  constexpr void handle_type_calc_imm(const Decoder::Instruction_info &instr_info, Memory &rf) {
    const Uxlen a{rf.read(instr_info.rs1)};
    const Uxlen alu_res{Alu::calc_result(to_alu_op(instr_info.instruction), a, instr_info.imm)};
    rf.write(instr_info.rd, alu_res);
  }

  constexpr void handle_type_calc_reg(const Decoder::Instruction_info &instr_info, Memory &rf) {
    const Uxlen a{rf.read(instr_info.rs1)};
    const Uxlen b{rf.read(instr_info.rs2)};
    const Uxlen alu_res{Alu::calc_result(to_alu_op(instr_info.instruction), a, b)};
    rf.write(instr_info.rd, alu_res);
  }

  constexpr void handle_type_store(const Decoder::Instruction_info &instr_info, Memory &rf, Memory &data_mem) {
    const std::size_t addr{rf.read(instr_info.rs1) + instr_info.imm};
    const Uxlen data{rf.read(instr_info.rs2)};
    const auto lsu_op = to_lsu_op(instr_info.instruction);
    if (Lsu::is_misaligned(lsu_op, addr)) {
      throw Errors::Misalignment{addr, "lsu_op: " + to_string(lsu_op)};
    }
    data_mem.write(addr, data, Lsu::get_be(lsu_op, addr));
  }

  constexpr void handle_type_load(const Decoder::Instruction_info &instr_info, Memory &rf, Memory &data_mem) {
    const std::size_t addr{rf.read(instr_info.rs1) + instr_info.imm};
    const auto lsu_op = to_lsu_op(instr_info.instruction);
    if (Lsu::is_misaligned(lsu_op, addr)) {
      throw Errors::Misalignment{addr, "lsu_op: " + to_string(lsu_op)};
    }
    Uxlen data{data_mem.read(addr, Lsu::get_be(lsu_op, addr))};
    data = Lsu::transform_data(lsu_op, addr, data);
    rf.write(instr_info.rd, data);
  }

  constexpr void handle_type_branch(const Decoder::Instruction_info &instr_info, Memory &rf, auto &pc) {
    const Uxlen a{rf.read(instr_info.rs1)};
    const Uxlen b{rf.read(instr_info.rs2)};
    const Uxlen alu_flag{Alu::calc_flag(to_alu_op(instr_info.instruction), a, b)};
    if (alu_flag) pc += instr_info.imm;
  }

  constexpr void handle_type_auipc(const Decoder::Instruction_info &instr_info, Memory &rf, const auto &pc) {
    rf.write(instr_info.rd, pc + (instr_info.imm << 12));
  }

  constexpr void handle_type_lui(const Decoder::Instruction_info &instr_info, Memory &rf) {
    rf.write(instr_info.rd, instr_info.imm << 12);
  }

  constexpr void handle_type_jal(const Decoder::Instruction_info &instr_info, Memory &rf, auto &pc) {
    rf.write(instr_info.rd, pc + 4);
    pc += instr_info.imm;
  }

  constexpr void handle_type_jalr(const Decoder::Instruction_info &instr_info, Memory &rf, auto &pc) {
    rf.write(instr_info.rd, pc + 4);
    pc = (rf.read(instr_info.rs1) + instr_info.imm) & make_mask<Uxlen>(32, 1);
  }

  constexpr Uxlen exec_csr_op(Memory &csr, Csr_op op, std::size_t addr, Uxlen data) {
    using enum Csr_op;
    Uxlen res{csr.read(addr)};
    switch (op) {
      case CSR_RW: case CSR_RWI:
        csr.write(addr, data);
        break;
      case CSR_RS: case CSR_RSI:
        csr.write(addr, res | data);
        break;
      case CSR_RC: case CSR_RCI:
        csr.write(addr, res & ~data);
        break;
      default: assert(0 && "Illegal csr op.");
    }
    return res;
  }

  constexpr void handle_type_csr_imm(const Decoder::Instruction_info &instr_info, Memory &rf, Memory &csr) {
    const Csr_op op{to_csr_op(instr_info.instruction)};
    const Uxlen rd_data{exec_csr_op(csr, op, instr_info.imm, instr_info.rs1)};
    rf.write(instr_info.rd, rd_data);
  }

  constexpr void handle_type_csr_reg(const Decoder::Instruction_info &instr_info, Memory &rf, Memory &csr) {
    const Csr_op op{to_csr_op(instr_info.instruction)};
    const Uxlen rd_data{exec_csr_op(csr, op, instr_info.imm,
        rf.read(instr_info.rs1))};
    rf.write(instr_info.rd, rd_data);
  }

  void handle_type_mret(std::function<void(void)> return_from_irq, Memory &csr, auto &pc) {
    const auto mepc = csr.read(Csr::Register::MEPC);
    pc = mepc;
    return_from_irq();
  }
}

void Core::cycle() {
  const Uxlen instruction{fetch_instruction()};
  const Decoder decoder{Configurator::get_instance().create_decoder()};
  const Decoder::Instruction_info instr_info{decoder.decode(instruction)};
  Memory &rf{m_rf};
  Memory &csr{m_csr};
  Memory &data_mem{m_data_mem};
  auto &pc = m_pc;
  auto return_from_irq = m_return_from_irq;

  switch (to_handler_type(instr_info.instruction)) {
    case Handler_type::type_calc_imm: handle_type_calc_imm(instr_info, rf); break;
    case Handler_type::type_calc_reg: handle_type_calc_reg(instr_info, rf); break;
    case Handler_type::type_store: handle_type_store(instr_info, rf, data_mem); break;
    case Handler_type::type_load: handle_type_load(instr_info, rf, data_mem); break;
    case Handler_type::type_branch: handle_type_branch(instr_info, rf, pc); return;
    case Handler_type::type_auipc: handle_type_auipc(instr_info, rf, pc); break;
    case Handler_type::type_lui: handle_type_lui(instr_info, rf); break;
    case Handler_type::type_fence: break;
    case Handler_type::type_jal: handle_type_jal(instr_info, rf, pc); return;
    case Handler_type::type_jalr: handle_type_jalr(instr_info, rf, pc); return;
    case Handler_type::type_csr_imm: handle_type_csr_imm(instr_info, rf, csr); break;
    case Handler_type::type_csr_reg: handle_type_csr_reg(instr_info, rf, csr); break;
    case Handler_type::type_mret: handle_type_mret(return_from_irq, csr, pc); break;
  }

  pc += 4;

}

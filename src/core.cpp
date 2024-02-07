#include "core.hpp"
#include "alu.hpp"
#include "decoder.hpp"

void Core::increment_pc() {
  pc += 4;
}

void Core::cycle() {
  const Uxlen instruction{fetch_instruction(this->instr_mem)};

  Decoder::Callbacks callbacks {
    .op_imm = [this](Alu::Op op, std::size_t ra1, Uxlen imm_i, std::size_t wa) {
      const Uxlen a{rf.read(ra1)};
      Alu alu{op, a, imm_i};
      rf.write(wa, alu.get_result());
      increment_pc();
    },
    .op = [this](Alu::Op op, std::size_t ra1, std::size_t ra2, std::size_t wa) {
      const Uxlen a{rf.read(ra1)};
      const Uxlen b{rf.read(ra2)};
      Alu alu{op, a, b};
      rf.write(wa, alu.get_result());
      increment_pc();
    },
    .load = [this](unsigned int funct3, std::size_t ra1, Uxlen imm_i, std::size_t wa) {
      
    }
  };

  const Decoder decoder{instruction};


  Execute_stage execute_stage{decode_stage, rd1, rd2};

  Control_unit control_unit{decode_stage, this->current_pc, rd1};

  write_back();

  calculate_next_pc(execute_stage.get_result(), control_unit.get_csr_result(), this->data_mem.read();

}

#include "core.hpp"

#include "alu.hpp"
#include "decoder.hpp"
#include "factory.hpp"

void Core::increment_pc() {
  pc += 4;
}

void Core::cycle() {
  Decoder decoder{Factory::get_instance().create_decoder()};

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
      Uxlen a{rf.read(ra1)};
      a += imm_i;
      rf.write(load(funct3, a));
      increment_pc();
    },
    .store = [this](unsigned int funct3, std::size_t ra1, std::size_t ra2, Uxlen imm_i) {
      const Uxlen b{rf.read(ra2)};
      Uxlen a{rf.read(ra1)};
      a += imm_i;
      store(funct3, a, b);
      increment_pc();
    },
    .lui = [this](Uxlen imm_u, std::size_t wa) {
      rf.write(wa, imm_u << 12);
      increment_pc();
    },
    .auipc = [this](Uxlen imm_u, std::size_t wa) {
      rf.write(wa, pc + (imm_u << 12));
      increment_pc();
    },
    .jalr = [this](std::size_t ra1, Uxlen imm_j, std::size_t wa) {
      rf.write(wa, pc + 4);
      pc = rf.read(rf.read(ra1) + imm_j);
    },
    .jal = [this](Uxlen imm_j, std::size_t wa) {
      rf.write(wa, pc + 4);
      pc += imm_j;
    },
    .branch = [this](Alu::Op op, std::size_t ra1, std::size_t ra2, Uxlen imm_b) {
      const Uxlen a{rf.read(ra1)};
      const Uxlen b{rf.read(ra2)};
      Alu alu{op, a, b};
      if (alu.get_flag()) pc += imm_b;
    },
    .system = [this](Csr::Op op, std::size_t ra1, std::size_t wa, Uxlen imm_zicsr, bool mret, bool csr_we) {
      if (mret) {
        pc = csr.get_mepc();
      } else {
        const Uxlen a{rf.read(ra1)};
        const Uxlen wd{csr(op, a, imm_zicsr, csr_we)};
        rf.write(wa, wd);
      }
    }

  };

  const Decoder decoder{instruction};


  Execute_stage execute_stage{decode_stage, rd1, rd2};

  Control_unit control_unit{decode_stage, this->current_pc, rd1};

  write_back();

  calculate_next_pc(execute_stage.get_result(), control_unit.get_csr_result(), this->data_mem.read();

}

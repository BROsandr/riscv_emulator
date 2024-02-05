#include "Core.hpp"

void Core::cycle() {
  const Xlen instruction{fetch_instruction(this->instr_mem)};

  const Decode_stage decode_stage{instruction};

  const Xlen rd1{this->rf.get(decode_stage.get_ra1())};
  const Xlen rd2{this->rf.get(decode_stage.get_ra2())};

  Execute_stage execute_stage{decode_stage, rd1, rd2};

  Control_unit control_unit{decode_stage, this->current_pc, rd1};

  write_back();

  calculate_next_pc(execute_stage.get_result(), control_unit.get_csr_result(), this->data_mem.read();

}

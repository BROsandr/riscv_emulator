#include "Core.hpp"

void Core::cycle() {
  Fetch_stage fectch_stage{this->instr_mem};

  Memory_stage memory_stage{this->data_mem};
  Decoder decoder{};

  while (true) {
    Xlen instr = fectch_stage.get_instruction();

    decoder.decode(instr);

    Execute_stage execute_stage{docoder.get_unit_select, decoder.get_op, decoder.get_rs1, decoder.get_rs2};
    execute_stage.execute();

    Csr csr

    fectch_stage.calculate_next_pc();

  }

}

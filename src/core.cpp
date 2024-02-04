
void run(Memory instr_mem, Memory data_mem, Irq irq) {
  Fetch_stage fectch_stage{instr_mem};

  Memory_stage memory_stage{data_mem};
  Decoder decoder{};

  while (true) {
    instr = fectch_stage.get_instruction();

    decoder.decode(instr);

    Execute_stage execute_stage{dococder.get_unit_select, decoder.get_op, decoder.get_rs1, decoder.get_rs2};
    execute_stage.execute();

    Csr csr

    fectch_stage.calculate_next_pc();

  }

}

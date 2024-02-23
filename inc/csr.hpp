class Csr {
  public:
    enum class Op {
      CSR_RW  = 0b001,
      CSR_RS  = 0b010,
      CSR_RC  = 0b011,
      CSR_RWI = 0b101,
      CSR_RSI = 0b110,
      CSR_RCI = 0b111
    };
};

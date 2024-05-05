#include "alu.hpp"

#include <string>
#include <cassert>

namespace {
  using enum Alu::Op;

  constexpr std::pair<Uxlen, bool> calculate(Alu::Op op, Uxlen a, Uxlen b) {
      const Sxlen a_signed{static_cast<Sxlen>(a)};
      const Sxlen b_signed{static_cast<Sxlen>(b)};

      const auto get_shamt = [b]() constexpr {
        constexpr Uxlen shamt_mask{(sizeof(Uxlen) * 8) - 1};
        assert((b & shamt_mask) == b);
        return b;
      };

      Uxlen result{};
      bool  flag  {};

      switch (op) {
        case ADD : result = a + b                                      ; break;
        case SUB : result = a - b                                      ; break;
        case XOR : result = a ^ b                                      ; break;
        case OR  : result = a | b                                      ; break;
        case AND : result = a & b                                      ; break;
        case SRA : result = static_cast<Uxlen>(a_signed >> get_shamt()); break;
        case SRL : result = a >> get_shamt()                           ; break;
        case SLL : result = a << get_shamt()                           ; break;
        case SLTS: result = a_signed < b_signed                        ; break;
        case SLTU: result = a < b                                      ; break;
        case LTS : flag   = a_signed < b_signed                        ; break;
        case LTU : flag   = a < b                                      ; break;
        case GES : flag   = a_signed >= b_signed                       ; break;
        case GEU : flag   = a >= b                                     ; break;
        case EQ  : flag   = a == b                                     ; break;
        case NE  : flag   = a != b                                     ; break;
        default  : assert((void("Unknown_alu_op " + std::to_string(op)), 0));
      }

      return {result, flag};
  }
}

namespace Alu {
  Uxlen calc_result(Alu::Op op, Uxlen a, Uxlen b) {
    const auto [result, flag] = calculate(op, a, b);
    return result;
  }

  bool calc_flag(Alu::Op op, Uxlen a, Uxlen b) {
    const auto [result, flag] = calculate(op, a, b);
    return flag;
  }
}

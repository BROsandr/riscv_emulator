#pragma once

#include "riscv.hpp"

#include <stdexcept>

namespace Errors {
  struct Error : public std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  struct Illegal_instruction : public Error {
    const Uxlen m_instruction;

    Illegal_instruction(Uxlen instruction, const std::string &message = "")
        : Error(std::to_string(instruction) + " : " + message),  m_instruction{instruction} {}
  };

  struct Illegal_addr : public Error {
    using Addr = std::size_t;
    Addr m_addr{};

    Illegal_addr(Addr addr, const std::string &message = "")
        : Error("Illegal address " + std::to_string(addr) + " : " + message),  m_addr{addr} {}
  };

  struct Read_only : public Error {
    Read_only(const std::string &message = "")
        : Error("Write to read only memory : " + message) {}
  };

  struct Misalignment : public Error {
    using Addr = std::size_t;
    Addr m_addr{};

    Misalignment(Addr addr, const std::string &message = "")
        : Error("Misaligment. Addr:" + std::to_string(addr) + " : " + message),
          m_addr{addr} {}
  };
}

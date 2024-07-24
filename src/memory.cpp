#include "memory.hpp"

#include "spdlog/logger.h"

void Traced_mem_span::write(std::size_t addr, Uxlen data, unsigned int byte_en) {
  m_logger->info("{} write addr:0x{:x}, data:0x{:x}, byte_en:0x{:x}", m_msg, addr, data,
      byte_en);
  m_mem.write(addr, data, byte_en);
}

[[nodiscard]] Uxlen Traced_mem_span::read (std::size_t addr, unsigned int byte_en) {
  const Uxlen data{m_mem.read(addr, byte_en)};
  m_logger->info("{} read addr:0x{:x}, data:0x{:x}, byte_en:0x{:x}", m_msg, addr, data,
      byte_en);
  return data;
}

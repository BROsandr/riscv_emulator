
#include "core.hpp"

#include "exception.hpp"
#include "instr_mem.hpp"
#include "data_mem.hpp"
#include "csr.hpp"
#include "isa_extension.hpp"
#include "memory.hpp"
#include "rf.hpp"

#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

template <typename D, typename R, typename C>
class Mem_requirements {
  public:
    Mem_requirements(const D &data_mem, const R &rf, const C &csr)
        : m_data_mem{data_mem}, m_rf{rf}, m_csr{csr} {}

    void eq(const auto &ref_data_content, const auto &ref_rf_content,
        const auto &ref_csr_content) const {
      REQUIRE(m_data_mem.get_content() == ref_data_content);
      REQUIRE(m_rf.get_content()       == ref_rf_content  );
      REQUIRE(m_csr.get_content()      == ref_csr_content );
    }

  private:
    const D &m_data_mem;
    const R &m_rf;
    const C &m_csr;
};

using Byte = std::byte;

TEST_CASE("basic", "[BASIC]") {

  auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
  auto my_logger = std::make_shared<spdlog::logger>("console", sink);
  my_logger->set_level(spdlog::level::debug);
  Isa_ext_container extensions{Isa_extension::isa_zicsr};
  my_logger->debug("--------------------------------------------------------------------");

  SECTION("single_instr_li") {
    const std::vector<Uxlen> instr{0x00100093}; // addi x1 x0 1
    Instr_mem instr_mem{std::move(instr)};
    Traced_mem_wrap traced_instr_mem{instr_mem, my_logger, "Instr_mem"};
    std::unordered_map<std::size_t, std::byte> data{};
    Data_mem data_mem{std::move(data)};
    Traced_mem_wrap traced_data_mem{data_mem, my_logger, "Data_mem"};
    Rf rf{};
    Traced_mem_wrap traced_rf{rf, my_logger, "Rf"};
    Csr csr{};
    Traced_mem_wrap traced_csr{csr, my_logger, "Csr"};
    auto ref_rf_content = rf.get_content();
    auto ref_csr_content = csr.get_content();
    auto ref_data_content = data_mem.get_content();

    Core core{traced_instr_mem, traced_data_mem, traced_csr, traced_rf,
        my_logger, extensions};

    Mem_requirements mem_reqs{data_mem, rf, csr};

    try {
      core.cycle();
    } catch (const Errors::Error& exc) {
      my_logger->critical(exc.what());
    } catch (...) {
      my_logger->critical("Unhandled exception");
    }

    ref_rf_content[1] = 1;
    mem_reqs.eq(ref_data_content, ref_rf_content, ref_csr_content);
  }

  SECTION("data_mem") {
    const std::vector<Uxlen> instr{0xfff00193,
        0x00400213, 0x00322023}; // addi x3, x0, -1; li x4, 4; sw x3, 0x0(x4)
    Instr_mem instr_mem{std::move(instr)};
    Traced_mem_wrap traced_instr_mem{instr_mem, my_logger, "Instr_mem"};
    std::unordered_map<std::size_t, std::byte> data{};
    Data_mem data_mem{std::move(data)};
    Traced_mem_wrap traced_data_mem{data_mem, my_logger, "Data_mem"};
    Rf rf{};
    Traced_mem_wrap traced_rf{rf, my_logger, "Rf"};
    Csr csr{};
    Traced_mem_wrap traced_csr{csr, my_logger, "Csr"};
    auto ref_rf_content = rf.get_content();
    auto ref_csr_content = csr.get_content();
    auto ref_data_content = data_mem.get_content();

    Core core{traced_instr_mem, traced_data_mem, traced_csr, traced_rf,
        my_logger, extensions};

    Mem_requirements mem_reqs{data_mem, rf, csr};

    try {
      for (auto instr_num = instr.size(); instr_num != 0; --instr_num) {
        core.cycle();
      }
    } catch (const Errors::Error& exc) {
      my_logger->critical(exc.what());
    } catch (...) {
      my_logger->critical("Unhandled exception");
    }

    ref_rf_content[3] = 0xffffffff;
    ref_rf_content[4] = 4;
    ref_data_content[4] = Byte{0xff};
    ref_data_content[5] = Byte{0xff};
    ref_data_content[6] = Byte{0xff};
    ref_data_content[7] = Byte{0xff};
    mem_reqs.eq(ref_data_content, ref_rf_content, ref_csr_content);
  }
}

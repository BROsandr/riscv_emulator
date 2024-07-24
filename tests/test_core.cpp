
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

#include <map>

template <typename D, typename R, typename C>
class Mem_requirements {
  public:
    Mem_requirements(const D &data_mem, const R &rf, const C &csr)
        : m_data_mem{data_mem}, m_rf{rf}, m_csr{csr} {}

    void eq(auto &ref_data_content, auto &ref_rf_content, auto &ref_csr_content) const {
      REQUIRE(m_data_mem.get_content() == ref_data_content);
      REQUIRE(m_rf.get_content()       == ref_rf_content  );
      REQUIRE(m_csr.get_content()      == ref_csr_content );
    }

  private:
    const D &m_data_mem;
    const R &m_rf;
    const C &m_csr;
};

TEST_CASE("basic", "[BASIC]") {

  auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
  auto my_logger = std::make_shared<spdlog::logger>("console", sink);
  my_logger->set_level(spdlog::level::debug);
  Isa_ext_container extensions{Isa_extension::isa_zicsr};

  SECTION("single_instr1") {
    const std::vector<Uxlen> instr{0x00100093}; // addi x1 x0 1
    Instr_mem instr_mem{std::move(instr)};
    Traced_mem_wrap traced_instr_mem{instr_mem, my_logger, "Instr_mem"};
    std::map<std::size_t, std::byte> data{};
    Data_mem data_mem{std::move(data)};
    Traced_mem_wrap traced_data_mem{data_mem, my_logger, "Data_mem"};
    Rf rf{};
    Traced_mem_wrap traced_rf{rf, my_logger, "Rf"};
    Csr csr{};
    Traced_mem_wrap traced_csr{csr, my_logger, "Csr"};
    auto ref_rf_content = rf.get_content();
    auto ref_csr_content = csr.get_content();
    auto ref_data_content = data_mem.get_content();

    Core core{traced_instr_mem, data_mem, traced_csr, traced_rf,
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
}

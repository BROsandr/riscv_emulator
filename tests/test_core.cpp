
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
    Rf rf{};
    Csr csr{};

    Core core{traced_instr_mem, data_mem, csr, rf, my_logger,
        extensions};

    try {
      core.cycle();
    } catch (const Errors::Error& exc) {
      my_logger->critical(exc.what());
    } catch (...) {
      my_logger->critical("Unhandled exception");
    }

    REQUIRE(rf.get_content()[1] == 1);
  }
}

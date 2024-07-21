#pragma once

#include "decoder.hpp"
#include "config_db.hpp"

class Configurator {
  public:
    static Configurator &get_instance() {
      static Configurator instance{};
      return instance;
    }

    Configurator(const Configurator& ) = delete;
    Configurator(      Configurator&&) = delete;

    Configurator& operator=(const Configurator& ) = delete;
    Configurator& operator=(      Configurator&&) = delete;

    Decoder create_decoder() const {
      return Decoder{config_db.get_isa_extensions()};
    }
  private:
    Configurator() : config_db{Config_db::get_instance()} {};
    ~Configurator() = default;

    const Config_db &config_db;
};

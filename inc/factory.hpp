#pragma once

#include "decoder.hpp"
#include "config_db.hpp"

class Factory {
  public:
    static Factory &get_instance() {
      static Factory instance{};
      return instance;
    }

    Factory(const Factory& ) = delete;
    Factory(      Factory&&) = delete;

    Factory& operator=(const Factory& ) = delete;
    Factory& operator=(      Factory&&) = delete;

    constexpr Decoder create_decoder() const {
      return Decoder{config_db.get_isa_extensions()};
    }
  private:
    Factory() : config_db{Config_db::get_instance()} {};
    ~Factory() = default;

    const Config_db &config_db;
};

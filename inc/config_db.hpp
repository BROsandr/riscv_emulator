#pragma once

#include "isa_extension.hpp"

class Config_db {
  public:
    static Config_db &get_instance() {
      static Config_db instance{};
      return instance;
    }

    Config_db &set_isa_extensions(const Isa_ext_container &isa_extensions) {
      get_instance().m_isa_extensions = isa_extensions;
      return get_instance();
    }

    const Isa_ext_container &get_isa_extensions() const {
      return get_instance().m_isa_extensions;
    }

    Isa_ext_container get_isa_extensions() {
      return get_instance().m_isa_extensions;
    }

    Config_db(const Config_db& ) = delete;
    Config_db(      Config_db&&) = delete;

    Config_db& operator=(const Config_db& ) = delete;
    Config_db& operator=(      Config_db&&) = delete;


  private:
    Config_db()  = default;
    ~Config_db() = default;

    Isa_ext_container m_isa_extensions{};
};

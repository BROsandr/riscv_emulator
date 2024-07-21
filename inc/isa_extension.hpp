#pragma once

#include <bitset>
#include <initializer_list>
#include <algorithm>

enum class Isa_extension {
  isa_zicsr,
  isa_number_
};

class Isa_ext_container {
  private:
    using Base_bitset = std::bitset<static_cast<std::size_t>(Isa_extension::isa_number_)>;

  public:
    Isa_ext_container(std::initializer_list<Isa_extension> extensions) {
      std::for_each(extensions.begin(), extensions.end(),
          [this](const auto &extension) {
            set(extension);
          });
    }
    Isa_ext_container(Isa_extension extension)
        : Isa_ext_container({extension}) {}
    Isa_ext_container() = default;

    bool operator[](Isa_extension extension) const {
      return m_extensions[static_cast<std::size_t>(extension)];
    }
    Base_bitset::reference operator[](Isa_extension extension) {
      return m_extensions[static_cast<std::size_t>(extension)];
    }

    Isa_ext_container& set() {
      m_extensions.set();
      return *this;
    }
    Isa_ext_container& set(Isa_extension extension, bool value = true) {
      m_extensions.set(static_cast<std::size_t>(extension), value);
      return *this;
    }

  private:
    Base_bitset m_extensions{};
};

#pragma once

#include "exception.hpp"
#include "memory.hpp"

#include <map>
#include <vector>

class Bus : public Memory {
  private:
    using Nodes = std::map<std::size_t, Memory&>;
    Nodes nodes{};

  public:
    void attach(std::size_t start_addr, Memory &node_mem) {
      bool was_inserted{nodes.insert_or_assign(start_addr, node_mem).second};
      if (!was_inserted) {
        throw Errors::Illegal_addr{start_addr,
            "Bus. Failed to attach at start_addr=" + std::to_string(start_addr)};
      }
    }

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      Nodes::value_type node{try_get_node(addr)};
      node.second.write(addr - node.first, data, byte_en);
    }
    [[nodiscard]] Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      Nodes::value_type node{try_get_node(addr)};
      return node.second.read(addr - node.first, byte_en);
    }
  private:
    Nodes::value_type try_get_node(std::size_t addr) const {
      if (nodes.empty()) {
        Errors::Error{"Bus. There isn't a node attached to the bus."};
      }
      Nodes::const_iterator it{nodes.lower_bound(addr)};
      if ((it == nodes.end()) || (it->first >= addr)) {
        return *--it;
      } else if (it->first == addr) {
        return *it;
      } else {
        throw Errors::Illegal_addr{addr,
            "Bus. Failed to get a node at addr=" + std::to_string(addr)};
      }
    }
};

class Common_write_bus : public Memory {
  public:
    explicit Common_write_bus(Memory &rw_node) : nodes(1, &rw_node) {};

    void attach(Memory *write_only_node) {
      nodes.push_back(write_only_node);
    }

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      for (auto *el : nodes) el->write(addr, data, byte_en);
    }

    [[nodiscard]] Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      assert(nodes.at(0));
      return nodes[0]->read(addr, byte_en);
    }

  private:
    std::vector<Memory*> nodes{};
};

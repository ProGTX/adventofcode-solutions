// https://adventofcode.com/2022/day/7

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "../common.h"

class btree {
 private:
  using children_t = std::vector<std::unique_ptr<btree>>;

 public:
  using const_iterator = typename children_t::const_iterator;

  enum type_t {
    folder,
    file,
  };

  btree() : name{"/"} {}

  btree* add_file(std::string name_, int size_) {
    if (type == file) {
      throw std::runtime_error("Files cannot add files");
    }
    children.push_back(
        std::unique_ptr<btree>{new btree{this, name_, size_, file}});
    return children.back().get();
  }

  btree* add_folder(std::string name_) {
    if (type == file) {
      throw std::runtime_error("Files cannot add folders");
    }
    children.push_back(
        std::unique_ptr<btree>{new btree{this, name_, 0, folder}});
    return children.back().get();
  }

  friend void set_sizes(btree& node) {
    if (node.type == file) {
      return;
    }
    int sum = 0;
    for (const auto& child : node.children) {
      set_sizes(*child);
      sum += child->size;
    }
    node.size = sum;
  }

  btree* get_child(std::string_view child_name) const {
    if (child_name == "..") {
      return parent;
    }
    auto it = std::ranges::find_if(children, [&](auto& child_ptr) {
      return (child_ptr->name == child_name);
    });
    if (it == std::end(children)) {
      return nullptr;
    }
    return it->get();
  }

  btree* get_parent() const { return parent; }

  std::string_view get_name() const { return name; }

  int get_size() const { return size; }

  type_t get_type() const { return type; }

  const_iterator begin() const { return std::begin(children); }

  const_iterator end() const { return std::end(children); }

 private:
  btree(btree* parent_, std::string name_, int size_, type_t type_)
      : parent{parent_}, name{name_}, size{size_}, type{type_} {}

  children_t children;
  btree* parent = nullptr;
  std::string name{"/"};
  int size = 0;
  type_t type = folder;
};

template <int max_size>
int sum_folder_sizes(const btree* node) {
  int current_size = node->get_size();
  int sum = ((node->get_type() == btree::folder) && (current_size <= max_size))
                ? current_size
                : 0;
  for (const auto& child : *node) {
    sum += sum_folder_sizes<max_size>(child.get());
  }
  return sum;
}

using folder_sizes_t = std::vector<std::pair<std::string_view, int>>;

template <int max_used_space>
void get_folder_sizes(btree* node, const int used_space,
                      folder_sizes_t& folder_sizes) {
  for (const auto& child : *node) {
    if (child->get_type() != btree::folder) {
      continue;
    }
    int current_size = child->get_size();
    if ((max_used_space - used_space + current_size) > 0) {
      folder_sizes.emplace_back(child->get_name(), current_size);
    }
    get_folder_sizes<max_used_space>(child.get(), used_space, folder_sizes);
  }
}

template <bool delete_space>
void solve_case(const std::string& filename) {
  btree filesystem;
  btree* current_node = &filesystem;

  readfile_op(filename, [&](std::string_view line) {
    auto [instruction, name, cd_to] =
        split<std::array<std::string, 3>>(std::string{line}, ' ');
    if (instruction[0] == '$') {
      auto command = name;
      if (name == "cd") {
        if (cd_to == "/") {
          current_node = &filesystem;
          return;
        }
        auto child_node = current_node->get_child(cd_to);
        if (child_node == nullptr) {
          current_node = current_node->add_folder(std::move(cd_to));
        } else {
          current_node = child_node;
        }
      } else if (name == "ls") {
        // Don't have to do anything with ls
      } else {
        throw std::runtime_error("Unknown instruction " + name);
      }
      return;
    }
    auto child_node = current_node->get_child(name);
    if (instruction == "dir") {
      if (child_node == nullptr) {
        current_node->add_folder(std::move(name));
      } else {
        // Folder already exists
      }
    } else {
      // File
      auto filesize = std::stoi(instruction);
      if (child_node == nullptr) {
        current_node->add_file(std::move(name), filesize);
      } else {
        // File already exists
      }
    }
  });

  set_sizes(filesystem);

  int space = 0;
  if constexpr (!delete_space) {
    space = sum_folder_sizes<100000>(&filesystem);
  } else {
    static constexpr int max_space = 70'000'000;
    static constexpr int required_free_space = 30'000'000;
    folder_sizes_t folder_sizes;
    get_folder_sizes<max_space - required_free_space>(
        &filesystem, filesystem.get_size(), folder_sizes);
    std::ranges::sort(folder_sizes,
                      [](auto&& name_size_lhs, auto&& name_size_rhs) {
                        return (name_size_lhs.second < name_size_rhs.second);
                      });
    space = folder_sizes[0].second;
  }

  std::cout << filename << " -> " << space << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<false>("day07.example");
  solve_case<false>("day07.input");
  // std::cout << "Part 2" << std::endl;
  solve_case<true>("day07.example");
  solve_case<true>("day07.input");
}

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

class filesystem_t : public btree<std::string, filesystem_t> {
 private:
  using base_t = btree<std::string, filesystem_t>;

 public:
  enum type_t {
    folder,
    file,
  };

  filesystem_t() : btree{nullptr, "/", false} {}

  filesystem_t* add_file(std::string name, int size_) {
    return this->add_child(std::unique_ptr<filesystem_t>{
        new filesystem_t{this, std::move(name), size_, file}});
  }

  filesystem_t* add_folder(std::string name) {
    return this->add_child(std::unique_ptr<filesystem_t>{
        new filesystem_t{this, std::move(name), 0, folder}});
  }

  filesystem_t* get_child(std::string_view child_name) const {
    if (child_name == "..") {
      return this->get_parent();
    }
    return base_t::get_child(child_name);
  }

  friend void set_sizes(filesystem_t& node) {
    if (node.get_type() == file) {
      return;
    }
    int sum = 0;
    for (const auto& child : node) {
      set_sizes(*child);
      sum += child->size;
    }
    node.size = sum;
  }

  int get_size() const { return size; }

  type_t get_type() const { return this->is_leaf() ? file : folder; }

  std::string_view get_name() const { return this->get_value(); }

 private:
  filesystem_t(filesystem_t* parent_, std::string name_, int size_, type_t type)
      : btree{parent_, name_, (type == file)}, size{size_} {}

  int size = 0;
};

template <int max_size>
int sum_folder_sizes(const filesystem_t* node) {
  int current_size = node->get_size();
  int sum =
      ((node->get_type() == filesystem_t::folder) && (current_size <= max_size))
          ? current_size
          : 0;
  for (const auto& child : *node) {
    sum += sum_folder_sizes<max_size>(child.get());
  }
  return sum;
}

using folder_sizes_t = std::vector<std::pair<std::string_view, int>>;

template <int max_used_space>
void get_folder_sizes(filesystem_t* node, const int used_space,
                      folder_sizes_t& folder_sizes) {
  for (const auto& child : *node) {
    if (child->get_type() != filesystem_t::folder) {
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
  filesystem_t filesystem;
  filesystem_t* current_node = &filesystem;

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

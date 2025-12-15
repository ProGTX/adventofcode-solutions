// https://adventofcode.com/2022/day/7

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

using Folder = aoc::static_vector<usize, 11>;
using Filesystem = std::pair<Vec<Folder>, Vec<u32>>;
constexpr let FILE_ID_START = usize{1} << 16;

namespace stdv = std::views;
namespace stdr = std::ranges;

auto parse(String const& filename) -> Filesystem {
  auto folder_ids = aoc::name_to_id{};
  auto file_ids = aoc::name_to_id{FILE_ID_START};
  let get_root_dir = [] {
    auto path = aoc::static_vector<String, 11>{};
    path.emplace_back("/");
    return path;
  };
  auto current_dir = get_root_dir();
  auto folders = Vec<Folder>{};
  folders.emplace_back();
  auto file_sizes = Vec<u32>{};
  auto current_dir_id = folder_ids.intern(current_dir[0]);

  for (str line : aoc::views::read_lines(filename)) {
    if (line.starts_with("$ ")) {
      let command = line.substr(2);
      if (command.starts_with("cd ")) {
        let dir = command.substr(3);
        if (dir == "/") {
          current_dir = get_root_dir();
        } else if (dir == "..") {
          current_dir.pop_back();
        } else {
          current_dir.emplace_back(dir);
        }
        current_dir_id = folder_ids.intern(aoc::ranges::join(current_dir, '/'));
      } else {
        // ls, handled on following lines
      }
    } else {
      folders.resize(folder_ids.new_size(folders.size()));
      let[first, name] = aoc::split_once<String>(line, " ");
      if (first == "dir") {
        auto subfolder = current_dir;
        subfolder.push_back(std::move(name));
        folders[current_dir_id].push_back(
            folder_ids.intern(aoc::ranges::join(subfolder, '/')));
      } else {
        auto filename = current_dir;
        filename.push_back(std::move(name));
        folders[current_dir_id].push_back(
            file_ids.intern(aoc::ranges::join(filename, '/')));

        let size = aoc::to_number<u32>(first);
        file_sizes.push_back(size);
      }
    }
  }

  return {std::move(folders), std::move(file_sizes)};
}

fn get_folder_size(Vec<Folder> const& folders, Vec<u32> const& file_sizes,
                   usize folder_id) -> u32 {
  return aoc::ranges::accumulate(
      folders[folder_id] | stdv::transform([&](usize id) {
        if (id >= FILE_ID_START) {
          return file_sizes[id - FILE_ID_START];
        } else {
          return get_folder_size(folders, file_sizes, id);
        }
      }),
      u32{});
}

fn solve_case1(Filesystem const& filesystem) -> u32 {
  let & [ folders, file_sizes ] = filesystem;
  return aoc::ranges::accumulate(
      Range{0uz, folders.size()} | //
          stdv::transform([&](usize folder_id) {
            return get_folder_size(folders, file_sizes, folder_id);
          }) |
          stdv::filter([](u32 folder_size) { return folder_size <= 100000; }),
      u32{});
}

fn solve_case2(Filesystem const& filesystem) -> u32 {
  let & [ folders, file_sizes ] = filesystem;
  let used_space = get_folder_size(folders, file_sizes, 0);
  let free_space = 70000000 - used_space;
  let min_delete_space = 30000000 - free_space;
  return stdr::min( //
      Range{1uz, folders.size()} |
      stdv::transform([&](usize folder_id) {
        return get_folder_size(folders, file_sizes, folder_id);
      }) |
      stdv::filter(
          [&](u32 folder_size) { return folder_size >= min_delete_space; }));
}

int main() {
  std::println("Part 1");
  let example = parse("day07.example");
  AOC_EXPECT_RESULT(95437, solve_case1(example));
  let input = parse("day07.input");
  AOC_EXPECT_RESULT(1453349, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(24933642, solve_case2(example));
  AOC_EXPECT_RESULT(2948823, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}

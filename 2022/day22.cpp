// https://adventofcode.com/2022/day/22

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>
#include <span>

using parsed_map_t = aoc::char_grid<>;
constexpr let empty_char = ' ';
constexpr let tile_char = '.';
constexpr let wall_char = '#';

// Encode each position
// Edges of the box/cube specify the index of the connected tile
// The index is encoded as a linear index into the grid,
// which is at most 152x202 in size
// However each empty space on the edge needs to distinguish direction as well,
// so it needs to store two indexes
// The inside tiles then need to be larger than any index
using jungle_t = aoc::grid<int>;
constexpr let empty_space = 1000 * 1000;
constexpr let empty_tile = 1000 * 1000 + 1;
constexpr let wall_tile = 1000 * 1000 + 2;

constexpr let clockwise_char = 'R';
constexpr let counterclockwise_char = 'L';
constexpr let turn_clockwise = -1;
constexpr let turn_counterclockwise = -2;

fn parse_map(std::span<String> raw_map, point map_size) -> parsed_map_t {
  // Add empty space all around the map to remove the need for bounds checking
  map_size += {2, 2};
  parsed_map_t parsed_map{};
  for (String& line : raw_map) {
    if (parsed_map.empty()) {
      parsed_map.add_row(std::views::repeat(empty_char, map_size.x));
    }
    auto empty_suffix =
        std::views::repeat(empty_char, map_size.x - line.size() - 1) |
        aoc::ranges::to<String>();
    parsed_map.add_row(empty_char + std::move(line) + std::move(empty_suffix));
  }
  parsed_map.add_row(std::views::repeat(empty_char, map_size.x));
  return parsed_map;
}

struct input_t {
  parsed_map_t map;
  Vec<int> steps;
};

fn parse(String const& filename) -> input_t {
  auto steps = Vec<int>{};
  auto raw_map = Vec<String>{};
  auto min_max = aoc::min_max_helper{};

  bool parsing_steps = false;
  for (int height = 1; String line : aoc::views::read_lines(
                           filename, aoc::keep_empty{}, aoc::keep_spaces{})) {
    if (line.empty()) {
      parsing_steps = true;
      continue;
    }
    if (!parsing_steps) {
      min_max.update(point(line.size(), height));
      raw_map.push_back(std::move(line));
      ++height;
    } else {
      int start = 0;
      int size = 0;
      let get_step_size = [&]() {
        steps.push_back(aoc::to_number<int>(line.substr(start, size)));
      };
      for (int i = 0; i < (int)line.size(); ++i) {
        let current = line[i];
        if (current == clockwise_char) {
          get_step_size();
          steps.push_back(turn_clockwise);
          start = i + 1;
          size = 0;
        } else if (current == counterclockwise_char) {
          get_step_size();
          steps.push_back(turn_counterclockwise);
          start = i + 1;
          size = 0;
        } else {
          ++size;
        }
      }
      get_step_size();
    }
  }

  return {parse_map(raw_map, min_max.max_value), std::move(steps)};
}

fn get_jungle(parsed_map_t const& parsed_map) -> jungle_t {
  let num_rows = parsed_map.num_rows();
  let num_columns = parsed_map.num_columns();
  jungle_t jungle{empty_space, num_rows, num_columns};
  let size = jungle.size();

  let get_pos = [](int primary_index, int nested_index, bool is_row) {
    point pos{primary_index, nested_index};
    if (is_row) {
      std::swap(pos.x, pos.y);
    }
    return pos;
  };

  let set_edge = [&](const point edge_pos, const int primary_index,
                     const int nested_index, const bool is_row) {
    let jump_pos = get_pos(primary_index, nested_index, is_row);
    auto linear_index = jungle.linear_index(jump_pos.y, jump_pos.x);
    if (is_row) {
      linear_index *= size;
    }
    let edge_value = jungle.at(edge_pos.y, edge_pos.x);
    if (edge_value != empty_space) {
      // Edge has already been modified, so we need to add to it
      // This happens in corners
      linear_index += edge_value;
    }
    jungle.modify(linear_index, edge_pos.y, edge_pos.x);
  };

  using line_t = String;
  let add_line = [&](line_t const& line, const int primary_index,
                     const bool is_row) {
    // Boundaries hold indexes to non-empty tiles
    std::pair<int, int> empty_bounds{-1, -1};
    auto it = std::ranges::begin(line);
    for (int nested_index = 0; nested_index < line.size();
         ++nested_index, ++it) {
      let pos = get_pos(primary_index, nested_index, is_row);
      let current = *it;
      if (current != empty_char) {
        if (empty_bounds.first == -1) {
          // First non-empty tile is a boundary
          empty_bounds.first = nested_index;
          // This will be processed when we get to the end of non-empty tiles
        }
      } else if (empty_bounds.first > 0) {
        if (empty_bounds.second == -1) {
          // First empty tile after non-empty tiles is a boundary
          empty_bounds.second = nested_index - 1;

          // Process lower edge (we're currently here)
          // Inner tile jumps to the upper edge
          set_edge(pos, primary_index, empty_bounds.first, is_row);

          // Process upper edge
          // Upper edge jumps to the lower edge
          // Minus 1 because we're modifying empty space before inner tiles
          let upper_edge_pos =
              get_pos(primary_index, empty_bounds.first - 1, is_row);
          set_edge(upper_edge_pos, primary_index, empty_bounds.second, is_row);
        } else {
          // Just empty space
        }
      } else {
        // Just empty space
      }
      if (current == tile_char) {
        jungle.modify(empty_tile, pos.y, pos.x);
      } else if (current == wall_char) {
        jungle.modify(wall_tile, pos.y, pos.x);
      }
    }
  };

  // Iterate over rows and columns
  // Skip the empty edges of the map
  for (int index : std::views::iota(1, static_cast<int>(num_rows - 1))) {
    add_line(parsed_map.row_view<line_t>(index), index, true);
  }
  for (int index : std::views::iota(1, static_cast<int>(num_columns - 1))) {
    add_line(parsed_map.column_view<line_t>(index), index, false);
  }

  return jungle;
}

using arrow_t = aoc::arrow_type<int>;

fn walk_through(jungle_t const& jungle, std::span<const int> steps) -> arrow_t {
  let size = jungle.size();
  auto current = [&]() {
    // Starting position is the left-most empty tile on the top (non-empty) row
    auto it = std::ranges::find(jungle.row_view(1), empty_tile);
    return arrow_t{point(std::ranges::distance(jungle.begin_row(1), it), 1),
                   aoc::east};
  }();
  for (int step : steps) {
    if (step == turn_clockwise) {
      current.direction = aoc::clockwise_basic(current.direction);
    } else if (step == turn_counterclockwise) {
      current.direction = aoc::anticlockwise_basic(current.direction);
    }
    for (int i = 0; i < step; ++i) {
      auto next_pos = current.position + aoc::get_diff(current.direction);
      auto next_value = jungle.at(next_pos.y, next_pos.x);
      if (next_value == wall_tile) {
        break;
      } else if (next_value == empty_tile) {
        current.position = next_pos;
      } else {
        AOC_ASSERT(next_value != empty_space, "Should never reach empty space");
        // next_value acts as a linear index in this case
        auto index = next_value;
        let dir = current.direction;
        if ((dir == aoc::east) || (dir == aoc::west)) {
          index = index / size;
        } else {
          index = index % size;
        }

        next_pos = jungle.position(index);
        next_value = jungle.at(next_pos.y, next_pos.x);
        if (next_value == wall_tile) {
          break;
        } else {
          AOC_ASSERT(next_value == empty_tile,
                     "Can only jump into inner tiles");
        }
      }
      current.position = next_pos;
    }
  }
  return current;
}

template <int cube_side>
fn solve_case1(input_t const& input) -> int {
  let[pos, facing] = walk_through(get_jungle(input.map), input.steps);
  return 1000 * pos.y + 4 * pos.x + facing;
}

template <int cube_side>
fn solve_case2(input_t const& input) -> int {
  let& parsed_map = input.map;
  let num_rows = (int)parsed_map.num_rows();
  let num_columns = (int)parsed_map.num_columns();

  struct vec3 {
    int x, y, z;
    constexpr vec3 operator-() const { return {-x, -y, -z}; }
    constexpr bool operator==(const vec3&) const = default;
  };
  struct face_info {
    point pos;
    vec3 normal, right, down;
  };

  // BFS from the first face, assigning 3D orientations to all 6 cube faces
  // Crossing a 2D edge transforms the orientation:
  //   east:  { fi.right,  -fi.normal,  fi.down    }
  //   west:  { -fi.right,  fi.normal,  fi.down    }
  //   south: { fi.down,    fi.right,  -fi.normal  }
  //   north: { -fi.down,   fi.right,   fi.normal  }
  let start_face = [&]() -> point {
    for (int fy = 1; fy < num_rows - 1; fy += cube_side)
      for (int fx = 1; fx < num_columns - 1; fx += cube_side)
        if (parsed_map.at(fy, fx) != empty_char)
          return point{fx, fy};
    return point{-1, -1};
  }();

  Vec<face_info> faces;
  {
    Vec<face_info> bfs;
    bfs.push_back({start_face, {0, 0, 1}, {1, 0, 0}, {0, 1, 0}});
    for (int qi = 0; qi < (int)bfs.size(); ++qi) {
      auto fi = bfs[qi];
      if (std::ranges::any_of(faces, [&](let& f) { return f.pos == fi.pos; }))
        continue;
      faces.push_back(fi);
      for (auto dir : aoc::basic_sky_directions) {
        auto d = aoc::get_diff(dir);
        point np{fi.pos.x + d.x * cube_side, fi.pos.y + d.y * cube_side};
        if (np.x < 1 ||
            np.y < 1 ||
            np.x >= num_columns - 1 ||
            np.y >= num_rows - 1)
          continue;
        if (parsed_map.at(np.y, np.x) == empty_char)
          continue;
        if (std::ranges::any_of(bfs, [&](let& f) { return f.pos == np; }))
          continue;
        switch (dir) {
          case aoc::east:
            bfs.push_back({np, fi.right, -fi.normal, fi.down});
            break;
          case aoc::west:
            bfs.push_back({np, -fi.right, fi.normal, fi.down});
            break;
          case aoc::south:
            bfs.push_back({np, fi.down, fi.right, -fi.normal});
            break;
          case aoc::north:
            bfs.push_back({np, -fi.down, fi.right, fi.normal});
            break;
          default:
            break;
        }
      }
    }
  }

  let get_face = [&](point pos) -> const face_info& {
    return *std::ranges::find_if(faces, [&](let& f) {
      return pos.x >= f.pos.x &&
             pos.x < f.pos.x + cube_side &&
             pos.y >= f.pos.y &&
             pos.y < f.pos.y + cube_side;
    });
  };
  let find_face = [&](vec3 normal) -> const face_info& {
    return *std::ranges::find_if(faces,
                                 [&](let& f) { return f.normal == normal; });
  };

  // Given an arrow at a face boundary stepping into empty space,
  // compute the cube-wrapped destination arrow
  let cube_jump = [&](arrow_t arr) -> arrow_t {
    let& fa = get_face(arr.position);

    vec3 target_normal;
    switch (arr.direction) {
      case aoc::east:
        target_normal = fa.right;
        break;
      case aoc::west:
        target_normal = -fa.right;
        break;
      case aoc::south:
        target_normal = fa.down;
        break;
      default:
        target_normal = -fa.down;
        break;
    }
    let& fb = find_face(target_normal);

    // After folding across the edge,
    // -fa.normal is the movement direction on the destination face
    vec3 move_dir = -fa.normal;
    aoc::facing_t new_facing;
    int fixed_coord;
    bool fixed_is_y;
    vec3 free_dir_b;
    if (move_dir == fb.right) {
      new_facing = aoc::east;
      fixed_coord = 0;
      fixed_is_y = false;
      free_dir_b = fb.down;
    } else if (move_dir == -fb.right) {
      new_facing = aoc::west;
      fixed_coord = cube_side - 1;
      fixed_is_y = false;
      free_dir_b = fb.down;
    } else if (move_dir == fb.down) {
      new_facing = aoc::south;
      fixed_coord = 0;
      fixed_is_y = true;
      free_dir_b = fb.right;
    } else {
      new_facing = aoc::north;
      fixed_coord = cube_side - 1;
      fixed_is_y = true;
      free_dir_b = fb.right;
    }

    // The free coordinate runs along the edge:
    //   east/west edge: runs in fa.down direction (varying y within face)
    //   north/south edge: runs in fa.right direction (varying x within face)
    bool is_ew = (arr.direction == aoc::east || arr.direction == aoc::west);
    int free_a =
        is_ew ? (arr.position.y - fa.pos.y) : (arr.position.x - fa.pos.x);
    vec3 free_dir_a = is_ew ? fa.down : fa.right;
    int free_b =
        (free_dir_a == -free_dir_b) ? (cube_side - 1 - free_a) : free_a;

    int x_in_b = fixed_is_y ? free_b : fixed_coord;
    int y_in_b = fixed_is_y ? fixed_coord : free_b;
    return {fb.pos + point(x_in_b, y_in_b), new_facing};
  };

  // Starting position: leftmost tile in row 1
  auto current = [&]() -> arrow_t {
    for (int x = 1; x < num_columns; ++x)
      if (parsed_map.at(1, x) == tile_char)
        return {point{x, 1}, aoc::east};
    return {{}, aoc::east};
  }();

  for (int step : input.steps) {
    if (step == turn_clockwise) {
      current.direction = aoc::clockwise_basic(current.direction);
    } else if (step == turn_counterclockwise) {
      current.direction = aoc::anticlockwise_basic(current.direction);
    }
    for (int i = 0; i < step; ++i) {
      auto next_pos = current.position + aoc::get_diff(current.direction);
      auto next_dir = current.direction;
      if (parsed_map.at(next_pos.y, next_pos.x) == empty_char) {
        auto [jp, jd] = cube_jump(current);
        next_pos = jp;
        next_dir = jd;
      }
      if (parsed_map.at(next_pos.y, next_pos.x) == wall_char)
        break;
      current.position = next_pos;
      current.direction = next_dir;
    }
  }

  return 1000 * current.position.y + 4 * current.position.x + current.direction;
}

int main() {
  std::println("Part 1");
  let example = parse("day22.example");
  AOC_EXPECT_RESULT(6032, (solve_case1<4>(example)));
  let input = parse("day22.input");
  AOC_EXPECT_RESULT(97356, (solve_case1<50>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(5031, (solve_case2<4>(example)));
  AOC_EXPECT_RESULT(120175, (solve_case2<50>(input)));

  AOC_RETURN_CHECK_RESULT();
}

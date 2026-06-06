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

constexpr let clockwise_char = 'R';
constexpr let counterclockwise_char = 'L';
constexpr let turn_clockwise = -1;
constexpr let turn_counterclockwise = -2;

using arrow_t = aoc::arrow_type<int>;

fn parse_map(std::span<String> raw_map, point map_size) -> parsed_map_t {
  // Add empty space all around the map to remove the need for bounds checking
  map_size += {2, 2};
  parsed_map_t parsed_map{};
  parsed_map.add_row(stdv::repeat(empty_char, map_size.x));
  for (String& line : raw_map) {
    auto empty_suffix = stdv::repeat(empty_char, map_size.x - line.size() - 1) |
                        aoc::ranges::to<String>();
    parsed_map.add_row(empty_char + std::move(line) + std::move(empty_suffix));
  }
  parsed_map.add_row(stdv::repeat(empty_char, map_size.x));
  return parsed_map;
}

struct input_t {
  parsed_map_t map;
  Vec<int> steps;
};

fn parse(String const& filename) -> input_t {
  auto steps = Vec<int>{};
  auto raw_map = Vec<String>{};

  auto parsing_steps = false;
  for (String line : aoc::views::read_lines(filename, aoc::keep_empty{},
                                            aoc::keep_spaces{})) {
    if (line.empty()) {
      parsing_steps = true;
      continue;
    }
    if (!parsing_steps) {
      raw_map.push_back(std::move(line));
    } else {
      auto start = 0;
      auto size = 0;
      let get_step_size = [&]() {
        steps.push_back(aoc::to_number<int>(line.substr(start, size)));
      };
      for (int i = 0; i < std::ssize(line); ++i) {
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

  let max_width =
      static_cast<int>(stdr::max_element(raw_map, {}, &String::size)->size());
  let map_size = point{max_width, static_cast<int>(raw_map.size())};
  return {parse_map(raw_map, map_size), std::move(steps)};
}

fn find_start(parsed_map_t const& parsed_map) -> arrow_t {
  let num_columns = static_cast<int>(parsed_map.num_columns());
  for (int x = 1; x < num_columns; ++x) {
    if (parsed_map.at(1, x) == tile_char) {
      return {point{x, 1}, aoc::east};
    }
  }
  return {{}, aoc::east};
}

/// Executes the step sequence:
///   - turns rotate the arrow in place
///   - move counts call step_fn(current) once per tile
///
/// step_fn returns false to stop early (hit a wall), true to continue.
template <class StepFn>
  requires requires(StepFn step_fn, arrow_t& arrow) {
    { step_fn(arrow) } -> std::convertible_to<bool>;
  }
fn execute_steps(std::span<const int> steps, arrow_t start, StepFn step_fn)
    -> arrow_t {
  auto current = start;
  for (int step : steps) {
    if (step == turn_clockwise) {
      current.direction = aoc::clockwise_basic(current.direction);
    } else if (step == turn_counterclockwise) {
      current.direction = aoc::anticlockwise_basic(current.direction);
    }
    for (int i = 0; i < step; ++i) {
      if (!step_fn(current)) {
        break;
      }
    }
  }
  return current;
}

fn password(arrow_t const& arr) -> int {
  return 1000 * arr.position.y + 4 * arr.position.x + arr.direction;
}

fn solve_case1(input_t const& input) -> int {
  let & [ parsed_map, steps ] = input;
  let start = find_start(parsed_map);

  let final_arrow = execute_steps(steps, start, [&](arrow_t& current) {
    let diff = aoc::get_diff(current.direction);
    auto next_pos = current.position + diff;
    if (parsed_map.at(next_pos.y, next_pos.x) == empty_char) {
      // Flat wrap: step back from current position to find the far edge
      auto wrapped = current.position;
      while (parsed_map.at((wrapped - diff).y, (wrapped - diff).x) !=
             empty_char) {
        wrapped = wrapped - diff;
      }
      next_pos = wrapped;
    }
    if (parsed_map.at(next_pos.y, next_pos.x) == wall_char) {
      return false;
    }
    current.position = next_pos;
    return true;
  });

  return password(final_arrow);
}

using vec3 = aoc::nd_point_type<int, 3>;
struct face_info {
  point pos;
  vec3 normal;
  vec3 right;
  vec3 down;
  constexpr bool operator==(const face_info&) const = default;
  constexpr auto operator<=>(const face_info&) const = default;
};

template <int cube_side>
fn solve_case2(input_t const& input) -> int {
  let & [ parsed_map, steps ] = input;
  let num_rows = static_cast<int>(parsed_map.num_rows());
  let num_columns = static_cast<int>(parsed_map.num_columns());

  // BFS from the first face, assigning 3D orientations to all 6 cube faces
  let start_face = [&]() -> point {
    for (int fy = 1; fy < num_rows - 1; fy += cube_side) {
      for (int fx = 1; fx < num_columns - 1; fx += cube_side) {
        if (parsed_map.at(fy, fx) != empty_char) {
          return point{fx, fy};
        }
      }
    }
    return point{-1, -1};
  }();

  // Crossing a 2D edge transforms the orientation:
  //   east:  { face.right,  -face.normal,  face.down    }
  //   west:  { -face.right,  face.normal,  face.down    }
  //   south: { face.down,    face.right,  -face.normal  }
  //   north: { -face.down,   face.right,   face.normal  }
  let faces_map = aoc::shortest_distances_dijkstra(
      face_info{
          .pos = start_face,
          .normal = {0, 0, 1},
          .right = {1, 0, 0},
          .down = {0, 1, 0},
      },
      [&](face_info const& face) {
        // face == current face
        auto neighbors = Vec<face_info>{};
        for (let dir : aoc::basic_sky_directions) {
          let diff = aoc::get_diff(dir);
          let new_pos = point{(diff.x * cube_side) + face.pos.x,
                              (diff.y * cube_side) + face.pos.y};
          if ((new_pos.x < 1) ||
              (new_pos.y < 1) ||
              (new_pos.x >= (num_columns - 1)) ||
              (new_pos.y >= (num_rows - 1))) {
            continue;
          }
          if (parsed_map.at(new_pos.y, new_pos.x) == empty_char) {
            continue;
          }
          neighbors.push_back([&] -> face_info {
            switch (dir) {
              case aoc::east:
                return {new_pos, face.right, -face.normal, face.down};
              case aoc::west:
                return {new_pos, -face.right, face.normal, face.down};
              case aoc::south:
                return {new_pos, face.down, face.right, -face.normal};
              case aoc::north:
                return {new_pos, -face.down, face.right, face.normal};
              default:
                AOC_UNREACHABLE("Invalid direction");
            }
          }());
        }
        return aoc::dijkstra_uniform_neighbors(std::move(neighbors));
      });
  auto faces = Vec<face_info>{};
  for (let& [ face, _ ] : faces_map) {
    faces.push_back(face);
  }

  // Find face by 2D position
  let face_from_pos = [&](point pos) -> const face_info& {
    return *stdr::find_if(faces, [&](let& f) {
      return (pos.x >= f.pos.x) &&
             (pos.x < (f.pos.x + cube_side)) &&
             (pos.y >= f.pos.y) &&
             (pos.y < (f.pos.y + cube_side));
    });
  };
  // Find face by 3D normal
  let face_from_normal = [&](vec3 normal) -> const face_info& {
    return *stdr::find_if(faces, [&](let& f) { return f.normal == normal; });
  };

  // Given an arrow stepping off a face edge into empty space,
  // compute the cube-wrapped destination arrow.
  // Works purely from the 3D orientations (normal/right/down)
  // assigned to each face during the BFS above.
  let cube_jump = [&](const arrow_t arrow) -> arrow_t {
    let& src = face_from_pos(arrow.position);

    // Moving east/west/south/north on the 2D map corresponds to moving in the
    // src.right / -src.right / src.down / -src.down directions in 3D.
    // The destination face is the one whose normal points that way.
    let dest_normal = [&] -> vec3 {
      switch (arrow.direction) {
        case aoc::east:
          return src.right;
        case aoc::west:
          return -src.right;
        case aoc::south:
          return src.down;
        default:
          return -src.down;
      }
    }();
    let& dest = face_from_normal(dest_normal);

    // Folding across the shared edge, the src face's outward normal becomes
    // the in-plane arrival direction on the destination face (reversed)
    let arrival_dir = -src.normal;
    auto new_facing = aoc::facing_t{};
    auto entry_coord = 0; // value of the fixed axis on dest (0 or cube_side-1)
    auto y_is_fixed = false;  // true when entering from north/south (y fixed)
    auto dest_along = vec3{}; // dest axis running parallel to the entry edge
    if (arrival_dir == dest.right) {
      new_facing = aoc::east;
      entry_coord = 0;
      y_is_fixed = false;
      dest_along = dest.down;
    } else if (arrival_dir == -dest.right) {
      new_facing = aoc::west;
      entry_coord = cube_side - 1;
      y_is_fixed = false;
      dest_along = dest.down;
    } else if (arrival_dir == dest.down) {
      new_facing = aoc::south;
      entry_coord = 0;
      y_is_fixed = true;
      dest_along = dest.right;
    } else {
      new_facing = aoc::north;
      entry_coord = cube_side - 1;
      y_is_fixed = true;
      dest_along = dest.right;
    }

    // Measure how far along the src entry edge the arrow sits,
    // then map it onto the destination edge.
    // Reverse when the two edge axes are antiparallel.
    let is_EW =
        (arrow.direction == aoc::east) || (arrow.direction == aoc::west);
    let src_along = is_EW ? src.down : src.right;
    let src_offset =
        is_EW ? (arrow.position.y - src.pos.y) : (arrow.position.x - src.pos.x);
    let dest_offset =
        (src_along == -dest_along) ? (cube_side - 1 - src_offset) : src_offset;

    let x_in_dst = y_is_fixed ? dest_offset : entry_coord;
    let y_in_dst = y_is_fixed ? entry_coord : dest_offset;
    return {dest.pos + point(x_in_dst, y_in_dst), new_facing};
  };

  let start = find_start(parsed_map);

  let final_arrow = execute_steps(steps, start, [&](arrow_t& current) {
    auto next_pos = current.position + aoc::get_diff(current.direction);
    auto next_dir = current.direction;
    if (parsed_map.at(next_pos.y, next_pos.x) == empty_char) {
      let next_arrow = cube_jump(current);
      next_pos = next_arrow.position;
      next_dir = next_arrow.direction;
    }
    if (parsed_map.at(next_pos.y, next_pos.x) == wall_char) {
      return false;
    }
    current.position = next_pos;
    current.direction = next_dir;
    return true;
  });

  return password(final_arrow);
}

int main() {
  std::println("Part 1");
  let example = parse("day22.example");
  AOC_EXPECT_RESULT(6032, solve_case1(example));
  let input = parse("day22.input");
  AOC_EXPECT_RESULT(97356, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(5031, (solve_case2<4>(example)));
  AOC_EXPECT_RESULT(120175, (solve_case2<50>(input)));

  AOC_RETURN_CHECK_RESULT();
}

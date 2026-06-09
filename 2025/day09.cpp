// https://adventofcode.com/2025/day/9

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>

using Point = aoc::point_type<i64>;
using Segment = aoc::closed_range<Point>;
using Polygon = Vec<Point>;
using Rectangle = std::array<Point, 4>;

auto parse(String const& filename) -> Polygon {
  return aoc::views::read_lines(filename) |
         stdv::transform(
             [](str line) { return aoc::split<Point>(line, ','); }) |
         aoc::ranges::to<Polygon>();
}

fn area(Point const& p1, Point const& p2) -> u64 {
  let diff = (p2 - p1).abs() + Point{1, 1};
  return static_cast<u64>(diff.x) * static_cast<u64>(diff.y);
}

fn solve_case1(Polygon const& polygon) -> u64 {
  return stdr::max( //
      stdv::cartesian_product(polygon, polygon) |
      stdv::filter([](auto&& pair) {
        let & [ p1, p2 ] = pair;
        return p1 < p2;
      }) |
      stdv::transform([](auto&& pair) {
        let & [ p1, p2 ] = pair;
        return area(p1, p2);
      }));
}

fn point_on_segment(Point const& point, Segment const& segment) -> bool {
  return aoc::orientation(segment.begin, segment.end, point) == 0 &&
         segment.contains(point);
}

fn point_inside_polygon(Polygon const& polygon, Point const& point) -> bool {
  let vertex_count = polygon.size();
  auto inside = false;
  auto prev_idx = vertex_count - 1;
  for (auto curr_idx : Range{0uz, vertex_count}) {
    let& curr_vertex = polygon[curr_idx];
    let& prev_vertex = polygon[prev_idx];
    if (point_on_segment(point, Segment{curr_vertex, prev_vertex})) {
      // The boundary is inside
      return true;
    }
    // Handle vertices on the ray without double-counting
    if ((curr_vertex.y > point.y) != (prev_vertex.y > point.y)) {
      let delta_y = prev_vertex.y - curr_vertex.y;
      let lhs = (point.x - curr_vertex.x) * delta_y;
      let rhs = (prev_vertex.x - curr_vertex.x) * (point.y - curr_vertex.y);
      // Avoid integer division by rearranging the crossing comparison
      if ((lhs < rhs) == (delta_y > 0)) {
        inside = !inside;
      }
    }
    prev_idx = curr_idx;
  }
  return inside;
}

fn corners_inside(Polygon const& polygon, Rectangle const& rectangle) -> bool {
  return stdr::all_of(rectangle, [&](Point const& corner) {
    return point_inside_polygon(polygon, corner);
  });
}

fn segments_cross(Segment const& seg1, Segment const& seg2) -> bool {
  let o1 = aoc::orientation(seg2.begin, seg2.end, seg1.begin);
  let o2 = aoc::orientation(seg2.begin, seg2.end, seg1.end);
  // Collinear: shared boundary segment, not a proper intersection
  if (o1 == 0 && o2 == 0) {
    return false;
  }
  let o3 = aoc::orientation(seg1.begin, seg1.end, seg2.begin);
  let o4 = aoc::orientation(seg1.begin, seg1.end, seg2.end);
  // Proper crossing: each segment strictly straddles the other's line
  return (o1 * o2 < 0) && (o3 * o4 < 0);
}

fn edges_intersect(Polygon const& polygon, Rectangle const& rectangle) -> bool {
  // rectangle indices:
  //   0=(xmin,ymin),
  //   1=(xmin,ymax),
  //   2=(xmax,ymin),
  //   3=(xmax,ymax)
  let rect_edges = std::array{
      Segment{rectangle[0], rectangle[1]}, // left
      Segment{rectangle[2], rectangle[3]}, // right
      Segment{rectangle[0], rectangle[2]}, // bottom
      Segment{rectangle[1], rectangle[3]}, // top
  };
  let edge_count = polygon.size();
  auto prev_idx = edge_count - 1;
  for (auto curr_idx : Range{0uz, edge_count}) {
    let poly_segment = Segment{polygon[curr_idx], polygon[prev_idx]};
    for (let& rect_segment : rect_edges) {
      if (segments_cross(poly_segment, rect_segment)) {
        return true;
      }
    }
    prev_idx = curr_idx;
  }
  return false;
}

fn solve_case2(Polygon const& polygon) -> u64 {
  auto pairs = stdv::cartesian_product(polygon, polygon) |
               stdv::filter([](auto&& pair) {
                 let & [ p1, p2 ] = pair;
                 return p1 < p2;
               }) |
               aoc::collect_vec<std::pair<Point, Point>>();
  stdr::sort(pairs, [](auto&& a, auto&& b) {
    let & [ a1, a2 ] = a;
    let & [ b1, b2 ] = b;
    return area(a1, a2) > area(b1, b2);
  });
  for (let& [ p1, p2 ] : pairs) {
    let xmin = std::min(p1.x, p2.x);
    let xmax = std::max(p1.x, p2.x);
    let ymin = std::min(p1.y, p2.y);
    let ymax = std::max(p1.y, p2.y);
    let corners = Rectangle{
        Point{xmin, ymin},
        Point{xmin, ymax},
        Point{xmax, ymin},
        Point{xmax, ymax},
    };
    if (corners_inside(polygon, corners) &&
        !edges_intersect(polygon, corners)) {
      return area(p1, p2);
    }
  }
  return 0;
}

int main() {
  std::println("Part 1");
  let example = parse("day09.example");
  AOC_EXPECT_RESULT(50, solve_case1(example));
  let input = parse("day09.input");
  AOC_EXPECT_RESULT(4744899849, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(24, solve_case2(example));
  AOC_EXPECT_RESULT(1540192500, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}

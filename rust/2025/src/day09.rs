use std::cmp::Ordering;

use itertools::Itertools;

type Point = aoc::point::Point<i64>;
type Segment = (Point, Point);
type Polygon = Vec<Point>;
type Rectangle = [Point; 4];

fn parse(filename: &str) -> Polygon {
    let parse = |s: &str| s.parse::<i64>().unwrap();
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let coords = line.split(',').collect::<Vec<_>>();
            Point::new(parse(coords[0]), parse(coords[1]))
        })
        .collect()
}

fn area(p1: &Point, p2: &Point) -> u64 {
    let diff = (*p2 - *p1).abs() + Point::new(1, 1);
    (diff.x as u64) * (diff.y as u64)
}

fn solve_case1(polygon: &Polygon) -> u64 {
    polygon
        .iter()
        .cartesian_product(polygon.iter())
        .filter(|(p1, p2)| p1 < p2)
        .map(|(p1, p2)| area(p1, p2))
        .max()
        .unwrap()
}

fn point_on_segment(point: &Point, segment: &Segment) -> bool {
    let (seg_start, seg_end) = segment;
    // Check collinearity via cross product
    // Bounds check coordinates
    seg_start.orientation(seg_end, point) == Ordering::Equal
        && point.x >= seg_start.x.min(seg_end.x)
        && point.x <= seg_start.x.max(seg_end.x)
        && point.y >= seg_start.y.min(seg_end.y)
        && point.y <= seg_start.y.max(seg_end.y)
}

fn point_inside_polygon(polygon: &Polygon, point: &Point) -> bool {
    let vertex_count = polygon.len();
    let mut inside = false;
    let mut prev_idx = vertex_count - 1;
    for curr_idx in 0..vertex_count {
        let (curr_vertex, prev_vertex) = (&polygon[curr_idx], &polygon[prev_idx]);
        if point_on_segment(point, &(*curr_vertex, *prev_vertex)) {
            // The boundary is inside
            return true;
        }
        // Handle vertices on the ray without double-counting
        if (curr_vertex.y > point.y) != (prev_vertex.y > point.y) {
            let delta_y = prev_vertex.y - curr_vertex.y;
            let lhs = (point.x - curr_vertex.x) * delta_y;
            let rhs = (prev_vertex.x - curr_vertex.x) * (point.y - curr_vertex.y);
            // Avoid integer division by rearranging the crossing comparison
            if (lhs < rhs) == (delta_y > 0) {
                inside = !inside;
            }
        }
        prev_idx = curr_idx;
    }
    return inside;
}

/// Run a standard ray casting test for each corner.
/// Treat polygon on an edge as inside.
/// If any corner is outside, return false.
fn corners_inside(polygon: &Polygon, rectangle: &Rectangle) -> bool {
    rectangle
        .iter()
        .all(|corner| point_inside_polygon(polygon, corner))
}

fn segments_cross(seg1: &Segment, seg2: &Segment) -> bool {
    let o1 = seg2.0.orientation(&seg2.1, &seg1.0);
    let o2 = seg2.0.orientation(&seg2.1, &seg1.1);
    // Collinear: shared boundary segment, not a proper intersection
    if o1 == Ordering::Equal && o2 == Ordering::Equal {
        return false;
    }
    let o3 = seg1.0.orientation(&seg1.1, &seg2.0);
    let o4 = seg1.0.orientation(&seg1.1, &seg2.1);
    // Proper crossing: each segment strictly straddles the other's line
    (o1 != o2 && o1 != Ordering::Equal && o2 != Ordering::Equal)
        && (o3 != o4 && o3 != Ordering::Equal && o4 != Ordering::Equal)
}

/// For each polygon edge check whether it properly
/// intersects any rectangle edge.
/// If any intersection exists that is not just a shared boundary segment,
/// returns false.
fn edges_intersect(polygon: &Polygon, rectangle: &Rectangle) -> bool {
    // rectangle indices: 0=(xmin,ymin), 1=(xmin,ymax), 2=(xmax,ymin), 3=(xmax,ymax)
    let rect_edges: [Segment; 4] = [
        (rectangle[0], rectangle[1]), // left
        (rectangle[2], rectangle[3]), // right
        (rectangle[0], rectangle[2]), // bottom
        (rectangle[1], rectangle[3]), // top
    ];
    let edge_count = polygon.len();
    let mut prev_idx = edge_count - 1;
    for curr_idx in 0..edge_count {
        let poly_segment = (polygon[curr_idx], polygon[prev_idx]);
        for rect_segment in &rect_edges {
            if segments_cross(&poly_segment, rect_segment) {
                return true;
            }
        }
        prev_idx = curr_idx;
    }
    return false;
}

fn solve_case2(polygon: &Polygon) -> u64 {
    polygon
        .iter()
        .cartesian_product(polygon.iter())
        .map(|(&p1, &p2)| {
            if (p1 >= p2) {
                return 0;
            }
            let (xmin, xmax) = (p1.x.min(p2.x), p1.x.max(p2.x));
            let (ymin, ymax) = (p1.y.min(p2.y), p1.y.max(p2.y));
            let corners = [
                Point::new(xmin, ymin),
                Point::new(xmin, ymax),
                Point::new(xmax, ymin),
                Point::new(xmax, ymax),
            ];
            if (!corners_inside(polygon, &corners) || edges_intersect(polygon, &corners)) {
                return 0;
            }
            return area(&p1, &p2);
        })
        .max()
        .unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day09.example");
    aoc::expect_result!(50, solve_case1(&example));
    let input = parse("day09.input");
    aoc::expect_result!(4744899849, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(24, solve_case2(&example));
    aoc::expect_result!(1540192500, solve_case2(&input));
}

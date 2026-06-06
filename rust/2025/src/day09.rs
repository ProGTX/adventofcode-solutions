use itertools::Itertools;

type Point = aoc::point::Point<i64>;
type Polygon = Vec<Point>;

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

type Rectangle = [aoc::point::Point<i64>; 4];

fn point_on_segment(point: &Point, seg_start: &Point, seg_end: &Point) -> bool {
    // Check collinearity via cross product
    let cross = (seg_end.x - seg_start.x) * (point.y - seg_start.y)
        - (seg_end.y - seg_start.y) * (point.x - seg_start.x);
    // Bounds check coordinates
    cross == 0
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
        if point_on_segment(point, curr_vertex, prev_vertex) {
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

fn cross2d(from: &Point, to: &Point, point: &Point) -> i64 {
    (to.x - from.x) * (point.y - from.y) - (to.y - from.y) * (point.x - from.x)
}

fn segments_cross(
    seg1_start: &Point,
    seg1_end: &Point,
    seg2_start: &Point,
    seg2_end: &Point,
) -> bool {
    let seg1_start_side = cross2d(seg2_start, seg2_end, seg1_start);
    let seg1_end_side = cross2d(seg2_start, seg2_end, seg1_end);
    let seg2_start_side = cross2d(seg1_start, seg1_end, seg2_start);
    let seg2_end_side = cross2d(seg1_start, seg1_end, seg2_end);
    // Collinear: shared boundary segment, not a proper intersection
    if seg1_start_side == 0 && seg1_end_side == 0 {
        return false;
    }
    // Proper crossing: each segment strictly straddles the other's line
    return (seg1_start_side > 0 && seg1_end_side < 0 || seg1_start_side < 0 && seg1_end_side > 0)
        && (seg2_start_side > 0 && seg2_end_side < 0 || seg2_start_side < 0 && seg2_end_side > 0);
}

/// For each polygon edge check whether it properly
/// intersects any rectangle edge.
/// If any intersection exists that is not just a shared boundary segment,
/// returns false.
fn edges_intersect(polygon: &Polygon, rectangle: &Rectangle) -> bool {
    // rectangle indices: 0=(xmin,ymin), 1=(xmin,ymax), 2=(xmax,ymin), 3=(xmax,ymax)
    let rect_edges: [(&Point, &Point); 4] = [
        (&rectangle[0], &rectangle[1]), // left
        (&rectangle[2], &rectangle[3]), // right
        (&rectangle[0], &rectangle[2]), // bottom
        (&rectangle[1], &rectangle[3]), // top
    ];
    let edge_count = polygon.len();
    let mut prev_idx = edge_count - 1;
    for curr_idx in 0..edge_count {
        let (poly_start, poly_end) = (&polygon[curr_idx], &polygon[prev_idx]);
        for &(rect_start, rect_end) in &rect_edges {
            if segments_cross(poly_start, poly_end, rect_start, rect_end) {
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

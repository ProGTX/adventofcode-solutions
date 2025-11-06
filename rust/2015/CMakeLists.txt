set(year 2015)
set(rust_test_names "")

add_rust_test(day01 ${year})
add_rust_test(day02 ${year})
add_rust_test(day03 ${year})
# Not computing MD5 hashes for day04
add_rust_test(day05 ${year})
add_rust_test(day06 ${year})
add_rust_test(day07 ${year})
add_rust_test(day08 ${year})
add_rust_test(day09 ${year})
add_rust_test(day10 ${year})
add_rust_test(day11 ${year})
add_rust_test(day12 ${year})

generate_cargo_toml(${year} "${rust_test_names}")

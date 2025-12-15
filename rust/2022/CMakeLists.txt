set(year 2022)
set(rust_local_sources "")

add_rust_test(day01 ${year})
add_rust_test(day07 ${year})
add_rust_test(day08 ${year})
add_rust_test(day11 ${year})
add_rust_test(day12 ${year})
add_rust_test(day13 ${year})
add_rust_test(day20 ${year})

generate_cargo_toml(${year} "${rust_local_sources}")

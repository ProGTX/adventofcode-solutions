set(year 2021)
set(rust_local_sources "")

add_rust_test(day01 ${year})
add_rust_test(day02 ${year})
add_rust_test(day03 ${year})
add_rust_test(day04 ${year})

generate_cargo_toml(${year} "${rust_local_sources}")

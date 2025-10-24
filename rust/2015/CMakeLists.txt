set(year 2015)
set(rust_test_names "")

add_rust_test(day01 ${year})
add_rust_test(day02 ${year})

generate_cargo_toml(${year} "${rust_test_names}")

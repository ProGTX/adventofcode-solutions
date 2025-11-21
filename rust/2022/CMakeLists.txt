set(year 2022)
set(rust_local_sources "")

add_rust_test(day01 ${year})

generate_cargo_toml(${year} "${rust_local_sources}")

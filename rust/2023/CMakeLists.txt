set(year 2023)
set(rust_local_sources "")

add_rust_test(day01 ${year})

generate_cargo_toml(${year} "${rust_local_sources}")

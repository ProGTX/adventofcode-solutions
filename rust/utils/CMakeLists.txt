# Let CMake know about the utility library
set(rust_local_sources "")

function(add_rust_source filename folder)
    set_property(GLOBAL APPEND PROPERTY RUST_SOURCES
        "${RUST_DIR}/${folder}/src/${filename}.rs"
    )
    # rust_local_sources is needed by generate_utils
    set(local_sources "${rust_local_sources}")
    list(APPEND local_sources "${filename}")
    set(rust_local_sources "${local_sources}" PARENT_SCOPE)
endfunction()

add_rust_source(file utils)
add_rust_source(grid utils)
add_rust_source(math utils)
add_rust_source(point utils)

function(generate_utils)
    set(RUST_UTILS_SECTION "")
    foreach(src ${rust_local_sources})
        string(APPEND RUST_UTILS_SECTION
"pub mod ${src};
")
    endforeach()
    configure_file(
        "${RUST_DIR}/utils/lib.rs.in"
        "${RUST_DIR}/utils/src/lib.rs"
        @ONLY
    )
endfunction()

generate_utils()

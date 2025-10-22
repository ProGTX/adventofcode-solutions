set(RUST_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(RUST_TARGET_DIR "${RUST_DIR}/target")
set(RUST_RELEASE_DIR "${RUST_TARGET_DIR}/release")

add_custom_command(
    OUTPUT "${RUST_RELEASE_DIR}/.stamp"
    COMMAND cargo build --release
    COMMAND ${CMAKE_COMMAND} -E touch "${RUST_RELEASE_DIR}/.stamp"
    WORKING_DIRECTORY "${RUST_DIR}"
    DEPENDS "${RUST_DIR}/Cargo.toml"
    COMMENT "Building all Rust workspaces"
)

add_custom_target(rust_build ALL
    DEPENDS "${RUST_RELEASE_DIR}/.stamp"
)

function(add_rust_test sample_name year)
    add_test(NAME ${year}_${sample_name}_rust
        COMMAND ${RUST_RELEASE_DIR}/${sample_name}
        DEPENDS rust_build
    )
endfunction()

add_subdirectory(2015)

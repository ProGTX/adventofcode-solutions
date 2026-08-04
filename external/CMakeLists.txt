enable_language(C)

add_library(md5 STATIC md5/md5c.c)
target_include_directories(md5 PUBLIC md5)
# TODO: The code breaks with C23, which is what GCC 15 defaults to
set_property(TARGET md5 PROPERTY C_STANDARD 17)
target_compile_definitions(md5 PRIVATE PROTOTYPES=1)

# https://github.com/martinus/unordered_dense - a submodule pinned to v4.8.1
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/unordered_dense/CMakeLists.txt")
  message(FATAL_ERROR
    "external/unordered_dense is empty. Run:\n"
    "  git submodule update --init --recursive"
  )
endif()

if (AOC_MODULE_SUPPORT)
  # Builds unordered_dense_module alongside the header-only target.
  # CMP0077 is NEW here, so this normal variable wins over the option() inside.
  set(ANKERL_ENABLE_MODULES ON)
endif()

add_subdirectory(unordered_dense EXCLUDE_FROM_ALL)

if (TARGET unordered_dense_module)
  # The module unit imports std when the compiler advertises it,
  # and nothing here provides a std module target to satisfy that.
  target_compile_definitions(unordered_dense_module
    PRIVATE ANKERL_UNORDERED_DENSE_STD_MODULE=0
  )
  set_target_properties(unordered_dense_module PROPERTIES FOLDER external)
endif()

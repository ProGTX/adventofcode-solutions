enable_language(C)

add_library(md5 STATIC md5/md5c.c)
target_include_directories(md5 PUBLIC md5)
# TODO: The code breaks with C23, which is what GCC 15 defaults to
set_property(TARGET md5 PROPERTY C_STANDARD 17)
target_compile_definitions(md5 PRIVATE PROTOTYPES=1)

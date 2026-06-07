enable_language(C)

add_library(md5 STATIC md5/md5c.c)
target_include_directories(md5 PUBLIC md5)

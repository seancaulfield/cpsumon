cmake_minimum_required(VERSION 2.8.11)
project (cpsufancli)

include_directories(include)

add_subdirectory (libcpsumon)

add_executable(cpsufancli cpsufancli.c)
add_executable(cpsumoncli cpsumoncli.c)

target_link_libraries (cpsumoncli cpsumon m)
target_link_libraries (cpsufancli cpsumon m)

install (TARGETS cpsufancli cpsumoncli
	RUNTIME DESTINATION /sbin
	LIBRARY DESTINATION /lib64)

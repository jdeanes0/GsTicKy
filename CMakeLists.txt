cmake_minimum_required(VERSION 3.16)
project(sticky_notes_app C)

set(CMAKE_DEPFILE_FLAGS_C "-MMD -MP")
add_compile_options(-Wall)

# Tell pkg-config to look in the MinGW environment
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

include_directories(
  ${GTK4_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/include
)

add_compile_options(${GTK4_CFLAGS_OTHER})
link_libraries(${GTK4_LIBRARIES} m)

# Windows-only: X11 not needed
if (NOT WIN32)
  link_libraries(X11)
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/src/*.c
  ${CMAKE_SOURCE_DIR}/lib/*.c
)

add_executable(sticky-notes-app ${SOURCES})

target_link_libraries(sticky-notes-app ${GTK4_LIBRARIES})
target_compile_options(sticky-notes-app PRIVATE ${GTK4_CFLAGS_OTHER})
target_include_directories(sticky-notes-app PRIVATE ${GTK4_INCLUDE_DIRS} include)

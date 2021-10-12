cmake_minimum_required(VERSION 3.6)

project(riddle
  VERSION 1.0
  LANGUAGES C
)

include(CTest)

option(USE_SANITIZER "Use Sanitizers?" OFF)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL 1.1 REQUIRED)

add_library(riddle STATIC
  src/riddle.c
)
target_include_directories(riddle PUBLIC
  src
  ${OPENSSL_INCLUDE_DIR}
)
if(UNIX)
  set(CMAKE_C_FLAGS_COVERAGE       "-O0 -pg --coverage")
  if(NOT CMAKE_BUILD_TYPE STREQUAL "Coverage" AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_EXE_LINKER_FLAGS    "-s")
    target_compile_definitions(riddle PUBLIC
      _FORTIFY_SOURCE=2
    )
  endif()
  target_compile_options(riddle PUBLIC
    -Wall
    -Wextra
    -Werror
    -fstack-protector-all
  )
  if(${USE_SANITIZER})
    target_compile_options(riddle PUBLIC
      -fsanitize=address
      -fsanitize=leak
      -fsanitize=undefined
    )
  endif()
elseif(WIN32)
  if (MSVC)
    target_compile_options(riddle PUBLIC /W3)
  endif()
endif()

# Build the tests
set(tests test1 test2 test3 test4)
foreach(test ${tests})
  add_executable(riddle_${test}
    tests/common.c
    tests/${test}.c
  )
  if(${USE_SANITIZER})
    target_link_libraries(riddle_${test} PUBLIC
      asan
      ubsan
    )
  endif()
  target_link_libraries(riddle_${test} PUBLIC
    riddle
    ${OPENSSL_CRYPTO_LIBRARY}
    ${CMAKE_DL_LIBS}
  )
  add_test(NAME test_riddle_${test} COMMAND riddle_${test})
endforeach()

# Install the stuff
install(FILES    src/riddle.h  DESTINATION  include)
install(TARGETS  riddle        DESTINATION  lib)

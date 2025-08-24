cmake_minimum_required(VERSION 3.16)
project(SimpleOrderBook CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------
# Library (headers now live in inc/)
# ----------------------------------------
add_library(order_book
	src/order_book.cpp
)
target_include_directories(order_book PUBLIC inc)

# ----------------------------------------
# Demo executable
# ----------------------------------------
add_executable(order_book_demo
	src/main.cpp
)
target_link_libraries(order_book_demo PRIVATE order_book)
target_include_directories(order_book_demo PRIVATE inc)

# ----------------------------------------
# Tests (system-installed GoogleTest only)
# ----------------------------------------
include(CTest)  # provides BUILD_TESTING option
if(BUILD_TESTING)
	# Try CONFIG package first (modern installs), then CMake's FindGTest module.
	find_package(GTest QUIET CONFIG)
	if(NOT GTest_FOUND)
		find_package(GTest REQUIRED)  # module mode; provides GTest::GTest, GTest::Main
	endif()

	# Normalize target names between CONFIG and module modes
	if(TARGET GTest::gtest)
		set(GTEST_LIB  GTest::gtest)
		set(GTEST_MAIN GTest::gtest_main)
	else()
		set(GTEST_LIB  GTest::GTest)
		set(GTEST_MAIN GTest::Main)
	endif()

	add_executable(order_book_tests
		tests/test_order_book.cpp
	)
	target_link_libraries(order_book_tests PRIVATE order_book ${GTEST_LIB} ${GTEST_MAIN})
	target_include_directories(order_book_tests PRIVATE inc)

	include(GoogleTest)        # from CMake's bundled module
	gtest_discover_tests(order_book_tests)
endif()


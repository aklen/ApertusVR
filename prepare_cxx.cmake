include(CheckCCompilerFlag)

message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")

# on Apple we need to use C++17
if (APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
	add_definitions(-std=c++17)
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

	check_c_compiler_flag("-arch arm64" arm64Supported)
	# message("arm64Supported=${arm64Supported}")
	if(arm64Supported EQUAL 1)
		message(STATUS "Apple M1")
		set(APPLE_M1 TRUE)

		execute_process(
			COMMAND xcrun --sdk macosx --show-sdk-path
			OUTPUT_VARIABLE SDK_PATH
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
			set(CMAKE_OSX_SYSROOT ${SDK_PATH} CACHE STRING "macOS SDK Path")
		endif ()
	endif ()
endif ()

if (UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif ()

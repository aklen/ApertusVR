include(CheckCCompilerFlag)

# on Apple we need to use C++17
if (APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
	add_definitions(-std=c++17)
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

	check_c_compiler_flag("-arch arm64" arm64Supported)
	# message("arm64Supported=${arm64Supported}")
	if(arm64Supported EQUAL 1)
		message("Apple M1")
		set(APPLE_M1 TRUE)
	endif ()
endif ()

if (UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif ()

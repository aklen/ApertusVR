# Find required packages
find_package(PkgConfig REQUIRED)

# Locate GStreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
if(NOT GSTREAMER_FOUND)
    message(FATAL_ERROR "GStreamer 1.0 not found! Please install the required package.")
endif()

# Locate libcurl (optional)
find_package(CURL)
if(CURL_FOUND)
    message(STATUS "libcurl found, enabling USE_LIBCURL")
    add_definitions(-DUSE_LIBCURL)
    set(USE_LIBCURL ON)
else()
    message(WARNING "libcurl not found, building without URL encoding support")
endif()

# Add all header and cpp files in the directory to the project
set (HEADERS
	apeGStreamerPlugin.h
	)

set (SOURCES 
    apeGStreamerPlugin.cpp
	)

include_directories(
    ${APE_SOURCE_DIR}/include
    ${APE_SOURCE_DIR}/core/eventManager
    )

add_library (apeGStreamerPlugin SHARED ${SOURCES} ${HEADERS})

target_include_directories(apeGStreamerPlugin PUBLIC
    ${GSTREAMER_INCLUDE_DIRS}   # GStreamer include directories
)

# Link with dependency libraries
target_link_libraries (apeGStreamerPlugin apePluginManager apeEventManager apeLogManager ${GSTREAMER_LIBRARIES} CURL::libcurl)
set_property (TARGET apeGStreamerPlugin PROPERTY COMPILE_DEFINITIONS APE_SOURCE_DIR="${APE_SOURCE_DIR}")
set_property (TARGET apeGStreamerPlugin PROPERTY FOLDER "Plugins/GStreamer")

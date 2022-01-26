find_package(Curses REQUIRED)

set (HEADERS
	apeCliGuiPlugin.h
)
set (SOURCES 
	apeCliGuiPlugin.cpp
)
include_directories(	
)

add_library (apeCliGuilugin SHARED ${SOURCES} ${HEADERS})

# Link with dependency libraries
target_link_libraries (apeCliGuilugin apePluginManager apeEventManager apeLogManager)
set_property (TARGET apeCliGuilugin PROPERTY COMPILE_DEFINITIONS APE_SOURCE_DIR="${APE_SOURCE_DIR}")
set_property (TARGET apeCliGuilugin PROPERTY FOLDER "Plugins/CliGui")

set (HEADERS
	apeConfigManagerImpl.h
)
set (SOURCES
	apeConfigManagerImpl.cpp
)

include_directories(
	${APE_THIRDPARTY_DIR}/rapidjson/include/
)

add_library (apeConfigManager SHARED ${SOURCES} ${HEADERS})

# Link with dependency libraries
target_link_libraries (apeConfigManager apeLogManager)

# Place target xxx into the specified folder in IDEs supporting it
set_property (TARGET apeConfigManager PROPERTY COMPILE_DEFINITIONS BUILDING_APE_CONFIGMANAGER_DLL)
set_property (TARGET apeConfigManager PROPERTY FOLDER "Core")

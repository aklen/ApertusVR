project(apeDataStreamerPlugin)

set(SOURCE_FILES
    apeDataStreamerPlugin.cpp
)

set(HEADER_FILES
    apeDataStreamerPlugin.h
)

include_directories(
    ${APE_SOURCE_DIR}/include
    ${APE_SOURCE_DIR}/core/eventManager
)

add_library(apeDataStreamerPlugin SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(apeDataStreamerPlugin PRIVATE ${APE_INCLUDE_DIRS})
target_link_libraries(apeDataStreamerPlugin PRIVATE apeLogManager apeEventManager apePluginManager apeSceneManager)

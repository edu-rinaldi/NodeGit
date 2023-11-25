project(nd_blender)

# handle external
add_subdirectory(external)

option(ND_LOG_ENABLED "Enable log on nd_blender executable" ON)
if(ND_LOG_ENABLED)
    add_compile_definitions(ND_LOG_ENABLED)
endif(ND_LOG_ENABLED)


file(GLOB NDBLENDER_APP_FILES CONFIGURE_DEPENDS *.cpp *.h *.hpp)

add_executable(${PROJECT_NAME} ${NDBLENDER_APP_FILES})

# link
target_link_libraries(${PROJECT_NAME} PRIVATE nodediff argparse fmt)

# copy resource files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_CURRENT_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)
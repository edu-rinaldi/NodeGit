project(nodediff)

# parent dir
get_filename_component(PARENT_DIR ../ ABSOLUTE)

file(GLOB_RECURSE ND_LIB_FILES CONFIGURE_DEPENDS *.cpp *.h *.hpp)
message(STATUS "NodeDiff lib files: ${ND_LIB_FILES}")
add_library(${PROJECT_NAME} STATIC ${ND_LIB_FILES})

# expose options
option(ND_PARALLELIZE "Parallelize NodeGit execution where possible" ON)

# handle exposed options
if(ND_PARALLELIZE)
add_compile_definitions(ND_PARALLELIZE)
endif(ND_PARALLELIZE)

# include
target_include_directories(${PROJECT_NAME} PUBLIC ${PARENT_DIR})

# link: nlohmann json 
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json)

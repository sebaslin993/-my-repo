cmake_minimum_required(VERSION 3.15)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
cmake_policy(VERSION 3.15)
project(TouchEvents)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ImportDependencies)
importDependencies()

# Transform.h/.cpp use GLM on every assignment target.
include(FetchContent)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

add_definitions(-DPLATFORM_WINDOWS -DGLEW_STATIC)
set(SCENE_DIR ${CMAKE_CURRENT_LIST_DIR}/Scene)
file(GLOB SCENE_SRC "${SCENE_DIR}/*.cpp")
add_executable(TouchEvents ${SCENE_SRC})
target_include_directories(TouchEvents PRIVATE ${SCENE_DIR})
target_link_libraries(TouchEvents
    PRIVATE glfw
    PRIVATE libglew_static
    PRIVATE glm::glm
)

if(MSVC)
    target_compile_options(TouchEvents PRIVATE /W4 /WX- /permissive- /std:c++17)
endif()

add_custom_command(TARGET TouchEvents POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_LIST_DIR}/android/app/src/main/assets"
        "$<TARGET_FILE_DIR:TouchEvents>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_LIST_DIR}/Models"
        "$<TARGET_FILE_DIR:TouchEvents>/Models"
    COMMENT "Copying shader and model assets to build directory"
)
set_target_properties(TouchEvents PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:TouchEvents>"
)

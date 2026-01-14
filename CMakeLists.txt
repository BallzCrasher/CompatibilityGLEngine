cmake_minimum_required(VERSION 3.10)
project(OpenGLEngine)

# --- 1. C++ Standard ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- 2. Find Packages ---
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(assimp REQUIRED)


# --- 3. Define Sources and Headers ---
file(GLOB SOURCES "Source/*.cpp")
file(GLOB HEADERS "Include/*.h")

# --- 4. Create Executable ---
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# --- Add Debug Symbols ---
#target_compile_definitions(${PROJECT_NAME} PRIVATE SHOW_COLLISION_BOXES)
#target_compile_options(${PROJECT_NAME} PRIVATE -g)

# --- 5. Include Directories ---
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${OPENGL_INCLUDE_DIR}
    ${GLUT_INCLUDE_DIR}
    ${Assimp_INCLUDE_DIRS}
)

# --- 6. Link Libraries ---
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    OpenGL::GLU       # <--- ADD THIS LINE
    GLUT::GLUT
    assimp::assimp
)

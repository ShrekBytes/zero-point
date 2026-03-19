cmake_minimum_required(VERSION 3.10)
project(ZeroPoint)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)

if(WIN32 OR APPLE)
    # Use system GLUT on Windows/macOS
    find_package(GLUT REQUIRED)
    set(GLUT_LIB GLUT::GLUT)
else()
    # On Linux, prefer FreeGLUT via pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FREEGLUT REQUIRED glut)
    include_directories(${FREEGLUT_INCLUDE_DIRS})
    set(GLUT_LIB ${FREEGLUT_LIBRARIES})
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES} ${GLUT_LIB})

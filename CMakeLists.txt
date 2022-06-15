cmake_minimum_required(VERSION 3.15...3.19)
project(yolo_app)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(DEFINED OpenCV_DIR)
    find_package(OpenCV REQUIRED PATHS ${OpenCV_DIR})
else()
    find_package(OpenCV REQUIRED)
endif()

## Pybind11
## https://pybind11.readthedocs.io/en/latest/compiling.html#building-with-cmake
# CMake does not detect the right Python version? See
# https://pybind11.readthedocs.io/en/stable/faq.html?highlight=cmake#cmake-doesn-t-detect-the-right-python-version
# use -DPYTHON_EXECUTABLE=$(which python) to find your preferred Python version
set(PYBIND11_DIR lib/pybind11)
set(PYBIND11_FINDPYTHON ON)
add_subdirectory(${PYBIND11_DIR})
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# See https://cmake.org/cmake/help/latest/module/FindPython3.html#module:FindPython3
if (NOT PYTHON3_FOUND)
    message(FATAL_ERROR "Python3 is required to build this project")
endif()
message(STATUS "Python3_VERSION=${Python3_VERSION}")
message(STATUS "Python3_INCLUDE_DIRS=${Python3_INCLUDE_DIRS}")
message(STATUS "Python3_LIBRARY=${Python3_LIBRARIES}")

# target spdlog
# you can just comment this line if you want to system library
if (WITH_SPDLOG)
    add_subdirectory(lib/spdlog)
endif ()

# https://github.com/gabime/spdlog
# https://github.com/gabime/spdlog/blob/v1.x/example/CMakeLists.txt
# cd lib/spdlog && mkdir build && cd build
# cmake .. && make -j
# make install
# or install it with package manager
# sudo apt install libspdlog-dev
if (NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif ()

# use CLI11 from https://github.com/CLIUtils/CLI11
if (NOT DEFINED CLI11_DIR)
    set(CLI11_DIR "lib/CLI11" CACHE STRING "CLI11 git repository")
endif ()
include_directories(${CLI11_DIR}/include)

# use https://github.com/HowardHinnant/date
if (NOT DEFINED DATE_DIR)
    set(DATE_DIR "lib/date" CACHE STRING "HowardHinnant/date git repository")
endif ()
include_directories(${DATE_DIR}/include/date)

# OpenMP
find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

# use pkg-config to find glib and gstreamer
# pkg-config --list-all to list all packages
# find_package(PkgConfig)
# pkg_search_module(GLIB REQUIRED glib-2.0)
# pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
# pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
# pkg_check_modules(GST_VIDEO REQUIRED gstreamer-video-1.0)


# NCNN
# I have copied the libs from /ncnn/build/install to /usr/local
# You should set the libs manually in order to compile it
# cross compile
# cmake -DCMAKE_TOOLCHAIN_FILE=../toolchains/aarch64-linux-gnu.toolchain.cmake ..
# /usr/local/bin is for normal user programs not managed by the distribution package manager,
# e.g. locally compiled packages.
# You should not install them into /usr/bin because future distribution upgrades may modify or delete them without warning.
if (NOT DEFINED NCNN_INSTALL_DIR)
    set(NCNN_INSTALL_DIR "/usr/local" CACHE STRING "NCNN install directory")
endif ()
set(NCNN_LIBS ${NCNN_INSTALL_DIR}/lib/libncnn.a)
set(NCNN_INCLUDE_DIRS ${NCNN_INSTALL_DIR}/include/ncnn)

set(YOLO_FASTEST_INCLUDE_DIRS lib/yolo/include)
set(YOLO_FASTEST_SRC lib/yolo/yolo-fastestv2.cpp)

set(MAIN_INCLUDE_DIRS src/include)
set(MAIN_SRC
        src/utils.cpp
        src/detect.cpp
        src/PullTask.cpp
        src/include/VideoInterface.h
        src/VideoInterface.cpp
        src/MainWrapper.cpp
        src/include/MainWrapper.h)

# <------------ add hiredis dependency --------------->
find_path(HIREDIS_HEADER hiredis)

find_library(HIREDIS_LIB hiredis)

# <------------ add redis-plus-plus dependency -------------->
# NOTE: this should be *sw* NOT *redis++*
find_path(REDIS_PLUS_PLUS_HEADER sw)

find_library(REDIS_PLUS_PLUS_LIB redis++)

include_directories(${OpenCV_INCLUDE_DIRS} ${Python3_INCLUDE_DIRS} ${YOLO_FASTEST_INCLUDE_DIRS} ${NCNN_INCLUDE_DIRS} ${MAIN_INCLUDE_DIRS})

set(PY_BIND_SRC
        bind/bind.cpp)

pybind11_add_module(yolo_app
        ${PY_BIND_SRC}
        ${MAIN_SRC}
        ${YOLO_FASTEST_SRC})
target_link_libraries(yolo_app PRIVATE ${OpenCV_LIBS})
target_link_libraries(yolo_app PRIVATE ${HIREDIS_LIB})
target_link_libraries(yolo_app PRIVATE ${REDIS_PLUS_PLUS_LIB})
target_link_libraries(yolo_app PRIVATE ${NCNN_LIBS})

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
include(GNUInstallDirs)
set(YOLO_APP_PY_INSTALL_DIRECTORY "${PROJECT_SOURCE_DIR}/py")
install(TARGETS yolo_app
    COMPONENT python
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${YOLO_APP_PY_INSTALL_DIRECTORY}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")

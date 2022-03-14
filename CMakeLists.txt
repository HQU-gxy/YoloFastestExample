# g++ -g -o demo demo.cpp src/yolo-fastestv2.cpp -I src/include -I /usr/include/ncnn /usr/lib/libncnn.a `pkg-config --libs --cflags opencv4` -fopenmp
# https://blog.csdn.net/TwT520Ly/article/details/80457497

cmake_minimum_required(VERSION 3.14)
project(Yolo-FastestV2)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

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

# from https://github.com/cpm-cmake/CPM.cmake/blob/master/examples/asio-standalone/CMakeLists.txt
# ASIO
set(asio_SOURCE_DIR lib/asio)

find_package(Threads REQUIRED)
add_library(asio INTERFACE)
target_include_directories(asio SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
target_link_libraries(asio INTERFACE Threads::Threads)

# I don't think anyone will run this on Windows
# I still leave this piece of code here for reference
if (WIN32)
    # macro see @ https://stackoverflow.com/a/40217291/1746503
    macro(get_win32_winnt version)
        if (CMAKE_SYSTEM_VERSION)
            set(ver ${CMAKE_SYSTEM_VERSION})
            string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
            string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
            # Check for Windows 10, b/c we'll need to convert to hex 'A'.
            if ("${verMajor}" MATCHES "10")
                set(verMajor "A")
                string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
            endif ("${verMajor}" MATCHES "10")
            # Remove all remaining '.' characters.
            string(REPLACE "." "" ver ${ver})
            # Prepend each digit with a zero.
            string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
            set(${version} "0x${ver}")
        endif ()
    endmacro()

    if (NOT DEFINED _WIN32_WINNT)
        get_win32_winnt(ver)
        set(_WIN32_WINNT ${ver})
    endif ()

    message(STATUS "Set _WIN32_WINNET=${_WIN32_WINNT}")
    target_compile_definitions(asio INTERFACE _WIN32_WINNT=${_WIN32_WINNT} WIN32_LEAN_AND_MEAN)
endif ()

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
set(MAIN_SRC src/main.cpp src/utils.cpp src/detect.cpp src/include/RecognizeInterface.h src/RecognizeInterface.cpp)

# The named <target> must have been created by a command such as add_executable()
# or add_library() and must not be an ALIAS target.
add_executable(Yolo-FastestV2 ${MAIN_SRC} ${YOLO_FASTEST_SRC})

# <------------ add hiredis dependency --------------->
find_path(HIREDIS_HEADER hiredis)
target_include_directories(Yolo-FastestV2 PUBLIC ${HIREDIS_HEADER})

find_library(HIREDIS_LIB hiredis)
target_link_libraries(Yolo-FastestV2 ${HIREDIS_LIB})

# <------------ add redis-plus-plus dependency -------------->
# NOTE: this should be *sw* NOT *redis++*
find_path(REDIS_PLUS_PLUS_HEADER sw)
target_include_directories(Yolo-FastestV2 PUBLIC ${REDIS_PLUS_PLUS_HEADER})

find_library(REDIS_PLUS_PLUS_LIB redis++)
target_link_libraries(Yolo-FastestV2 ${REDIS_PLUS_PLUS_LIB})

include_directories(${OpenCV_INCLUDE_DIRS} ${YOLO_FASTEST_INCLUDE_DIRS} ${NCNN_INCLUDE_DIRS} ${MAIN_INCLUDE_DIRS})

target_link_libraries(Yolo-FastestV2 ${NCNN_LIBS})
target_link_libraries(Yolo-FastestV2 spdlog::spdlog)
target_link_libraries(Yolo-FastestV2 ${OpenCV_LIBS})
target_link_libraries(Yolo-FastestV2 asio)
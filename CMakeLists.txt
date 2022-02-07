# g++ -g -o demo demo.cpp src/yolo-fastestv2.cpp -I src/include -I /usr/include/ncnn /usr/lib/libncnn.a `pkg-config --libs --cflags opencv4` -fopenmp
# https://blog.csdn.net/TwT520Ly/article/details/80457497

cmake_minimum_required(VERSION 3.14)
project( Yolo-Fastestv2 )
find_package( OpenCV REQUIRED )

set(CMAKE_CXX_STANDARD 17)

# OpenMP
find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# NCNN
set(NCNN_LIBS /usr/lib/libncnn.a)
set(NCNN_INCLUDE_DIRS /usr/include/ncnn)

set(YOLO_FASTEST_INCLUDE_DIRS lib/include)
set(YOLO_FASTEST_SRC lib/yolo-fastestv2.cpp)

set(MAIN_SRC src/demo.cpp)

include_directories( ${OpenCV_INCLUDE_DIRS} ${YOLO_FASTEST_INCLUDE_DIRS} ${NCNN_INCLUDE_DIRS} )

add_executable(Yolo-Fastestv2 ${MAIN_SRC} ${YOLO_FASTEST_SRC})
target_link_libraries(Yolo-Fastestv2 ${NCNN_LIBS})
target_link_libraries(Yolo-Fastestv2 ${OpenCV_LIBS})
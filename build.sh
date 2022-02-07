g++ -g -o demo demo.cpp src/yolo-fastestv2.cpp -I src/include -I /usr/include/ncnn /usr/lib/libncnn.a `pkg-config --libs --cflags opencv4` -fopenmp

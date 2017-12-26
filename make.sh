#g++ main.cc libzippp.cpp -I/usr/local/lib -lsfml-graphics  -lsfml-window -lsfml-system -lripe -lzip -o secure-photo-viewer
 g++ main.cc libzippp.cpp mine.cc -I/usr/local/lib -lsfml-graphics  -lsfml-window -lsfml-system -lripe -lzip -lz -o secure-photo-viewer -std=c++11 -O3

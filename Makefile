secure-photo-viewer: main.cc
	g++ main.cc \
		external/libzippp.cpp external/mine.cc \
		-I/usr/local/lib \
		-lsfml-graphics -lsfml-window -lsfml-system -lzip -lz \
		-std=c++17 \
		-O3 -o secure-photo-viewer



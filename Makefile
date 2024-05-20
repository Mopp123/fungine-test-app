SHELL = /bin/bash
build_dir := ./build


build: $(wildcard ./src/*) CMakeLists.txt
	cmake -S . -B $(build_dir)
	cmake --build $(build_dir)
	# Also copy all libs to the build dir so we can run the build
	# immediately without any additional stuff..
	cp ./fungine/glfw/build/src/libglfw.so.3 $(build_dir)
	cp ./fungine/glew/lib/libGLEW.so.2.2 $(build_dir)
	cp ./fungine/freetype/objs/.libs/libfreetype.so.6.20.1 $(build_dir)
	cp ./fungine/assimp/build/bin/libassimp.so.5 $(build_dir)
	cp ./fungine/build/libfungine-engine.so $(build_dir)

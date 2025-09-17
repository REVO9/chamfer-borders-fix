all:
	$(CXX) -shared -fPIC --no-gnu-unique main.cpp -o chamfer-borders.so -g `pkg-config --cflags pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon` -std=c++2b -O2

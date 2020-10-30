CXXFLAGS = -std=c++11
wpa_ctl: main.cpp
	${CXX} ${CXXFLAGS} `pkg-config dbus-cpp --cflags` -I/usr/include main.cpp -o wpa_ctl `pkg-config dbus-cpp --libs` -pthread

ifeq ($(strip $(OPENCV_HOME)),)
#default OPENCV_HOME
OPENCV_HOME=/usr/local/opencv-2.4.1
endif

CXX=g++
CXXFLAGS=-I$(OPENCV_HOME)/include -I/System/Library/Frameworks/GLUT.framework/Headers -g -stdlib=libc++
LDFLAGS=-L${OPENCV_HOME}/lib -lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_imgproc -lopencv_calib3d -framework GLUT -framework OpenGL -lobjc -stdlib=libc++
BUILD_DIR=bin
SRCS= \
	Main.cpp CameraCalibration.cpp GeometryTypes.cpp Marker.cpp MarkerDetector.cpp TinyLA.cpp
OBJS=${SRCS:%.cpp=${BUILD_DIR}/%.o}

simplear: $(OBJS)
	${CXX} ${LDFLAGS} ${OBJS} -o $@
${BUILD_DIR}/%.o : %.cpp
	@mkdir -p $(dir $@)
	${CXX} ${CXXFLAGS} -c $< -o $@
clean:
	rm -rf bin simplear

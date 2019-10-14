cmake_minimum_required(VERSION 3.6)

# TODO: Find a way to require C++11 minimum
#set(CMAKE_CXX_STANDARD 11)

# You can alter these according to your needs, e.g if you don't need to display images - set(YOU_NEED_X11 0)
set(YOU_NEED_X11 False)
set(YOU_NEED_PNG True)
set(YOU_NEED_JPG True)

if(${YOU_NEED_X11})
    message(STATUS "Looking for X11...")
    find_package(X11 REQUIRED)
    set(CIMG_INCLUDE_DIRS ${CIMG_INCLUDE_DIRS} ${X11_INCLUDE_DIR})
    set(CIMG_LIBRARIES ${CIMG_LIBRARIES} ${X11_LIBRARIES})
else()
    set(CIMG_COMPILE_DEFINITIONS ${CIMG_COMPILE_DEFINITIONS} cimg_display=0)
endif()

if(${YOU_NEED_JPG})
    message(STATUS "Looking for libjpg...")
    find_package(JPEG REQUIRED)
    set(CIMG_INCLUDE_DIRS ${CIMG_INCLUDE_DIRS} ${JPEG_INCLUDE_DIR})
    set(CIMG_LIBRARIES ${CIMG_LIBRARIES} ${JPEG_LIBRARY})
    set(CIMG_COMPILE_DEFINITIONS ${CIMG_COMPILE_DEFINITIONS} cimg_use_jpeg=1)
endif()

if(${YOU_NEED_PNG})
    message(STATUS "Looking for libpng...")
    find_package(PNG REQUIRED)
    set(CIMG_INCLUDE_DIRS ${CIMG_INCLUDE_DIRS} ${PNG_INCLUDE_DIR})
    set(CIMG_LIBRARIES ${CIMG_LIBRARIES} ${PNG_LIBRARY})
    set(CIMG_COMPILE_DEFINITIONS ${CIMG_COMPILE_DEFINITIONS} cimg_use_png=1)
endif()

set(CIMG_COMPILE_DEFINITIONS ${CIMG_COMPILE_DEFINITIONS} cimg_use_openmp=0)

find_package_handle_standard_args(CImg DEFAULT_MSG CIMG_LIBRARIES CIMG_INCLUDE_DIRS CIMG_COMPILE_DEFINITIONS)
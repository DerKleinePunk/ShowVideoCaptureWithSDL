
SET(V4L2_SEARCH_PATHS
    ${LIBV4L2_SRC}
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	/usr/lib/arm-linux-gnueabihf/
	/usr/lib/usr/lib/x86_64-linux-gnu/
)

MESSAGE(STATUS "search for LIBV4L2 in ${V4L2_SEARCH_PATHS}")

# Look for the header file.
FIND_PATH(V4L2_INCLUDE_DIR NAMES libv4l2.h
    HINTS
	$ENV{LIBV4L2DIR}
	PATH_SUFFIXES include/libv4l include
	PATHS ${V4L2_SEARCH_PATHS})

# Look for the library.
FIND_LIBRARY(V4L2_LIBRARY NAMES v4l2
    HINTS
	$ENV{LIBV4L2DIR}
	PATH_SUFFIXES lib
	PATHS ${V4L2_SEARCH_PATHS}
)

# Handle the QUIETLY and REQUIRED arguments and set V4L2_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(V4L2 DEFAULT_MSG V4L2_LIBRARY V4L2_INCLUDE_DIR)

# Copy the results to the output variables.
IF(V4L2_FOUND)
	SET(V4L2_LIBRARIES ${V4L2_LIBRARY})
	SET(V4L2_INCLUDE_DIRS ${V4L2_INCLUDE_DIR})
ELSE(V4L2_FOUND)
	SET(LIBV4L2_LIBRARIES)
	SET(LIBV4L2_INCLUDE_DIRS)
ENDIF(V4L2_FOUND)

MARK_AS_ADVANCED(V4L2_INCLUDE_DIRS V4L2_LIBRARIES)

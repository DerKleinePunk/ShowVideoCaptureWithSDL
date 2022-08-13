
SET(LIBJPEG_SEARCH_PATHS
    ${LIBJPEG_SRC}
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

MESSAGE(STATUS "search for jpeglib in ${LIBJPEG_SEARCH_PATHS}")

# Look for the header file.
FIND_PATH(LIBJPEG_INCLUDE_DIR NAMES jpeglib.h
    HINTS
	$ENV{LIBJPEGDIR}
	PATH_SUFFIXES include/libv4l include
	PATHS ${LIBJPEG_SEARCH_PATHS})

# Look for the library.
FIND_LIBRARY(LIBJPEG_LIBRARY NAMES jpeg
    HINTS
	$ENV{LIBJPEGDIR}
	PATH_SUFFIXES lib
	PATHS ${LIBJPEG_SEARCH_PATHS}
)

# Handle the QUIETLY and REQUIRED arguments and set LIBJPEG_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBJPEG DEFAULT_MSG LIBJPEG_LIBRARY LIBJPEG_INCLUDE_DIR)

# Copy the results to the output variables.
IF(LIBJPEG_FOUND)
	SET(LIBJPEG_LIBRARIES ${LIBJPEG_LIBRARY})
	SET(LIBJPEG_INCLUDE_DIRS ${LIBJPEG_INCLUDE_DIR})
ELSE(LIBJPEG_FOUND)
	SET(LIBJPEG_LIBRARIES)
	SET(LIBJPEG_INCLUDE_DIRS)
ENDIF(LIBJPEG_FOUND)

MARK_AS_ADVANCED(LIBJPEG_INCLUDE_DIRS LIBJPEG_LIBRARIES)

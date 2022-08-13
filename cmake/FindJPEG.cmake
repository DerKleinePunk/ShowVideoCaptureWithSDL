
SET(JPEG_SEARCH_PATHS
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
FIND_PATH(JPEG_INCLUDE_DIR NAMES jpeglib.h
    HINTS
	$ENV{LIBJPEGDIR}
	PATH_SUFFIXES include
	PATHS ${JPEG_SEARCH_PATHS})

# Look for the library.
FIND_LIBRARY(JPEG_LIBRARY NAMES jpeg
    HINTS
	$ENV{LIBJPEGDIR}
	PATH_SUFFIXES lib
	PATHS ${JPEG_SEARCH_PATHS}
)

# Handle the QUIETLY and REQUIRED arguments and set LIBJPEG_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(JPEG DEFAULT_MSG JPEG_LIBRARY JPEG_INCLUDE_DIR)

# Copy the results to the output variables.
IF(JPEG_FOUND)
	SET(JPEG_LIBRARIES ${JPEG_LIBRARY})
	SET(JPEG_INCLUDE_DIRS ${JPEG_INCLUDE_DIR})
ELSE(JPEG_FOUND)
	SET(JPEG_LIBRARIES)
	SET(JPEG_INCLUDE_DIRS)
ENDIF(JPEG_FOUND)

MARK_AS_ADVANCED(JPEG_INCLUDE_DIRS JPEG_LIBRARIES)

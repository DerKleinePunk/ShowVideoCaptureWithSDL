# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
SET(CORE_SYSTEM_NAME rbpi)
SET(CMAKE_SYSTEM_PROCESSOR arm)
# Raspberry 2
SET(CPU cortex-a7)
SET(ARCH arm)
SET(ENABLE_NEON TRUE)
SET(OS "linux")
#-march=armv7
SET(CMAKE_C_FLAGS "-fPIC -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4")
SET(CMAKE_CXX_FLAGS "-fPIC -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4")
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)
SET(RPIROOT /home/punky/x-tools/rpiroot)
SET(CMAKE_SYSROOT ${RPIROOT})
SET(CMAKE_PREFIX_PATH rbpi)
# specify the cross compiler
SET(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc-6)
SET(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++-6)
SET(CMAKE_AR arm-linux-gnueabihf-ar)
SET(CMAKE_FIND_ROOT_PATH ${RPIROOT})
# SET(CMAKE_SYSROOT ${SYSROOT})
# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
SET(GCC_COVERAGE_LINK_FLAGS "-v")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
SET(ENV{PKG_CONFIG_PATH} "${RPIROOT}/lib/arm-linux-gnueabihf/pkgconfig:${RPIROOT}/usr/share/pkgconfig")
SET(ENV{PKG_CONFIG_LIBDIR} "${RPIROOT}/usr/lib/arm-linux-gnueabihf/pkgconfig:${RPIROOT}/usr/lib/pkgconfig")
SET(ENV{PKG_CONFIG_SYSROOT_DIR} ${RPIROOT})
SET(ENV{CFLAGS} "-I${RPIROOT}/opt/vc/include -I${RPIROOT}/opt/vc/include/interface/vcos/pthreads -I${RPIROOT}/opt/vc/include/interface/vmcs_host/linux -L${RPIROOT}/opt/vc/lib")

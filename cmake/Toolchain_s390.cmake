# http://www.cmake.org/Wiki/CmakeMingw
# http://www.cmake.org/Wiki/CMake_Cross_Compiling#The_toolchain_file
# http://wiki.debian.org/EmdebianToolchain#Get_the_binaries

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Generic)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER /usr/bin/s390-linux-gnu-gcc-4.4)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/s390-linux-gnu/)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


# http://www.cmake.org/Wiki/CmakeMingw
# http://www.cmake.org/Wiki/CMake_Cross_Compiling#The_toolchain_file

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Generic)

# which compilers to use for C and C++
# Settings for Ubuntu 12.04.1 LTS
SET(CMAKE_C_COMPILER /usr/bin/gcc)

# here is the target environment located
# Settings for Ubuntu 12.04.1 LTS
SET(CMAKE_FIND_ROOT_PATH  /usr/)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


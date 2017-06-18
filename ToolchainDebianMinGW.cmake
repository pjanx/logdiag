set (CMAKE_SYSTEM_NAME "Windows")
set (CMAKE_SYSTEM_PROCESSOR "x86")

set (CMAKE_C_COMPILER "i586-mingw32msvc-gcc")
set (CMAKE_CXX_COMPILER "i586-mingw32msvc-g++")
set (CMAKE_RC_COMPILER "i586-mingw32msvc-windres")

# Not needed to crosscompile an installation package
#set (CMAKE_CROSSCOMPILING_EMULATOR "wine")

set (CMAKE_FIND_ROOT_PATH "/usr/i586-mingw32msvc")

set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


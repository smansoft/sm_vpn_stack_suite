# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

set (SM_LOG_LIB 			"sm_log_lib-0.0.2.2")
set (SM_SSTRINGLIB 			"sstring_lib-0.0.1.1")
set (SM_VPN 				"vpn")

include_directories(.)
include_directories(${SM_LOG_LIB}/include)
include_directories(${SM_SSTRINGLIB}/include)
include_directories(${SM_VPN}/)

file(GLOB HEADER_FILES 					${${PROJECT_NAME}_SOURCE_DIR}/*.h)
file(GLOB SOURCE_FILES            		${${PROJECT_NAME}_SOURCE_DIR}/*.c*)

file(GLOB LOG_HEADER_FILES 				${${PROJECT_NAME}_SOURCE_DIR}/${SM_LOG_LIB}/include/*.h)
file(GLOB LOG_SOURCE_FILES 				${${PROJECT_NAME}_SOURCE_DIR}/${SM_LOG_LIB}/src/*.c*)

file(GLOB SSTRING_HEADER_FILES 			${${PROJECT_NAME}_SOURCE_DIR}/${SM_SSTRINGLIB}/include/*.h)
file(GLOB SSTRING_SOURCE_FILES 			${${PROJECT_NAME}_SOURCE_DIR}/${SM_SSTRINGLIB}/src/*.c*)

file(GLOB VPN_HEADER_FILES 				${${PROJECT_NAME}_SOURCE_DIR}/${SM_VPN}/*.h)
file(GLOB VPN_SOURCE_FILES 				${${PROJECT_NAME}_SOURCE_DIR}/${SM_VPN}/*.c*)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
			vpn_stack-lib
			# Sets the library as a shared library.
			SHARED
			# Provides a relative path to your source file(s).
			${HEADER_FILES} ${SOURCE_FILES}
			${LOG_HEADER_FILES} ${LOG_SOURCE_FILES}
			${SSTRING_HEADER_FILES} ${SSTRING_SOURCE_FILES}
			${VPN_HEADER_FILES} ${VPN_SOURCE_FILES}
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              LOG-LIB
              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       vpn_stack-lib
                       # Links the target library to the log library
                       # included in the NDK.
                       ${LOG-LIB} )

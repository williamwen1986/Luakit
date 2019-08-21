# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_SYS_TIME_H -DANDROID -Wno-multichar -DHAVE_CONFIG_H -DDYNAMIC_ANNOTATIONS_ENABLED=1 -DOS_ANDROID")


add_library(
                # Sets the name of the library.
                event

                # Sets the library as a shared library.
                STATIC

                # Provides a relative path to your source file(s).
                		buffer.c
				epoll.c
				evbuffer.c
				evdns.c
				event.c
				event_tagging.c
				evrpc.c
				evutil.c
				http.c
				log.c
				poll.c
				select.c
				sample/event-test.c
				sample/signal-test.c
				sample/time-test.c
				signal.c
				strlcpy.c
				test/bench.c
				test/regress.c
				test/regress.gen.c
				test/regress_dns.c
				test/regress_http.c
				test/regress_rpc.c
				test/test-eof.c
				test/test-init.c
				test/test-time.c
				test/test-weof.c
)

# Specifies a path to native header files.
include_directories(
                		$(PROJECT_SOURCE_DIR)
				$(PROJECT_SOURCE_DIR)/android
				$(PROJECT_SOURCE_DIR)/compat
				$(PROJECT_SOURCE_DIR)/compat/sys
				$(PROJECT_SOURCE_DIR)/freebsd
				$(PROJECT_SOURCE_DIR)/linux
				$(PROJECT_SOURCE_DIR)/mac
				$(PROJECT_SOURCE_DIR)/sample
				$(PROJECT_SOURCE_DIR)/solaris
				$(PROJECT_SOURCE_DIR)/test

)

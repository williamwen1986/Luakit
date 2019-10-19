cmake_minimum_required(VERSION 3.4.1)
project(luakit)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_HISTORICAL_STRERRO_R -DNVALGRIND -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -frtti -fPIC -DUSE_HISTORICAL_STRERRO_R")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -llog -lz -lc")


add_library( luaFramework 
        STATIC 
        dummy.cpp
)

link_directories(${PROJECT_SOURCE_DIR}/../libs/android${ANDROID_NATIVE_API_LEVEL}-${CMAKE_BUILD_TYPE})

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/jni/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/jni" ${lib_build_DIR} )
set_target_properties( jni PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libjni.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../extensions/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../extensions" ${lib_build_DIR} )
set_target_properties( extensions PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libextensions.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../third-party/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../third-party" ${lib_build_DIR} )
set_target_properties( third_party PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libthird_party.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../src/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../src" ${lib_build_DIR} )
set_target_properties( luakit_engine PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libluakit_engine.a)

### ssl ###

add_library( ssl
             STATIC
             IMPORTED )

set_target_properties( # Specifies the target library.
                       ssl

                       # Specifies the parameter you want to define.
                       PROPERTIES IMPORTED_LOCATION

                      # Provides the path to the library you want to import.
                      ${PROJECT_SOURCE_DIR}/../libs/android${ANDROID_NATIVE_API_LEVEL}-${CMAKE_BUILD_TYPE}/${ANDROID_ABI}/libssl.a 
                )
### crypto ###

add_library( crypto
             STATIC
             IMPORTED )

set_target_properties( # Specifies the target library.
                       crypto

                       # Specifies the parameter you want to define.
                       PROPERTIES IMPORTED_LOCATION

                        #Provides the path to the library you want to import.
                        ${PROJECT_SOURCE_DIR}/../libs/android${ANDROID_NATIVE_API_LEVEL}-${CMAKE_BUILD_TYPE}/${ANDROID_ABI}/libcrypto.a 
                      )
###

###link_directories(${outputDir}/${ANDROID_ABI})

target_link_libraries(luaFramework
                luakit_engine
                extensions
                base
                icu
                dmg_fp
                event
                curl
                sqlite
                ssl
                crypto
                third_party
                lua
                jni
                ###notification
                ###thread
)

# Specifies a path to native header files.
include_directories(
                    $ENV{OUTPUT_DIR}/
                    ${PROJECT_SOURCE_DIR}/
		    ${PROJECT_SOURCE_DIR}/jni
                    ${PROJECT_SOURCE_DIR}/../src
                    ${PROJECT_SOURCE_DIR}/../src/common/
                    ${PROJECT_SOURCE_DIR}/../third-party
                    ${PROJECT_SOURCE_DIR}/../third-party/openssl/include/
                    ${PROJECT_SOURCE_DIR}/../third-party/curl/include
                    ${PROJECT_SOURCE_DIR}/../third-party/lua-5.1.5/src
                    ${PROJECT_SOURCE_DIR}/../src/lua-tools
                    ${PROJECT_SOURCE_DIR}/../third-party/sqlite-amalgamation-3290000
                    ${PROJECT_SOURCE_DIR}/../third-party/toluapp/include
                    ${PROJECT_SOURCE_DIR}/../include

)

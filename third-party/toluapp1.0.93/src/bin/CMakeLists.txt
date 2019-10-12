add_executable(tolua++_bootstrap
	tolua.c
	toluabind_default.c
)

if(UNIX AND NOT APPLE)
	find_library(DL_LIBRARY dl)
	list(APPEND LUA_LIBRARIES ${DL_LIBRARY})
endif()

target_link_libraries(tolua++_bootstrap
	libtolua++
	"${LUA_LIBRARIES}"
)

file(GLOB LUA_FILES
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	"lua/*.lua"
)

add_custom_command(OUTPUT toluabind.c
	COMMAND tolua++_bootstrap -C -o "${CMAKE_CURRENT_BINARY_DIR}/toluabind.c" tolua.pkg
	DEPENDS tolua++_bootstrap tolua.pkg ${LUA_FILES}
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(tolua++
	tolua.c
	toluabind.c
)

target_link_libraries(tolua++
	libtolua++
	"${LUA_LIBRARIES}"
)

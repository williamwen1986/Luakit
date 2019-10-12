add_library(libtolua++
	tolua_event.c
	tolua_is.c
	tolua_map.c
	tolua_push.c
	tolua_to.c
)

set_target_properties(libtolua++ PROPERTIES OUTPUT_NAME "tolua++")

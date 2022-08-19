apply_include_paths("${JRTPLIB_INTERNAL_INCLUDES}")
apply_include_paths("${JRTPLIB_EXTERNAL_INCLUDES}")

add_definitions(-DRTP_SUPPORT_THREAD)

foreach(IDX 1 2 3 4 5 6 7 8)
	add_executable(example${IDX} example${IDX}.cpp)
	if (NOT MSVC OR JRTPLIB_COMPILE_STATIC)
		target_link_libraries(example${IDX} jrtplib-static /localhome/local-ydrame/Downloads/MAXINE/JThread/build/src/libjthread.a pthread)
	else ()
		target_link_libraries(example${IDX} jrtplib-shared /localhome/local-ydrame/Downloads/MAXINE/JThread/build/src/libjthread.a pthread)
	endif ()
endforeach(IDX)



message(STATUS CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

message(STATUS "platform: ${CMAKE_SYSTEM_NAME}")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
endif()

message(STATUS "compiler: ${CMAKE_CXX_COMPILER_ID}")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# add_compile_options("-Werror")
	add_compile_options("-Wall")

	if(NOT WIN32)
		if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
			add_compile_options(-fsanitize=leak -fsanitize=address -fno-omit-frame-pointer)
			set(LINK_LIBRARIES asan)
		endif()
	endif()

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/W4")
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
	add_compile_options("/Zc:__cplusplus")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

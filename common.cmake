
message(STATUS CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

message(STATUS "platform: ${CMAKE_SYSTEM_NAME}")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
endif()

message(STATUS "compiler: ${CMAKE_CXX_COMPILER_ID}")

if(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    add_compile_options("-Wall")
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
    # add_compile_options("-Werror")
    add_compile_options("-Wall")

    if(NOT WIN32)
        if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
            add_compile_options(-fsanitize=leak -fsanitize=address -fno-omit-frame-pointer)
            set(LINK_LIBRARIES asan)
        endif()
    endif()

elseif(${CMAKE_CXX_COMPILER_ID} MATCHES Intel)
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES MSVC)
    # 移除默认的警告级别，然后设置 W4
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    add_compile_options("/W4")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("/Zc:__cplusplus")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

function(GET_OPENCV_DLLS RESULT)
    file(TO_CMAKE_PATH ${OpenCV_LIB_PATH}/../bin OpenCV_BIN_PATH)
    set(OpenCV_DLLS ${OpenCV_DLLS}
        ${OpenCV_BIN_PATH}/opencv_world${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}.dll)
    set(OpenCV_DLLS ${OpenCV_DLLS}
        ${OpenCV_BIN_PATH}/opencv_world${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}d.dll)
    set(${RESULT} ${OpenCV_DLLS} PARENT_SCOPE)
endfunction(GET_OPENCV_DLLS)

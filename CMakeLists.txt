# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(my_tools)

if(NOT DEFINED CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED True)
endif()

# 使用通配符添加多个源文件
file(GLOB SRC_LIST "./src/*.cpp")

if(NOT PROJECT_IS_TOP_LEVEL)
	set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES}
		${PROJECT_SOURCE_DIR}/
		${PROJECT_SOURCE_DIR}/include
		PARENT_SCOPE
	)
	set(LINK_LIBRARIES ${LINK_LIBRARIES}
		${PROJECT_NAME}
		PARENT_SCOPE
	)
endif()

# 头文件路径
set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES}
	${PROJECT_SOURCE_DIR}/
	${PROJECT_SOURCE_DIR}/include
)

include_directories(${INCLUDE_DIRECTORIES})

if(NOT TARGET ${PROJECT_NAME})
	add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
endif()

# 生成可执行的文件
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

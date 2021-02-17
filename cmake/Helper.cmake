macro(LINK_LIB libName)
	find_package(${libName})
	if(NOT ${libName}_NOTFOUND)

		include(FetchContent)

		set(addr "https://github.com/hegaoxiang/${libName}.git")

		message("not find ${libName} , begin loading from ${addr}")

		FetchContent_Declare(
			${libName}
			GIT_REPOSITORY "${addr}"
		)
		FetchContent_MakeAvailable(${libName})
	endif()

	target_link_libraries(${PROJECT_NAME} ${libName})
endmacro()


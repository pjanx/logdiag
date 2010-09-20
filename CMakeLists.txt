project (logdiag)
cmake_minimum_required (VERSION 2.6.2)

# Default to 2.6.3 behaviour
cmake_policy (VERSION 2.6.3)

# Options
option (OPTION_NOINSTALL "Only for developers; work without installing" OFF)

if (OPTION_NOINSTALL)
	set (OPTION_NOINSTALL 1)
else (OPTION_NOINSTALL)
	set (OPTION_NOINSTALL 0)
endif (OPTION_NOINSTALL)


# Version
set (project_VERSION_MAJOR "0")
set (project_VERSION_MINOR "0")
set (project_VERSION_PATCH "1")

set (project_VERSION "${project_VERSION_MAJOR}")
set (project_VERSION "${project_VERSION}.${project_VERSION_MINOR}")
set (project_VERSION "${project_VERSION}.${project_VERSION_PATCH}")

# For custom modules
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Gather package information
find_package (GTK2 2.12 REQUIRED gtk)

# Test this machine
include (CheckCSourceCompiles)

CHECK_C_SOURCE_COMPILES (
"typedef struct abc *d;
int test (d __restrict x);
int main (void) {return 0;}"
HAVE_SANE___RESTRICT)

CHECK_C_SOURCE_COMPILES (
"int test (void *restrict x);
int main (void) {return 0;}"
HAVE_RESTRICT)

include (CheckFunctionExists)

CHECK_FUNCTION_EXISTS ("strtok_r" HAVE_STRTOK_R)
CHECK_FUNCTION_EXISTS ("strtok_s" HAVE_STRTOK_S)

if (WIN32)
        set (HAVE_THREADSAFE_STRTOK true)
endif (WIN32)

# Lua
find_package (Lua51 REQUIRED)


# Localization
find_package (Gettext)
if (GETTEXT_FOUND)
	set (HAVE_GETTEXT true)
	file (GLOB project_TRANSLATIONS ${CMAKE_CURRENT_SOURCE_DIR}/po/*.po)
	GETTEXT_CREATE_TRANSLATIONS (
		${CMAKE_CURRENT_SOURCE_DIR}/po/${PROJECT_NAME}.pot
		ALL ${project_TRANSLATIONS})
endif (GETTEXT_FOUND)

# Documentation
# TODO: Pregenerated docs

#set (GTK_DOC_ENABLE OFF CACHE BOOL
#	"Use gtk-doc to build documentation")
#set (GTK_DOC_ENABLE_HTML ON CACHE BOOL
#	"Build documentation in HTML format")

find_package (GtkDoc REQUIRED)
set (project_DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/docs/reference")
GTK_DOC_RUN (WORKING_DIR ${project_DOC_DIR}
	SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
	XML ${project_DOC_DIR}/xml --sgml-mode
	HTML ${project_DOC_DIR}/html)

# Project source files
set (logdiag_SOURCES
	src/logdiag.c
	src/ld-marshal.c
	src/ld-window-main.c
	src/ld-document.c
	src/ld-canvas.c
	src/ld-symbol.c
	src/ld-symbol-category.c
	src/ld-symbol-library.c)
set (logdiag_HEADERS
	${CMAKE_CURRENT_BINARY_DIR}/config.h
	src/ld-marshal.h
	src/ld-window-main.h
	src/ld-document.h
	src/ld-canvas.h
	src/ld-symbol.h
	src/ld-symbol-category.h
	src/ld-symbol-library.h)

# Generate a configure file
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories (${CMAKE_CURRENT_BINARY_DIR})

# Generate marshallers
function (glib_genmarshal base prefix)
	find_program (GLIB_GENMARSHAL_EXECUTABLE glib-genmarshal)

	if (GLIB_GENMARSHAL_EXECUTABLE)
		get_filename_component (base_path "${base}" PATH)
		get_filename_component (base_name "${base}" NAME)
		set (command_base ${GLIB_GENMARSHAL_EXECUTABLE}
			--prefix ${prefix} "${base_name}.list")

		# NOTE: VERBATIM may cause problems, so it is not used here
		add_custom_command (OUTPUT ${base}.c
			COMMAND ${command_base} --body   > "${base_name}.c"
			DEPENDS "${base}.list"
			WORKING_DIRECTORY "${base_path}"
			COMMENT "Generating marshallers source file")
		add_custom_command (OUTPUT ${base}.h
			COMMAND ${command_base} --header > "${base_name}.h"
			DEPENDS "${base}.list"
			WORKING_DIRECTORY "${base_path}"
			COMMENT "Generating marshallers header file")
	else (GLIB_GENMARSHAL_EXECUTABLE)
		message (WARNING "glib-genmarshal is not present on this system")
	endif (GLIB_GENMARSHAL_EXECUTABLE)
endfunction (glib_genmarshal)

glib_genmarshal (${CMAKE_CURRENT_SOURCE_DIR}/src/ld-marshal
	g_cclosure_user_marshal)

# Build the executable
include_directories (${GTK2_INCLUDE_DIRS} ${LUA_INCLUDE_DIR})
add_executable (logdiag ${logdiag_SOURCES} ${logdiag_HEADERS})
target_link_libraries (logdiag ${GTK2_LIBRARIES} ${LUA_LIBRARIES})

# Installation
install (TARGETS logdiag DESTINATION bin)
install (FILES share/logdiag.desktop DESTINATION share/applications)
install (DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs/html/"
	DESTINATION usr/gtk-doc/${CMAKE_PROJECT_NAME})

# CPack
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Electric diagram designer.")
set (CPACK_PACKAGE_VENDOR "Přemysl Janouch")
set (CPACK_PACKAGE_CONTACT "p.janouch@gmail.com")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR ${project_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${project_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${project_VERSION_PATCH})
set (CPACK_GENERATOR "TGZ;ZIP")
set (CPACK_PACKAGE_FILE_NAME
	"${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set (CPACK_PACKAGE_INSTALL_DIRECTORY
	"${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
set (CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set (CPACK_SOURCE_IGNORE_FILES "/build;/.svn;/.git")
set (CPACK_SOURCE_PACKAGE_FILE_NAME
	"${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")

include (CPack)


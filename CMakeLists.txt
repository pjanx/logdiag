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


# Dependencies
find_package (Lua51 REQUIRED)
find_package (JsonGlib REQUIRED)

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

find_package (GtkDoc)
set (project_DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/docs/reference")

# Project source files
set (liblogdiag_SOURCES
	liblogdiag/ld-marshal.c
	liblogdiag/ld-types.c
	liblogdiag/ld-diagram.c
	liblogdiag/ld-diagram-object.c
	liblogdiag/ld-diagram-symbol.c
	liblogdiag/ld-canvas.c
	liblogdiag/ld-library.c
	liblogdiag/ld-symbol-category.c
	liblogdiag/ld-symbol.c
	liblogdiag/ld-lua.c
	liblogdiag/ld-lua-symbol.c)
set (liblogdiag_HEADERS
	${CMAKE_CURRENT_BINARY_DIR}/config.h
	liblogdiag/liblogdiag.h
	liblogdiag/ld-marshal.h
	liblogdiag/ld-types.h
	liblogdiag/ld-diagram.h
	liblogdiag/ld-diagram-object.h
	liblogdiag/ld-diagram-symbol.h
	liblogdiag/ld-canvas.h
	liblogdiag/ld-library.h
	liblogdiag/ld-symbol-category.h
	liblogdiag/ld-symbol.h
	liblogdiag/ld-lua.h
	liblogdiag/ld-lua-private.h
	liblogdiag/ld-lua-symbol.h
	liblogdiag/ld-lua-symbol-private.h)

set (logdiag_SOURCES
	src/ld-window-main.c
	src/logdiag.c)
set (logdiag_HEADERS
	${liblogdiag_HEADERS}
	src/ld-window-main.h)

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

		# On Windows, the final makefile may contain:
		#   cd e:\abc && ...
		# That won't actually change the directory. We have to do:
		#   cd e:\abc && e: && ...
		if (WIN32 AND "${base_path}" MATCHES "^.:[/\\\\]")
			string (SUBSTRING "${base_path}" 0 2 base_drive)
			set (command_base "${base_drive}" && "${command_base}")
		endif (WIN32 AND "${base_path}" MATCHES "^.:[/\\\\]")

		# NOTE: VERBATIM would cause problems, so it is not used here
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

glib_genmarshal (${CMAKE_CURRENT_SOURCE_DIR}/liblogdiag/ld-marshal
	g_cclosure_user_marshal)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/liblogdiag)
include_directories (${GTK2_INCLUDE_DIRS}
	${JSON_GLIB_INCLUDE_DIRS} ${LUA_INCLUDE_DIR})

# Build the library
add_library (liblogdiag STATIC ${liblogdiag_SOURCES} ${liblogdiag_HEADERS})
set_target_properties (liblogdiag PROPERTIES OUTPUT_NAME logdiag)

# Build the executable
add_executable (logdiag ${logdiag_SOURCES} ${logdiag_HEADERS})
target_link_libraries (logdiag liblogdiag ${GTK2_LIBRARIES}
	${JSON_GLIB_LIBRARIES} ${LUA_LIBRARIES})

# Generate documentation
if (GTK_DOC_FOUND)
	GTK_DOC_RUN (WORKING_DIR ${project_DOC_DIR}
		SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/liblogdiag
		XML ${project_DOC_DIR}/xml --sgml-mode
		HTML ${project_DOC_DIR}/html)
endif (GTK_DOC_FOUND)

# Installation
install (TARGETS logdiag DESTINATION bin)
install (DIRECTORY share/gui share/library
	DESTINATION share/${CMAKE_PROJECT_NAME})

if (NOT WIN32)
	install (FILES share/logdiag.desktop DESTINATION share/applications)
	install (DIRECTORY share/icons DESTINATION share)
	install (DIRECTORY "${project_DOC_DIR}/html/"
		DESTINATION share/gtk-doc/${CMAKE_PROJECT_NAME})
	install (FILES LICENSE DESTINATION share/doc/${CMAKE_PROJECT_NAME})
endif (NOT WIN32)

# CPack
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Schematic editor")
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


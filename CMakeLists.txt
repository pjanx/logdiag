project (logdiag C)
cmake_minimum_required (VERSION 2.8)

# Default to 2.8 behaviour
cmake_policy (VERSION 2.8)

# Options
option (OPTION_USE_VERA "Use vera++ for source code style checks" OFF)
option (OPTION_NOINSTALL "Only for developers; work without installing" OFF)
option (BUILD_TESTING "Build tests" OFF)

# Project information
set (project_VERSION_MAJOR "0")
set (project_VERSION_MINOR "2")
set (project_VERSION_PATCH "0")

set (project_VERSION "${project_VERSION_MAJOR}")
set (project_VERSION "${project_VERSION}.${project_VERSION_MINOR}")
set (project_VERSION "${project_VERSION}.${project_VERSION_PATCH}")

set (project_URL "https://git.janouch.name/p/logdiag")

# For custom modules
set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Automatic retrieval of dependencies
if (WIN32)
	set (WIN32_DEPENDS_PATH ${PROJECT_SOURCE_DIR}/win32-depends)
	list (APPEND CMAKE_PREFIX_PATH ${WIN32_DEPENDS_PATH})
	list (APPEND CMAKE_INCLUDE_PATH ${WIN32_DEPENDS_PATH}/lib)
	list (APPEND CMAKE_C_FLAGS "-mms-bitfields")

	if (CMAKE_CROSSCOMPILING)
		list (APPEND CMAKE_FIND_ROOT_PATH ${WIN32_DEPENDS_PATH})
	endif (CMAKE_CROSSCOMPILING)

	set (PKG_CONFIG_USE_CMAKE_PREFIX_PATH FALSE)
	# Seems like there is a regression setting this to two paths in one,
	# setting the path manually
	set (ENV{PKG_CONFIG_PATH} "${WIN32_DEPENDS_PATH}/lib/pkgconfig")

	add_custom_target (win32-depends COMMAND ${CMAKE_COMMAND} -P
		${PROJECT_SOURCE_DIR}/Win32Depends.cmake
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
	add_custom_target (win32-depends-clean COMMAND ${CMAKE_COMMAND} -E
		remove_directory ${WIN32_DEPENDS_PATH})
endif (WIN32)

# Generate icons
find_program (CONVERT_EXECUTABLE convert)
mark_as_advanced (CONVERT_EXECUTABLE)

if (CONVERT_EXECUTABLE AND NOT WIN32)
	set (icon_base ${PROJECT_SOURCE_DIR}/share/icons/hicolor)
	set (icon_svg ${icon_base}/scalable/apps/logdiag.svg)
	set (icon_size_list 16x16 32x32 48x48)

	set (icon_png_list)
	set (icon_symbolic_list)
	foreach (icon_size ${icon_size_list})
		set (icon_png_path ${icon_base}/${icon_size}/apps)
		set (icon_png ${icon_png_path}/logdiag.png)
		list (APPEND icon_png_list ${icon_png})
		set (icon_symbolic ${PROJECT_NAME}_icons_${icon_size})
		list (APPEND icon_symbolic_list ${icon_symbolic})

		add_custom_command (OUTPUT ${icon_symbolic}
			COMMAND ${CMAKE_COMMAND} -E make_directory ${icon_png_path}
			COMMAND ${CONVERT_EXECUTABLE} -background none
				-resize ${icon_size} ${icon_svg} ${icon_png}
			DEPENDS ${icon_svg} VERBATIM)
		set_source_files_properties (${icon_symbolic}
			PROPERTIES SYMBOLIC TRUE)
	endforeach (icon_size)

	set (icon_ico ${PROJECT_SOURCE_DIR}/share/logdiag.ico)
	add_custom_target (${PROJECT_NAME}_icons
		DEPENDS ${icon_symbolic_list}
		COMMAND ${CONVERT_EXECUTABLE} ${icon_png_list} ${icon_ico}
		COMMENT "Generating logdiag.ico" VERBATIM)
endif (CONVERT_EXECUTABLE AND NOT WIN32)

# Dependencies
find_package (PkgConfig REQUIRED)
pkg_check_modules (GTK3 REQUIRED gtk+-3.0 json-glib-1.0)

if (NOT WIN32)
	pkg_search_module (Lua REQUIRED lua>=5.2 lua5.3 lua-5.3 lua5.2 lua-5.2)
else (NOT WIN32)
	# For whatever reason this now seems to be required
	set (LUA_INCLUDE_DIR "${WIN32_DEPENDS_PATH}/include")

	# XXX: this seems to require CMake 3.0 ... include it in the project?
	find_package (Lua REQUIRED)
	if (LUA_VERSION_STRING VERSION_LESS "5.2")
		message (FATAL_ERROR "Lua 5.2 not found")
	endif (LUA_VERSION_STRING VERSION_LESS "5.2")
	set (Lua_LIBRARIES ${LUA_LIBRARIES})
	set (Lua_INCLUDE_DIRS ${LUA_INCLUDE_DIR})
endif (NOT WIN32)

# This actually fucks up MinGW cross-compilation if omitted
link_directories (${GTK3_LIBRARY_DIRS})

# Localization
find_package (Gettext REQUIRED)
file (GLOB project_PO_FILES ${PROJECT_SOURCE_DIR}/po/*.po)
GETTEXT_CREATE_TRANSLATIONS (
	${PROJECT_SOURCE_DIR}/po/${PROJECT_NAME}.pot
	ALL ${project_PO_FILES})

foreach (file ${project_PO_FILES})
	get_filename_component (translation "${file}" NAME_WE)
	list (APPEND project_TRANSLATIONS "${translation}")
endforeach (file)

# Documentation
find_package (GtkDoc 1.25)
set (project_DOC_DIR "${PROJECT_BINARY_DIR}/liblogdiag")

# Project source files
set (liblogdiag_SOURCES
	liblogdiag/ld-marshal.c
	liblogdiag/ld-types.c
	liblogdiag/ld-undo-action.c
	liblogdiag/ld-diagram.c
	liblogdiag/ld-diagram-object.c
	liblogdiag/ld-diagram-symbol.c
	liblogdiag/ld-diagram-connection.c
	liblogdiag/ld-diagram-view.c
	liblogdiag/ld-library.c
	liblogdiag/ld-category-view.c
	liblogdiag/ld-category-tree-view.c
	liblogdiag/ld-category-symbol-view.c
	liblogdiag/ld-category.c
	liblogdiag/ld-symbol.c
	liblogdiag/ld-lua.c
	liblogdiag/ld-lua-symbol.c)
set (liblogdiag_HEADERS
	${PROJECT_BINARY_DIR}/config.h
	liblogdiag/liblogdiag.h
	liblogdiag/ld-marshal.h
	liblogdiag/ld-types.h
	liblogdiag/ld-undo-action.h
	liblogdiag/ld-diagram.h
	liblogdiag/ld-diagram-object.h
	liblogdiag/ld-diagram-symbol.h
	liblogdiag/ld-diagram-connection.h
	liblogdiag/ld-diagram-view.h
	liblogdiag/ld-library.h
	liblogdiag/ld-category-view.h
	liblogdiag/ld-category-tree-view.h
	liblogdiag/ld-category-symbol-view.h
	liblogdiag/ld-category.h
	liblogdiag/ld-symbol.h
	liblogdiag/ld-lua.h
	liblogdiag/ld-lua-private.h
	liblogdiag/ld-lua-symbol.h
	liblogdiag/ld-lua-symbol-private.h)

set (logdiag_TESTS
	point-array
	diagram)

set (logdiag_SOURCES
	src/ld-window-main.c
	src/logdiag.c)
set (logdiag_HEADERS
	${liblogdiag_HEADERS}
	src/ld-window-main.h)

# Resource compilation for MinGW
if (MINGW)
	set (CMAKE_RC_COMPILER_INIT windres)
	enable_language (RC)
	set (CMAKE_RC_FLAGS "-O coff")
	set (CMAKE_RC_COMPILE_OBJECT
		"<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -o <OBJECT> <SOURCE>")
endif (MINGW)

if (WIN32)
	set (logdiag_SOURCES
		${logdiag_SOURCES}
		share/logdiag.rc)
endif (WIN32)

# Generate a configure file
configure_file (${PROJECT_SOURCE_DIR}/config.h.in
	${PROJECT_BINARY_DIR}/config.h)
include_directories (${PROJECT_BINARY_DIR})

# Generate marshallers
find_program (GLIB_GENMARSHAL_EXECUTABLE glib-genmarshal)
if (NOT GLIB_GENMARSHAL_EXECUTABLE)
	message (FATAL_ERROR "glib-genmarshal not found")
endif (NOT GLIB_GENMARSHAL_EXECUTABLE)

function (glib_genmarshal base prefix)
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
endfunction (glib_genmarshal)

glib_genmarshal (${PROJECT_SOURCE_DIR}/liblogdiag/ld-marshal
	ld_marshal)

include_directories (${PROJECT_SOURCE_DIR})
include_directories (${GTK3_INCLUDE_DIRS} ${Lua_INCLUDE_DIRS})
set (logdiag_LIBS ${GTK3_LIBRARIES} ${Lua_LIBRARIES} m)

if (WIN32)
	find_package (LibIntl REQUIRED)
	list (APPEND logdiag_LIBS ${LIBINTL_LIBRARIES})
endif (WIN32)

# Build the library
add_library (liblogdiag STATIC ${liblogdiag_SOURCES} ${liblogdiag_HEADERS})
set_target_properties (liblogdiag PROPERTIES OUTPUT_NAME logdiag)

# Build the executable
add_executable (logdiag WIN32 ${logdiag_SOURCES} ${logdiag_HEADERS})
target_link_libraries (logdiag liblogdiag ${logdiag_LIBS})

# Testing
if (BUILD_TESTING)
	enable_testing ()

	foreach (name ${logdiag_TESTS})
		add_executable (test-${name} tests/${name}.c)
		target_link_libraries (test-${name} liblogdiag ${logdiag_LIBS})

		add_test (test-${name} test-${name})
	endforeach (name)
endif (BUILD_TESTING)

# Generate documentation
if (GTKDOC_FOUND)
	# FIXME: not our bug but xml/gtkdocentities.ent cannot be passed
	target_link_libraries (liblogdiag ${logdiag_LIBS})
	gtk_doc_add_module (liblogdiag
		SOURCE ${PROJECT_SOURCE_DIR}/liblogdiag
		SUFFIXES c h
		# The "official" module wants an XML file I don't want to give it
		XML ${PROJECT_BINARY_DIR}/liblogdiag/liblogdiag-docs.xml
		LIBRARIES liblogdiag)
endif ()

find_package (CppCheck)
GENERATE_CPPCHECK (SOURCES liblogdiag src tests ENABLE_IDS all
	TARGET_NAME ${PROJECT_NAME}_cppcheck INCLUDES ${PROJECT_BINARY_DIR})

if (OPTION_USE_VERA)
	# Force this off, we don't need it
	option (VERA_LUA "" OFF)

	# Make it build within the same build tree
	get_directory_property (backup_includes INCLUDE_DIRECTORIES)
	set_directory_properties (PROPERTIES INCLUDE_DIRECTORIES "")
	set (backup_source_dir "${CMAKE_SOURCE_DIR}")
	set (CMAKE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/vera++")

	add_subdirectory ("vera++")

	set_directory_properties (PROPERTIES INCLUDE_DIRECTORIES "${backup_includes}")
	set (CMAKE_SOURCE_DIR "${backup_source_dir}")

	# Check the sources
	# XXX: maybe do it per source file and not all at once
	set (vera_srcs ${liblogdiag_SOURCES} ${logdiag_SOURCES} ${logdiag_HEADERS})
	set (vera_output ${PROJECT_BINARY_DIR}/vera.log)
	add_custom_command (OUTPUT ${vera_output}
		COMMAND vera
			-R F001 -R L001 -R T001 -R T004 -R T005 -R T006
			-R T007 -R T009 -R T010 -R T015 -R T017 -R T018
			-R L004 -P max-line-length=80
			-R L005 -P max-consecutive-empty-lines=2
			--root ${CMAKE_SOURCE_DIR}/vera++
			--std-report=-
			--std-report=${vera_output}
			--warning -s
			${vera_srcs}
		DEPENDS ${vera_srcs}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Checking style for logdiag")

	add_custom_target (logdiag_vera ALL DEPENDS ${vera_output})
endif (OPTION_USE_VERA)

# GSettings
find_program (GLIB_COMPILE_SCHEMAS_EXECUTABLE glib-compile-schemas)
if (NOT GLIB_COMPILE_SCHEMAS_EXECUTABLE)
	message (FATAL_ERROR "glib-compile-schemas not found")
endif (NOT GLIB_COMPILE_SCHEMAS_EXECUTABLE)

set (GSETTINGS_DIR "${PROJECT_SOURCE_DIR}/share")
set (GSETTINGS_SCHEMAS "${GSETTINGS_DIR}/org.logdiag.gschema.xml")

add_custom_target (${PROJECT_NAME}_gschema_check
	COMMAND ${GLIB_COMPILE_SCHEMAS_EXECUTABLE} --dry-run "${GSETTINGS_DIR}"
	DEPENDS "${GSETTINGS_SCHEMAS}"
	COMMENT "Checking GSettings schema files" VERBATIM)

if (OPTION_NOINSTALL)
	set (GSETTINGS_COMPILED_SCHEMA "${PROJECT_BINARY_DIR}/gschemas.compiled")

	add_custom_command (OUTPUT "${GSETTINGS_COMPILED_SCHEMA}"
		COMMAND ${GLIB_COMPILE_SCHEMAS_EXECUTABLE}
			"--targetdir=${PROJECT_BINARY_DIR}" "${GSETTINGS_DIR}"
		DEPENDS "${GSETTINGS_SCHEMAS}"
		COMMENT "Compiling GSettings schema files" VERBATIM)
	add_custom_target (${PROJECT_NAME}_gschema ALL
		DEPENDS "${GSETTINGS_COMPILED_SCHEMA}")
endif (OPTION_NOINSTALL)

# Installation
if (WIN32)
	install (TARGETS logdiag DESTINATION .)
	install (DIRECTORY
		${WIN32_DEPENDS_PATH}/bin/
		DESTINATION .
		FILES_MATCHING PATTERN "*.dll"
		PATTERN "libgettext*" EXCLUDE)
	install (DIRECTORY
		${WIN32_DEPENDS_PATH}/etc/
		DESTINATION etc)
	install (DIRECTORY
		${WIN32_DEPENDS_PATH}/lib/gdk-pixbuf-2.0
		DESTINATION lib)
	install (DIRECTORY
		${WIN32_DEPENDS_PATH}/share/glib-2.0/schemas
		DESTINATION share/glib-2.0
		FILES_MATCHING PATTERN "org.gtk.Settings.*")

	install (FILES
		${WIN32_DEPENDS_PATH}/share/icons/hicolor/index.theme
		DESTINATION share/icons/hicolor)
	install (DIRECTORY share/icons DESTINATION share)

	# Avoid partial translations
	file (GLOB locales ${WIN32_DEPENDS_PATH}/share/locale/*)
	foreach (locale ${locales})
		get_filename_component (translation ${locale} NAME)
		list (FIND project_TRANSLATIONS ${translation} translation_found)
		if (translation_found GREATER -1)
			install (DIRECTORY
				${WIN32_DEPENDS_PATH}/share/locale/${translation}
				DESTINATION share/locale)
		endif (translation_found GREATER -1)
	endforeach (locale)
else (WIN32)
	install (TARGETS logdiag DESTINATION bin)
	install (FILES share/logdiag.desktop DESTINATION share/applications)
	install (FILES share/logdiag.xml DESTINATION share/mime/packages)
	install (DIRECTORY share/icons DESTINATION share)
# Don't install the documentation yet as we don't install any library
#	install (DIRECTORY "${project_DOC_DIR}/html/"
#		DESTINATION share/gtk-doc/html/${PROJECT_NAME})
	install (FILES LICENSE NEWS DESTINATION share/doc/${PROJECT_NAME})
endif (WIN32)

install (FILES ${GSETTINGS_SCHEMAS}
	DESTINATION share/glib-2.0/schemas)
install (CODE " # DESTDIR is not in use on Windows
	if (WIN32 OR \"\$ENV{DESTDIR}\" STREQUAL \"\")
		execute_process (COMMAND ${GLIB_COMPILE_SCHEMAS_EXECUTABLE}
			\"\${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas\")
	endif (WIN32 OR \"\$ENV{DESTDIR}\" STREQUAL \"\")")

install (DIRECTORY share/gui share/library
	DESTINATION share/${PROJECT_NAME})

# CPack
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Schematic editor")
set (CPACK_PACKAGE_VENDOR "Premysl Eric Janouch")
set (CPACK_PACKAGE_CONTACT "Přemysl Eric Janouch <p@janouch.name>")
set (CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR ${project_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${project_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${project_VERSION_PATCH})

if (WIN32)
	set (CPACK_GENERATOR "NSIS;ZIP")
else (WIN32)
	set (CPACK_GENERATOR "TGZ;ZIP")
	set (CPACK_SET_DESTDIR TRUE)
endif (WIN32)

set (CPACK_PACKAGE_FILE_NAME
	"${PROJECT_NAME}-${project_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set (CPACK_PACKAGE_INSTALL_DIRECTORY
	"${PROJECT_NAME} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")

# FIXME: It is possible to install multiple copies. Everything screws up then.
set (CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/share\\\\header.bmp")
set (CPACK_NSIS_INSTALLED_ICON_NAME "logdiag.exe")
set (CPACK_NSIS_CREATE_ICONS_EXTRA "
	CreateShortCut '\$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\logdiag.lnk' '\$INSTDIR\\\\logdiag.exe'")
set (CPACK_NSIS_DELETE_ICONS_EXTRA "
	Delete '\$SMPROGRAMS\\\\$MUI_TEMP\\\\logdiag.lnk'")
set (CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
	WriteRegStr HKCR '.ldd' '' 'logdiag.Diagram'
	WriteRegStr HKCR 'logdiag.Diagram' '' 'logdiag Diagram'
	WriteRegStr HKCR 'logdiag.Diagram\\\\shell\\\\open\\\\command' '' '\\\"\$INSTDIR\\\\logdiag.exe\\\" \\\"%1\\\"'
	WriteRegStr HKCR 'logdiag.Diagram\\\\shell\\\\edit\\\\command' '' '\\\"\$INSTDIR\\\\logdiag.exe\\\" \\\"%1\\\"'
	System::Call 'shell32::SHChangeNotify(i,i,i,i) (0x08000000, 0x1000, 0, 0)'")
set (CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
	DeleteRegKey HKCR 'logdiag.Diagram'
	System::Call 'shell32::SHChangeNotify(i,i,i,i) (0x08000000, 0x1000, 0, 0)'")

string (REPLACE "https://" "https:\\\\\\\\" project_URL_NSIS "${project_URL}")
set (CPACK_NSIS_URL_INFO_ABOUT "${project_URL_NSIS}")

set (CPACK_DEBIAN_PACKAGE_DEPENDS
	"libgtk3.0-0 (>= 3.8), libjson-glib-1.0-0 (>= 0.10.4), liblua5.2-0")
set (CPACK_DEBIAN_PACKAGE_BUILDS_DEPENDS
	"libgtk3.0-dev (>= 3.8), libjson-glib-dev (>= 0.10.4), liblua5.2-0-dev")
set (CPACK_DEBIAN_PACKAGE_SECTION "Electronics")
set (CPACK_DEBIAN_PACKAGE_HOMEPAGE "${project_URL}")

set (CPACK_RPM_PACKAGE_REQUIRES "gtk3 >= 3.8, json-glib >= 0.10.4, lua = 5.2")
set (CPACK_RPM_PACKAGE_GROUP "Graphics")
set (CPACK_RPM_PACKAGE_LICENSE "ISC")
set (CPACK_RPM_PACKAGE_URL "${project_URL}")

set (CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set (CPACK_SOURCE_IGNORE_FILES "/\\\\.git;/build;/CMakeLists.txt.user;/win32-depends;/liblogdiag/ld-marshal.[ch]")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${project_VERSION}")

include (CPack)


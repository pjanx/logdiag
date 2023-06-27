# The last version with Windows XP support is 3.13, we want to keep that
cmake_minimum_required (VERSION 3.10)
project (logdiag VERSION 0.3.0 DESCRIPTION "Schematic editor" LANGUAGES C)

# Options
option (OPTION_CLANG_TIDY "Enable use of clang-tidy" OFF)
option (OPTION_NOINSTALL "For developers only--work without installing" OFF)
option (OPTION_GTKDOC "For developers only--enable use of gtk-doc" OFF)
option (BUILD_TESTING "Build tests" OFF)

# Project information
set (project_ID "name.janouch.${PROJECT_NAME}")
set (project_URL "https://git.janouch.name/p/logdiag")

# For custom modules
set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Automatic retrieval of dependencies
if (WIN32)
	set (WIN32_DEPENDS_PATH ${PROJECT_SOURCE_DIR}/win32-depends)
	list (APPEND CMAKE_PREFIX_PATH ${WIN32_DEPENDS_PATH})
	list (APPEND CMAKE_INCLUDE_PATH ${WIN32_DEPENDS_PATH}/lib)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mms-bitfields")

	if (CMAKE_CROSSCOMPILING)
		list (APPEND CMAKE_FIND_ROOT_PATH ${WIN32_DEPENDS_PATH})
	endif ()

	set (PKG_CONFIG_USE_CMAKE_PREFIX_PATH FALSE)
	# Seems like there is a regression setting this to two paths in one,
	# setting the path manually
	set (ENV{PKG_CONFIG_PATH} "${WIN32_DEPENDS_PATH}/lib/pkgconfig")
endif ()

# Generate icons
find_program (CONVERT_EXECUTABLE convert)
mark_as_advanced (CONVERT_EXECUTABLE)

if (CONVERT_EXECUTABLE AND NOT WIN32)
	set (icon_base ${PROJECT_SOURCE_DIR}/share/icons/hicolor)
	set (icon_svg ${icon_base}/scalable/apps/logdiag.svg)
	set (icon_size_list 16x16 32x32 48x48)

	# ImageMagick very oddly outperforms rsvg-convert with our specific SVG
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
	endforeach ()

	# Symbolic targets prevent the icons from being either cleaned,
	# or pointlessly regenerated slightly differently
	set (icon_ico ${PROJECT_SOURCE_DIR}/share/logdiag.ico)
	add_custom_target (${PROJECT_NAME}_icons
		DEPENDS ${icon_symbolic_list}
		COMMAND ${CONVERT_EXECUTABLE} ${icon_png_list} ${icon_ico}
		COMMENT "Generating logdiag.ico" VERBATIM)
endif ()

# Dependencies
find_package (PkgConfig REQUIRED)
pkg_check_modules (GTK3 REQUIRED gtk+-3.0 json-glib-1.0)
link_directories (${GTK3_LIBRARY_DIRS})

if (NOT WIN32)
	pkg_search_module (Lua REQUIRED lua>=5.2 lua5.3 lua-5.3 lua5.2 lua-5.2)
	link_directories (${Lua_LIBRARY_DIRS})
else ()
	# For whatever reason this now seems to be required
	set (LUA_INCLUDE_DIR "${WIN32_DEPENDS_PATH}/include")

	find_package (Lua REQUIRED)
	if (LUA_VERSION_STRING VERSION_LESS "5.2")
		message (FATAL_ERROR "Lua 5.2 not found")
	endif ()
	set (Lua_LIBRARIES ${LUA_LIBRARIES})
	set (Lua_INCLUDE_DIRS ${LUA_INCLUDE_DIR})
endif ()

# Localization
find_package (Gettext REQUIRED)
file (GLOB project_PO_FILES ${PROJECT_SOURCE_DIR}/po/*.po)
GETTEXT_CREATE_TRANSLATIONS (
	${PROJECT_SOURCE_DIR}/po/${PROJECT_NAME}.pot
	ALL ${project_PO_FILES})

foreach (file ${project_PO_FILES})
	get_filename_component (translation "${file}" NAME_WE)
	list (APPEND project_TRANSLATIONS "${translation}")
endforeach ()

# Project source files
set (liblogdiag_SOURCES
	${PROJECT_BINARY_DIR}/ld-marshal.c
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
	${PROJECT_BINARY_DIR}/ld-marshal.h
	${PROJECT_BINARY_DIR}/config.h
	liblogdiag/liblogdiag.h
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
	${PROJECT_BINARY_DIR}/gresource.c
	src/ld-window-main.c
	src/logdiag.c)
set (logdiag_HEADERS
	${liblogdiag_HEADERS}
	src/ld-window-main.h)

# Resource compilation for Windows
if (WIN32)
	# The file isn't scanned for dependencies, but they will only rarely change
	list (APPEND logdiag_SOURCES share/logdiag.rc)
endif ()

# Generate marshallers
function (glib_genmarshal listfile prefix)
	find_program (GLIB_GENMARSHAL_EXECUTABLE glib-genmarshal)
	if (NOT GLIB_GENMARSHAL_EXECUTABLE)
		message (FATAL_ERROR "glib-genmarshal not found")
	endif ()

	get_filename_component (_basename "${listfile}" NAME_WE)
	set (_command_base ${GLIB_GENMARSHAL_EXECUTABLE} --prefix ${prefix}
		"${listfile}")

	add_custom_command (OUTPUT ${_basename}.c ${_basename}.h
		COMMAND ${_command_base} --body   > ${_basename}.c
		COMMAND ${_command_base} --header > ${_basename}.h
		DEPENDS "${listfile}"
		COMMENT "Generating marshallers source files" VERBATIM)
endfunction ()

glib_genmarshal (${PROJECT_SOURCE_DIR}/liblogdiag/ld-marshal.list ld_marshal)

# Generate resources
function (glib_compile_resources xml target)
	find_program (GLIB_COMPILE_RESOURCES_EXECUTABLE glib-compile-resources)
	if (NOT GLIB_COMPILE_RESOURCES_EXECUTABLE)
		message (FATAL_ERROR "glib-compile-resources not found")
	endif ()

	# glib-compile-resources reads paths relative to its working directory
	get_filename_component (_xml_path "${xml}" PATH)

	# On Windows, the final Makefile may contain:
	#   cd e:\abc && ...
	# That won't actually change the directory. We have to do:
	#   cd e:\abc && e: && ...
	set (_prefix)
	if (WIN32 AND "${_xml_path}" MATCHES "^.:[/\\\\]")
		string (SUBSTRING "${_xml_path}" 0 2 _xml_drive)
		set (_prefix ${_xml_drive} &&)
	endif ()

	# VERBATIM would cause problems, so it is not used here
	add_custom_command (OUTPUT ${target}
		COMMAND ${_prefix} ${GLIB_COMPILE_RESOURCES_EXECUTABLE}
		--generate-source --target ${target} "${xml}" DEPENDS "${xml}"
		WORKING_DIRECTORY "${_xml_path}" COMMENT "Compiling resources")

	execute_process (COMMAND ${GLIB_COMPILE_RESOURCES_EXECUTABLE}
		--generate-dependencies "${xml}"
		WORKING_DIRECTORY "${_xml_path}" OUTPUT_VARIABLE _deps)
	string (REPLACE "\n" ";" _deps "${_deps}")
	foreach (_dep ${_deps})
		set_property (SOURCE "${target}"
			APPEND PROPERTY OBJECT_DEPENDS "${_xml_path}/${_dep}")
	endforeach ()

	configure_file ("${xml}" glib-compile-resources.stamp COPYONLY)
endfunction ()

glib_compile_resources (${PROJECT_SOURCE_DIR}/share/logdiag.gresource.xml
	${PROJECT_BINARY_DIR}/gresource.c)

# Generate a configure file
add_definitions (-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_36)
add_definitions (-DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_36)
configure_file (config.h.in ${PROJECT_BINARY_DIR}/config.h)

include_directories (${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
	${GTK3_INCLUDE_DIRS} ${Lua_INCLUDE_DIRS})
set (logdiag_LIBS ${GTK3_LIBRARIES} ${Lua_LIBRARIES} m)

if (WIN32)
	find_package (Intl REQUIRED)
	list (APPEND logdiag_LIBS ${Intl_LIBRARIES} shell32)
endif ()

# Build the library
add_library (liblogdiag STATIC ${liblogdiag_SOURCES} ${liblogdiag_HEADERS})
set_target_properties (liblogdiag PROPERTIES OUTPUT_NAME logdiag)

# Build the executable
add_executable (logdiag WIN32 ${logdiag_SOURCES} ${logdiag_HEADERS})
target_link_libraries (logdiag liblogdiag ${logdiag_LIBS})

# GSettings
find_program (GLIB_COMPILE_SCHEMAS_EXECUTABLE glib-compile-schemas)
if (NOT GLIB_COMPILE_SCHEMAS_EXECUTABLE)
	message (FATAL_ERROR "glib-compile-schemas not found")
endif ()

set (GSETTINGS_DIR "${PROJECT_SOURCE_DIR}/share")
set (GSETTINGS_SCHEMAS "${GSETTINGS_DIR}/${PROJECT_NAME}.gschema.xml")

if (OPTION_NOINSTALL)
	set (GSETTINGS_COMPILED_SCHEMA "${PROJECT_BINARY_DIR}/gschemas.compiled")
	add_custom_command (OUTPUT "${GSETTINGS_COMPILED_SCHEMA}"
		COMMAND ${GLIB_COMPILE_SCHEMAS_EXECUTABLE}
			"--targetdir=${PROJECT_BINARY_DIR}" "${GSETTINGS_DIR}"
		DEPENDS "${GSETTINGS_SCHEMAS}"
		COMMENT "Compiling GSettings schema files" VERBATIM)
	add_custom_target (${PROJECT_NAME}-gschema ALL
		DEPENDS "${GSETTINGS_COMPILED_SCHEMA}")
endif ()

# Testing
if (BUILD_TESTING)
	enable_testing ()

	add_test (NAME test-gschema-${PROJECT_NAME}
		COMMAND ${GLIB_COMPILE_SCHEMAS_EXECUTABLE} --dry-run "${GSETTINGS_DIR}")

	foreach (name ${logdiag_TESTS})
		add_executable (test-${name} tests/${name}.c)
		target_link_libraries (test-${name} liblogdiag ${logdiag_LIBS})
		add_test (NAME test-${name} COMMAND test-${name})
		list (APPEND logdiag_TEST_TARGETS test-${name})
	endforeach ()
	if (WIN32 AND NOT CMAKE_CROSSCOMPILING)
		set_tests_properties (${logdiag_TEST_TARGETS}
			PROPERTIES ENVIRONMENT "PATH=${WIN32_DEPENDS_PATH}/bin")
	endif ()
endif ()

# Generate documentation
if (OPTION_GTKDOC)
	find_package (GtkDoc 1.25 REQUIRED)
	set (project_DOC_DIR "${PROJECT_BINARY_DIR}/liblogdiag")

	# Extracted LDFLAGS didn't contain -l in CMake 3.21.3 and gtk-doc 1.33.2,
	# pass them explicitly and work around insanity; CMake still exits with
	# an error and gtk-doc produces empty HTML documentation
	string (REGEX REPLACE "(;)([^-])" "\\1-l\\2" xldflags "${logdiag_LIBS}")
	set_target_properties (liblogdiag PROPERTIES LINK_LIBRARIES "")

	# The "official" module wants an XML file I don't want to give it
	file (WRITE ${PROJECT_BINARY_DIR}/liblogdiag-docs-dummy.xml "<book/>")

	# XXX: not our bug but xml/gtkdocentities.ent cannot be passed
	# XXX: this causes `make clean` to remove the entire build directory,
	# because it marks CMAKE_CURRENT_BINARY_DIR as an output
	gtk_doc_add_module (liblogdiag
		SOURCE ${PROJECT_SOURCE_DIR}/liblogdiag
		SUFFIXES c h LDFLAGS "-l${xldflags}"
		XML ${PROJECT_BINARY_DIR}/liblogdiag-docs-dummy.xml
		LIBRARIES liblogdiag)
endif ()

find_package (CppCheck)
GENERATE_CPPCHECK (SOURCES liblogdiag src tests ENABLE_IDS all
	TARGET_NAME ${PROJECT_NAME}_cppcheck INCLUDES ${PROJECT_BINARY_DIR})

# Various clang-based diagnostics, loads of fake positives and spam
if (OPTION_CLANG_TIDY)
	set (clang_tidy_checks
		clang-analyzer-* bugprone-* misc-* readability-* performance-*
		-bugprone-reserved-identifier          # GObject
		-performance-no-int-to-ptr             # GObject
		-bugprone-narrowing-conversions
		-bugprone-macro-parentheses            # too coarse-grained
		-readability-braces-around-statements  # fine by me
		-readability-isolate-declaration       # fine by me
		-readability-magic-numbers             # too coarse-grained
		-misc-unused-parameters)               # fine by me

	string (REPLACE ";" "," clang_tidy_checks "${clang_tidy_checks}")
	set (clang_tidy_config "{
		Checks: '${clang_tidy_checks}',
		CheckOptions: [{
			key: readability-function-cognitive-complexity.Threshold,
			value: '30'
		}]}")

	string (REPLACE "\n" " " clang_tidy_config "${clang_tidy_config}")

	set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
	set (clang_tidy_sources ${liblogdiag_SOURCES} ${logdiag_SOURCES})
	add_custom_target (${PROJECT_NAME}_clang_tidy
		COMMAND clang-tidy --quiet -p ${PROJECT_BINARY_DIR}
			"--config=${clang_tidy_config}" ${clang_tidy_sources}
			| sh -c "cat 1>&2"
		DEPENDS ${clang_tidy_sources} VERBATIM
		USES_TERMINAL WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif ()

# Installation
if (WIN32)
	install (TARGETS logdiag DESTINATION .)
	install (DIRECTORY
		${WIN32_DEPENDS_PATH}/bin/
		DESTINATION .
		FILES_MATCHING PATTERN "*.dll" PATTERN "gspawn-*-helper.exe")
	install (DIRECTORY
		${WIN32_DEPENDS_PATH}/etc/
		DESTINATION etc)
	install (DIRECTORY
		${WIN32_DEPENDS_PATH}/lib/gdk-pixbuf-2.0
		DESTINATION lib
		FILES_MATCHING PATTERN "*" PATTERN "*.a" EXCLUDE)
	install (DIRECTORY
		${WIN32_DEPENDS_PATH}/share/glib-2.0/schemas
		DESTINATION share/glib-2.0
		FILES_MATCHING PATTERN "org.gtk.Settings.*")

	install (DIRECTORY
		${WIN32_DEPENDS_PATH}/share/icons/Adwaita
		DESTINATION share/icons OPTIONAL)
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
		endif ()
	endforeach ()

	install (SCRIPT Win32Cleanup.cmake)
else ()
	install (TARGETS logdiag DESTINATION bin)
	install (FILES share/logdiag.desktop DESTINATION share/applications)
	install (FILES share/logdiag.xml DESTINATION share/mime/packages)
	install (DIRECTORY share/icons DESTINATION share)
# Don't install the documentation yet as we don't install any library
#	install (DIRECTORY "${project_DOC_DIR}/html/"
#		DESTINATION share/gtk-doc/html/${PROJECT_NAME})
	install (FILES LICENSE NEWS DESTINATION share/doc/${PROJECT_NAME})
endif ()

install (DIRECTORY docs/user-guide DESTINATION share/doc/${PROJECT_NAME})
install (DIRECTORY share/library DESTINATION share/${PROJECT_NAME})
install (FILES ${GSETTINGS_SCHEMAS} DESTINATION share/glib-2.0/schemas
	RENAME "${project_ID}.gschema.xml")
install (CODE " # DESTDIR is not in use on Windows (WIN32 is only native here!)
	if (WIN32 OR \"\$ENV{DESTDIR}\" STREQUAL \"\")
		execute_process (COMMAND \"${GLIB_COMPILE_SCHEMAS_EXECUTABLE}\"
			\"\${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas\")
	endif ()")

# CPack
set (CPACK_PACKAGE_VENDOR "Premysl Eric Janouch")
set (CPACK_PACKAGE_CONTACT "Přemysl Eric Janouch <p@janouch.name>")
set (CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

if (WIN32)
	set (CPACK_GENERATOR "NSIS;ZIP")
else ()
	set (CPACK_GENERATOR "TGZ;ZIP")
	set (CPACK_SET_DESTDIR TRUE)
endif ()

set (CPACK_PACKAGE_FILE_NAME
	"${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set (CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME} ${CPACK_PACKAGE_VERSION}")

# XXX: It is still possible to install multiple copies, making commands collide.
set (CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set (CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${PROJECT_NAME}")

set (CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/share\\\\header.bmp")
set (CPACK_NSIS_INSTALLED_ICON_NAME "logdiag.exe")
set (CPACK_NSIS_URL_INFO_ABOUT "${project_URL}")
set (CPACK_NSIS_HELP_LINK "${project_URL}")
# Or CPACK_NSIS_EXECUTABLES_DIRECTORY needs to be changed from "bin"
set (CPACK_NSIS_MENU_LINKS "logdiag.exe" ${PROJECT_NAME})
set (CPACK_NSIS_EXTRA_INSTALL_COMMANDS [[
	WriteRegStr HKCR '.ldd' '' 'logdiag.Diagram'
	WriteRegStr HKCR 'logdiag.Diagram' '' 'logdiag Diagram'
	WriteRegStr HKCR 'logdiag.Diagram\\shell\\open\\command' '' '\"$INSTDIR\\logdiag.exe\" \"%1\"'
	WriteRegStr HKCR 'logdiag.Diagram\\shell\\edit\\command' '' '\"$INSTDIR\\logdiag.exe\" \"%1\"'
	System::Call 'shell32::SHChangeNotify(i,i,i,i) (0x08000000, 0x1000, 0, 0)'
]])
set (CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS [[
	DeleteRegKey HKCR 'logdiag.Diagram'
	System::Call 'shell32::SHChangeNotify(i,i,i,i) (0x08000000, 0x1000, 0, 0)'
]])

set (CPACK_DEBIAN_PACKAGE_DEPENDS
	"libgtk3.0-0 (>= 3.8), libjson-glib-1.0-0 (>= 0.10.4), liblua5.2-0")
set (CPACK_DEBIAN_PACKAGE_BUILDS_DEPENDS
	"libgtk3.0-dev (>= 3.8), libjson-glib-dev (>= 0.10.4), liblua5.2-0-dev")
set (CPACK_DEBIAN_PACKAGE_SECTION "Electronics")
set (CPACK_DEBIAN_PACKAGE_HOMEPAGE "${project_URL}")

set (CPACK_RPM_PACKAGE_REQUIRES "gtk3 >= 3.8, json-glib >= 0.10.4, lua = 5.2")
set (CPACK_RPM_PACKAGE_GROUP "Graphics")
set (CPACK_RPM_PACKAGE_LICENSE "0BSD")
set (CPACK_RPM_PACKAGE_URL "${project_URL}")

set (CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set (CPACK_SOURCE_IGNORE_FILES "/\\\\.git;/build;/CMakeLists.txt.user;/win32-depends")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

include (CPack)

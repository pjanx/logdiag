# - Finding gtk-doc and building documentation
# This module provides the following function:
#
# GTK_DOC_RUN (
#   [ALL]
#   [MODULE <module-name>]
#   [WORKING_DIR <working-dir>]
#   SOURCE_DIRS <source-dir> ...
#   [IGNORE_FILES <file> ...]
#   [SCANGOBJ <library> [<link-libraries> ...]]
#   [{SGML | XML} [<mkdb-output-dir> [<mkdb-options>]]
#   [HTML <html-output-dir> <backend-options>]]
# )
#
# The function creates a target named <module-name>_gtkdocize
# which will build the documentation as specified by parameters.
#
#   ALL          - always build the target
#   MODULE       - the name of the module
#                  CMAKE_PROJECT_NAME by default
#   WORKING_DIR  - path to the working directory
#                  CMAKE_CURRENT_BINARY_DIR by default
#   SOURCE_DIRS  - documentation sources
#   IGNORE_FILES - ignore these files in the process
#   SCANGOBJ     - build an object hierarchy scanner
#   SGML         - make SGML output in the spec. directory
#   XML          - make XML output in the spec. directory
#   HTML         - make HTML output in the spec. directory
#                  (requires either SGML or XML)
#
# Also creates these virtual symbolic outputs if appropriate:
#   <module-name>_gtkdocize_scan
#   <module-name>_gtkdocize_scan_rebuild_types
#   <module-name>_gtkdocize_scan_rebuild_sections
#   <module-name>_gtkdocize_scan_gobject
#   <module-name>_gtkdocize_mkdb
#   <module-name>_gtkdocize_mkhtml
#
#

#=============================================================================
# Copyright Přemysl Janouch 2010 - 2011
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#=============================================================================

# TODO
# ====
#  - Since it doesn't work without the full Unix environment,
#    it might be actually proper to use pkg-config
#
#  - <module-name>_gtkdocize_fixxref
#  - gtkdoc-rebase
#  - Content files (included by the main SGML file)
#


find_program (GTK_DOC_SCAN_EXECUTABLE "gtkdoc-scan")
find_program (GTK_DOC_SCANGOBJ_EXECUTABLE "gtkdoc-scangobj")
find_program (GTK_DOC_MKTMPL_EXECUTABLE "gtkdoc-mktmpl")
find_program (GTK_DOC_MKDB_EXECUTABLE "gtkdoc-mkdb")
find_program (GTK_DOC_MKHTML_EXECUTABLE "gtkdoc-mkhtml")
find_program (GTK_DOC_FIXXREF_EXECUTABLE "gtkdoc-fixxref")

mark_as_advanced (GTK_DOC_SCAN_EXECUTABLE GTK_DOC_SCANGOBJ_EXECUTABLE
	GTK_DOC_MKTMPL_EXECUTABLE GTK_DOC_MKDB_EXECUTABLE
	GTK_DOC_MKHTML_EXECUTABLE GTK_DOC_FIXXREF_EXECUTABLE)

include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (GTK_DOC DEFAULT_MSG GTK_DOC_SCAN_EXECUTABLE)

include (ProcessArguments)

function (GTK_DOC_RUN)
	# Parse arguments given to this function
	set (__names  ALL  MODULE  WORKING_DIR  SOURCE_DIRS  IGNORE_FILES)
	set (__need   NO   NO      NO           YES          NO)
	set (__want   0    1       1            1            1)
	set (__more   NO   NO      NO           YES          YES)
	set (__skip   0    0       0            0            0)

	list (APPEND __names  SCANGOBJ  SGML  XML  HTML)
	list (APPEND __need   NO        NO    NO   NO)
	list (APPEND __want   1         0     0    1)
	list (APPEND __more   YES       YES   YES  YES)
	list (APPEND __skip   0         1     0    0)

	set (__argv ${ARGV})
	PROCESS_ARGUMENTS (__argv __names __need __want __more __skip "_opt_")

	# Further process the arguments
	if (_opt_all)
		set (_all ALL)
	else (_opt_all)
		set (_all)
	endif (_opt_all)

	if (_opt_module)
		set (_module_name ${_opt_module_param})
	else (_opt_module)
		set (_module_name ${CMAKE_PROJECT_NAME})
	endif (_opt_module)

	if (_opt_working_dir)
		set (_working_dir ${_opt_working_dir_param})
	else (_opt_working_dir)
		set (_working_dir ${CMAKE_CURRENT_BINARY_DIR})
	endif (_opt_working_dir)

	set (_source_dirs)
	foreach (_dir ${_opt_source_dirs_param})
		list (APPEND _source_dirs "--source-dir" "${_dir}")
	endforeach (_dir)

	set (_ignores)
	if (_opt_ignore_files)
		foreach (_file ${_opt_ignore_files_param})
			set (_ignores "${_ignores} ${_file}")
		endforeach (_file)
		string (STRIP "${_ignores}" _ignores)
	endif (_opt_ignore_files)

	if (_opt_sgml)
		set (_mkdb_format "sgml")
		set (_mkdb_options "${_opt_sgml_param}")
	elseif (_opt_xml)
		set (_mkdb_format "xml")
		set (_mkdb_options "${_opt_xml_param}")
	else (_opt_sgml)
		set (_mkdb_format OFF)
	endif (_opt_sgml)

	if (_mkdb_format)
		set (_mkdb_driver ${_working_dir}/${_module_name}-docs.${_mkdb_format})
		list (LENGTH _mkdb_options _length)
		if (${_length} GREATER 0)
			list (GET _mkdb_options 0 _mkdb_output_dir)
			list (REMOVE_AT _mkdb_options 0)
		else (${_length} GREATER 0)
			set (_mkdb_output_dir ${_working_dir}/${_mkdb_format})
		endif (${_length} GREATER 0)
	endif (_mkdb_format)

	# The basic target name
	set (_target_name ${_module_name}_gtkdocize)

	# Scan the source files
	set (_scan_target_base
	# These files are created if they don't exist
	#	        ${_working_dir}/${_module_name}.types
	#	        ${_working_dir}/${_module_name}-sections.txt
	#	        ${_working_dir}/${_module_name}-overrides.txt
		        ${_working_dir}/${_module_name}-decl.txt
		        ${_working_dir}/${_module_name}-decl-list.txt
		COMMAND ${GTK_DOC_SCAN_EXECUTABLE}
		        --module=${_module_name}
		        ${_source_dirs} "--ignore-headers=${_ignores}"
		        --output-dir=${_working_dir})
	add_custom_command (
		OUTPUT ${_target_name}_scan
		${_scan_target_base}
		COMMENT "Calling gtkdoc-scan" VERBATIM)

	# Special target to force rebuild of ${_module_name}.types
	add_custom_command (
		OUTPUT ${_target_name}_scan_rebuild_types
		${_scan_target_base} --rebuild-types
		COMMENT "Calling gtkdoc-scan to rebuild types" VERBATIM)
	add_custom_target (${_target_name}_rebuild_types
		DEPENDS ${_target_name}_scan_rebuild_types)

	# Special target to force rebuild of ${_module_name}-sections.txt
	add_custom_command (
		OUTPUT ${_target_name}_scan_rebuild_sections
		${_scan_target_base} --rebuild-sections
		COMMENT "Calling gtkdoc-scan to rebuild sections" VERBATIM)
	add_custom_target (${_target_name}_rebuild_sections
		DEPENDS ${_target_name}_scan_rebuild_sections)

	set_source_files_properties (
		${_target_name}_scan
		${_target_name}_scan_rebuild_types
		${_target_name}_scan_rebuild_sections
		PROPERTIES SYMBOLIC TRUE)
	set (_top_output ${_target_name}_scan)

	# Scan the object hierarchy
	# This is a terrible hack, but there's no other way around.
	if (_opt_scangobj)
		# Put all include directories into CFLAGS
		set (_cflags)
		get_directory_property (_include_dirs INCLUDE_DIRECTORIES)
		foreach (_dir ${_include_dirs})
			set (_cflags "${_cflags} -I${_dir}")
		endforeach (_dir)

		# Put all libraries to LDFLAGS
		set (_ldflags "-L${CMAKE_CURRENT_BINARY_DIR}")
		set (_lib_depends)
		set (_lib_dir_used)
		foreach (_lib ${_opt_scangobj_param} ${CMAKE_STANDARD_LIBRARIES})
			get_filename_component (_lib_dir ${_lib} PATH)
			get_filename_component (_lib_name ${_lib} NAME)

			if (TARGET ${_lib_name})
				get_target_property (_lib_output_name ${_lib_name} OUTPUT_NAME)
				if (_lib_output_name)
					set (_lib_name ${_lib_output_name})
				endif (_lib_output_name)
				list (APPEND _lib_depends ${_lib_name})
			else (TARGET ${_lib_name})
				list (FIND _lib_dir_used "${_lib_dir}" _lib_dir_found)
				if (_lib_dir AND _lib_dir_found EQUAL "-1")
					set (_ldflags "${_ldflags} -L${_lib_dir}")
					list (APPEND _lib_dir_used ${_lib_dir})
				endif (_lib_dir AND _lib_dir_found EQUAL "-1")

				string (REGEX REPLACE "^${CMAKE_SHARED_LIBRARY_PREFIX}" ""
					_lib_name "${_lib_name}")
				string (REGEX REPLACE "${CMAKE_SHARED_LIBRARY_SUFFIX}\$" ""
					_lib_name "${_lib_name}")
			endif (TARGET ${_lib_name})

			set (_ldflags "${_ldflags} -l${_lib_name}")
		endforeach (_lib)

		add_custom_command (
			OUTPUT ${_target_name}_scan_gobject
			       ${_working_dir}/${_module_name}.signals
			       ${_working_dir}/${_module_name}.hierarchy
			       ${_working_dir}/${_module_name}.interfaces
			       ${_working_dir}/${_module_name}.prerequisites
			       ${_working_dir}/${_module_name}.args
			COMMAND "CC=${CMAKE_C_COMPILER}" "CFLAGS=${_cflags}"
			        "LD=${CMAKE_C_COMPILER}" "LDFLAGS=${_ldflags}"
			        "RUN=" ${GTK_DOC_SCANGOBJ_EXECUTABLE}
			        --module=${_module_name} --output-dir=${_working_dir}
			DEPENDS ${_top_output} ${_lib_depends}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Calling gtkdoc-scangobj")

		set_source_files_properties (${_target_name}_scan_gobject
			PROPERTIES SYMBOLIC TRUE)
		set (_top_output ${_target_name}_scan_gobject)
	endif (_opt_scangobj)

	# Create XML or SGML files
	if (_mkdb_format)
		add_custom_command (
			OUTPUT ${_target_name}_mkdb
			       ${_mkdb_output_dir}
			       ${_working_dir}/sgml.stamp
			       ${_working_dir}/${_module_name}-undeclared.txt
			       ${_working_dir}/${_module_name}-undocumented.txt
			       ${_working_dir}/${_module_name}-unused.txt
		# --outputallsymbols --outputsymbolswithoutsince
		#	       ${_working_dir}/${_module_name}-symbols.txt
		#	       ${_working_dir}/${_module_name}-nosince.txt
			COMMAND ${CMAKE_COMMAND} -E remove_directory ${_mkdb_output_dir}
			COMMAND ${GTK_DOC_MKDB_EXECUTABLE}
			        --module=${_module_name}
			        ${_source_dirs} "--ignore-files=${_ignores}"
			        --output-format=${_mkdb_format}
			        --output-dir=${_mkdb_output_dir}
			        ${_mkdb_options} --main-sgml-file=${_mkdb_driver}
			DEPENDS ${_top_output}
			WORKING_DIRECTORY ${_working_dir}
			COMMENT "Calling gtkdoc-mkdb" VERBATIM)

		set_source_files_properties (${_target_name}_mkdb
			PROPERTIES SYMBOLIC TRUE)
		set (_top_output ${_target_name}_mkdb)
	endif (_mkdb_format)

	# Create HTML documentation
	if (_opt_html)
		if (NOT _mkdb_format)
			message (FATAL_ERROR "Given HTML but neither XML nor SGML")
		endif (NOT _mkdb_format)

		list (GET _opt_html_param 0 _html_output_dir)
		list (REMOVE_AT _opt_html_param 0)

		add_custom_command (
			OUTPUT ${_target_name}_mkhtml
			       ${_html_output_dir}/../html.stamp
		# We probably don't want this to be removed either
		#	       ${_html_output_dir}
			COMMAND ${CMAKE_COMMAND} -E remove_directory ${_html_output_dir}
			COMMAND ${CMAKE_COMMAND} -E make_directory ${_html_output_dir}
			COMMAND ${CMAKE_COMMAND} -E chdir ${_html_output_dir}
			        ${GTK_DOC_MKHTML_EXECUTABLE}
			        ${_module_name} ${_mkdb_driver} ${_opt_html_param}
			DEPENDS ${_top_output}
			COMMENT "Calling gtkdoc-mkhtml" VERBATIM)

		set_source_files_properties (${_target_name}_mkhtml
			PROPERTIES SYMBOLIC TRUE)
		set (_top_output ${_target_name}_mkhtml)
	endif (_opt_html)

	# gtkdoc-fixxref
	#   ? copy ${_html_output_dir} to CMAKE_BINARY_DIR,
	#     run gtkdoc-fixxref in there and install the directory
	#      -> FIXXREF <output-dir> [INSTALL]
	#
	#? set (_fixxref_dir ${CMAKE_CURRENT_BINARY_DIR}/html-fixxref)
	# add_custom_command (
	#	OUTPUT ${_target_name}_fixxref
	#?	       ${_fixxref_dir}
	#?	COMMAND ${CMAKE_COMMAND} -E remove_directory ${_fixxref_dir}
	#?	COMMAND ${CMAKE_COMMAND} -E copy_directory
	#?	        ${_html_output_dir} ${_fixxref_dir}
	#	COMMAND ${GTK_DOC_FIXXREF_EXECUTABLE}
	#	        --module=${_module_name}
	#	        --module-dir=${_html_output_dir}
	#?	        --module-dir=${_fixxref_dir}
	#	DEPENDS ${_html_output_dir}
	#	WORKING_DIRECTORY ${_working_dir}
	#	COMMENT "Calling gtkdoc-fixxref" VERBATIM)
	#? install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html-fixxref/
	#?          DESTINATION share/gtk-doc/html/${_module_name})
	#&          COMPONENT docs) -- see CPack component install

	# gtkdoc-rebase -- how to do this?
	#
	# Probably omit this because source tarball cannot be hooked
	# to do some action before.
	#  -> Custom dist target?
	#     add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
	#  -> ${_html_output_dir} could contain online links by default,
	#     then it could be copied to binary dir and rebased to relative
	#       * Looks like a very good idea, can work even without gtk-doc
	#  -> Or it can be first xreffed in the binary dir and then
	#     converted to online links in ${_html_output_dir}
	#       * Which one of those should be installed?
	#         The one in the binary directory or should the
	#         ${_html_output_dir} be rebased?
	#       * A rebasing custom command may create the binary directory
	#         if it doesn't exist
	#
	# When creating the source tarball for distribution,
	# gtkdoc-rebase turns all external links into web-links.
	# When installing distributed (pregenerated) docs the same
	# application will try to turn links back to local links
	# (where those docs are installed).

	add_custom_target (${_target_name} ${_all}
		DEPENDS ${_top_output})
endfunction (GTK_DOC_RUN)



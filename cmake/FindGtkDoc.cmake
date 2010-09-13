# - Finding gtk-doc and building documentation
# This module provides the following function:
#
# GTK_DOC_RUN (
#   [ALL]
#   [MODULE <module-name>]
#   [WORKING_DIR <working-dir>]
#   SOURCE_DIRS <source-dir> ...
#   [IGNORE_FILES <file> ...]
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
#   SGML         - make SGML output in the spec. directory
#   XML          - make XML output in the spec. directory
#   HTML         - make HTML output in the spec. directory
#                  (requires either SGML or XML)
#
# Also creates these virtual symbolic outputs if appropriate:
#   <module-name>_gtkdocize_scan
#   <module-name>_gtkdocize_scan_rebuild_types
#   <module-name>_gtkdocize_scan_rebuild_sections
#   <module-name>_gtkdocize_mkdb
#   <module-name>_gtkdocize_mkhtml
#
#

#=============================================================================
# Copyright Přemysl Janouch 2010
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

# Useful resources
# ================
#
# /usr/share/cmake-2.8/Modules/readme.txt
#
# Autotools stack
#  - /usr/share/aclocal/gtk-doc.m4
#  - /usr/share/gtk-doc/data/gtk-doc{.notmpl}.make
#
# gtk+-2.0, glib-2.0, totem, ... packages -- docs subdir etc.
#
# Python wrapper -- useful resource
#  - http://bjourne.webfactional.com/browser/gtkimageview/gtkdoc.py
#
# Overview of the process
#  - http://library.gnome.org
#         /devel/gtk-doc-manual/stable/howdoesgtkdocwork.html.en
#
# Documenting
#  - http://live.gnome.org/DocumentationProject/GtkDoc
#  - http://library.gnome.org
#         /devel/gtk-doc-manual/stable/documenting_syntax.html.en
#  - http://library.gnome.org
#         /devel/gtk-doc-manual/stable/metafiles.html.en
#
# TODO
# ====
#  - Since it doesn't work without the full Unix environment,
#    it might be actually proper to use pkg-config
#
#  - <module-name>_gtkdocize_scangobj
#  - <module-name>_gtkdocize_mktmpl
#  - <module-name>_gtkdocize_fixxref
#  - Content files (included by the main SGML file)
#


find_program (GTK_DOC_SCAN_EXECUTABLE "gtkdoc-scan")
find_program (GTK_DOC_SCANGOBJ_EXECUTABLE "gtkdoc-scangobj")
find_program (GTK_DOC_MKTMPL_EXECUTABLE "gtkdoc-mktmpl")
find_program (GTK_DOC_MKDB_EXECUTABLE "gtkdoc-mkdb")
find_program (GTK_DOC_MKHTML_EXECUTABLE "gtkdoc-mkhtml")
find_program (GTK_DOC_FIXXREF_EXECUTABLE "gtkdoc-fixxref")

set (GTK_DOC_FOUND TRUE)
if (NOT GTK_DOC_SCAN_EXECUTABLE)
	set (GTK_DOC_FOUND FALSE)
endif (NOT GTK_DOC_SCAN_EXECUTABLE)

if (GtkDoc_FIND_REQUIRED AND NOT GTK_DOC_FOUND)
	message (FATAL_ERROR "gtk-doc NOT found")
endif (GtkDoc_FIND_REQUIRED AND NOT GTK_DOC_FOUND)


include (ProcessArguments)

function (GTK_DOC_RUN)
	# Parse arguments given to this function
	set (__names  ALL  MODULE  WORKING_DIR)
	set (__need   NO   NO      NO)
	set (__want   0    1       1)
	set (__more   NO   NO      NO)
	set (__skip   0    0       0)

	list (APPEND __names  SOURCE_DIRS  IGNORE_FILES  SGML  XML  HTML)
	list (APPEND __need   YES          NO            NO    NO   NO)
	list (APPEND __want   1            1             0     0    1)
	list (APPEND __more   YES          YES           YES   YES  YES)
	list (APPEND __skip   0            0             1     0    0)

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
	endforeach (_dir ${_opt_source_dirs_param})

	set (_ignores)
	if (_opt_ignore_files)
		foreach (_file ${_opt_ignore_files_param})
			set (_ignores "${_ignores} ${_file}")
		endforeach (_file ${_opt_ignore_files_param})
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

	# gtkdoc-scangobj
	#
	# gtkdoc-scangobj builds and runs an inspection program. You must
	# tell it how to do that by running it
	#
	# CC=..  CFLAGS=..  LD=..  LDFLAGS=..  RUN=..  gtkdoc-scangobj
	#
	# where the variables contain the right compiler, linker and their flags
	# to build a program using your library.  See the source of
	# gtkdoc-scangobj around line containing `Compiling scanner' if you want
	# to know how exactly are the variables used.

	# gtkdoc-mktmpl
	#
	# See: cd /usr/share/gtk-doc/data/
	#         && diff -u gtk-doc.make gtk-doc.notmpl.make
	#
	# add_custom_command (
	#	OUTPUT ${_working_dir}/${_module_name}-unused.txt
	#	       ${_working_dir}/tmpl.stamp
	## This file is updated with unused templates
	##	       ${_tmpl_dir}/${_module_name}-unused.sgml
	## The directory is created if it didn't exist
	##	       ${_tmpl_dir}
	#	COMMAND ${GTK_DOC_MKTMPL_EXECUTABLE}
	#	        --module=${_module_name}
	#	        --output-dir=${_tmpl_dir}
	#	WORKING_DIRECTORY ${_working_dir}
	#	COMMENT "Calling gtkdoc-mktmpl" VERBATIM)

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
			COMMAND ${GTK_DOC_MKDB_EXECUTABLE}
			        --module=${_module_name}
			        ${_source_dirs} "--ignore-files=${_ignores}"
			        --output-format=${_mkdb_format}
			        --output-dir=${_mkdb_output_dir}
			        ${_mkdb_options} --main-sgml-file=${_mkdb_driver}
			DEPENDS ${_target_name}_scan
			WORKING_DIRECTORY ${_working_dir}
			COMMENT "Calling gtkdoc-mkdb" VERBATIM)

		set_source_files_properties (${_target_name}_mkdb
			PROPERTIES SYMBOLIC TRUE)
		set (_top_output ${_target_name}_mkdb)
	else (_mkdb_format)
		set (_top_output ${_target_name}_scan)
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
			COMMAND ${CMAKE_COMMAND} -E make_directory ${_html_output_dir}
			COMMAND ${CMAKE_COMMAND} -E chdir ${_html_output_dir}
			        ${GTK_DOC_MKHTML_EXECUTABLE}
			        ${_module_name} ${_mkdb_driver} ${_opt_html_param}
			DEPENDS ${_target_name}_mkdb
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



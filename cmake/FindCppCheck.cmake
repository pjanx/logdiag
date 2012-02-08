# - Find cppcheck
# This module looks for cppcheck.
# This module defines the following variables:
#
#   CPPCHECK_FOUND      - Set to TRUE when cppcheck is found.
#   CPPCHECK_EXECUTABLE - Path to the executable.
#
# and a convenience function for calling the utility:
#
#   GENERATE_CPPCHECK(SOURCES <sources to check...>
#                    [SUPPRESSION_FILE <file>]
#                    [ENABLE_IDS <id...>]
#                    [TARGET_NAME <name>]
#                    [INCLUDES <dir...>])
#
# This generates a "cppcheck" target that executes cppcheck on the specified
# sources. Sources may be either file names or directories containing files
# where all C/++ files will be parsed automatically. Use directories whenever
# possible because there is a limitation in arguments to pass to the cppcheck
# binary.
#
# SUPPRESSION_FILE may be given additionally to specify suppressions for
# cppcheck. The sources mentioned in the suppression file must be in the same
# format as given for SOURCES. This means if you specify them relative to
# CMAKE_CURRENT_SOURCE_DIR, then the same relative paths must be used in the
# suppression file.
#
# ENABLE_IDS allows to specify which additional cppcheck check ids to execute,
# e.g. all or style.
#
# With TARGET_NAME a different name for the generated check target can be
# specified. This is useful if several calls to this function are made in one
# CMake project, as otherwise the target names would collide.
#
# Additional include directories for the cppcheck program can be given with
# INCLUDES.
#
# cppcheck will be executed with CMAKE_CURRENT_SOURCE_DIR as working directory.
#
# This function can be called even if cppcheck wasn't found. In that case no
# target is created.
#
#

#=============================================================================
# Copyright 2011 Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
# Copyright 2012 Přemysl Janouch <p.janouch at gmail dot com>
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


find_program (CPPCHECK_EXECUTABLE cppcheck)
mark_as_advanced (CPPCHECK_EXECUTABLE)

include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (CppCheck DEFAULT_MSG CPPCHECK_EXECUTABLE)

include (ProcessArguments)

function (GENERATE_CPPCHECK)
	if (NOT CPPCHECK_FOUND)
		return ()
	endif (NOT CPPCHECK_FOUND)

	# Parse arguments given to this function
	set (__names  SOURCES  SUPPRESION_FILE  ENABLE_IDS  TARGET_NAME  INCLUDES)
	set (__need   YES      NO               NO          NO           NO)
	set (__want   1        1                1           1            1)
	set (__more   YES      NO               YES         NO           YES)
	set (__skip   0        0                0           0            0)

	set (__argv ${ARGV})
	PROCESS_ARGUMENTS (__argv __names __need __want __more __skip "opt_")

	# Get target name
	set (target_name "cppcheck")
	set (target_name_suffix "")

	if (opt_target_name)
		set (target_name ${opt_target_name_param})
		set (target_suffix "-${target_name}")
	endif (opt_target_name)

	set (cppcheck_base "${PROJECT_BINARY_DIR}/cppcheck${target_suffix}")

	set (cppcheck_report_file     "${cppcheck_base}-report.log")
	set (cppcheck_wrapper_script  "${cppcheck_base}.cmake")

	# Prepare a command line for cppcheck
	set (source_args ${opt_sources_param})
	set (options "--inline-suppr")

	# Suppression argument
	if (opt_suppression_file)
		get_filename_component (abs "${opt_suppression_file_param}" ABSOLUTE)
		set (options "${options} --suppressions \"${abs}\"")
	endif (opt_suppression_file)

	# Includes
	foreach (include ${opt_includes_param})
		set (options "${options} -I \"${include}\"")
	endforeach (include)

	# Enabled ids
	if (opt_enable_ids)
		set (id_list "")
		foreach (id ${opt_enable_ids_param})
			set (id_list "${id_list},${id}")
		endforeach (id)

		string (SUBSTRING ${id_list} 1 -1 id_list)
		set (options "${options} \"--enable=${id_list}\"")
	endif (opt_enable_ids)

	# Create a wrapper script which redirects stderr of cppcheck to a file
	file (WRITE ${cppcheck_wrapper_script} "
		execute_process (
			COMMAND \"${CPPCHECK_EXECUTABLE}\" ${options} ${source_args}
			RESULT_VARIABLE exit_code
			ERROR_VARIABLE error_out
			WORKING_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}\")
		if (exit_code)
			message (FATAL_ERROR \"Error executing cppcheck\")
		endif (exit_code)
		if (error_out)
			message (\"\\nDetected errors:\\n\${error_out}\")
		endif (error_out)
		file (WRITE \"${cppcheck_report_file}\" \"\${error_out}\")
	")

	add_custom_target (${target_name}
		COMMAND ${CMAKE_COMMAND} -P "${cppcheck_wrapper_script}"
		DEPENDS "${cppcheck_wrapper_script}"
		COMMENT "Calling cppcheck static code analyzer" VERBATIM)
endfunction (GENERATE_CPPCHECK)


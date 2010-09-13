# - Parse the arguments in ARGS.
# This module provides the following macro:
# PROCESS_ARGUMENTS (ARGS NAMES NEED WANT MORE SKIP [PREFIX])
#
#   ARGS   - the list of arguments
#   NAMES  - a list of accepted option names, in order
#   NEED   - a list of boolean values specifying whether the corresponding
#            options are required
#   WANT   - a list of integer values specifying how many arguments
#            the corresponding options require
#   MORE   - a list of boolean values specifying whether the corresponding
#            options accept more arguments than they want
#   SKIP   - a list of integer values for skipping control
#   PREFIX - output variables prefix, "_" by default
#
#   If an option is present, ${PREFIX}${lower cased name of the option}
#   will be set to TRUE. If it's got any parameters, they will be stored
#   in ${PREFIX}${lower case name of the option}_param.
#
#   All the lists are indirect and the arguments to this macro
#   specify names of the lists within the current scope.
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

macro (PROCESS_ARGUMENTS ARGS NAMES NEED WANT MORE SKIP)
	# Get the prefix
	if ("${ARGN}" STREQUAL "")
		set (__pa_prefix "_")
	else ("${ARGN}" STREQUAL "")
		set (__pa_value ${ARGN})
		list (GET __pa_value 0 __pa_prefix)
	endif ("${ARGN}" STREQUAL "")

	# By default, no parameters have been read
	foreach (__pa_value ${${NAMES}})
		string (TOLOWER ${__pa_value} __pa_value)
		set (${__pa_prefix}${__pa_value} FALSE)
		set (${__pa_prefix}${__pa_value}_param)
	endforeach (__pa_value)

	# Find the first required option or -1
	list (FIND ${NEED} "YES" __pa_need)

	# Currently processed option
	set (__pa_cur "")
	# Index of the option
	set (__pa_cur_index -1)
	# In lowercase, prefixed with an underscore
	set (__pa_cur_base "")
	# How many arguments we want for this option
	set (__pa_want 0)
	# Do we accept additional arguments?
	set (__pa_more FALSE)
	# The next expected index minus one
	set (__pa_cur_nextmm -1)

	foreach (__pa_arg ${${ARGS}})
		list (FIND ${NAMES} ${__pa_arg} __pa_found)

		# Found an option name
		if (${__pa_found} GREATER ${__pa_cur_nextmm})
			# Missing arguments
			if (${__pa_want} GREATER 0)
				message (FATAL_ERROR "Argument(s) required for ${__pa_cur}")
			# Missing option
			elseif (${__pa_need} GREATER -1
				AND ${__pa_found} GREATER ${__pa_need})
				list (GET ${NAMES} ${__pa_need} __pa_value)
				message (FATAL_ERROR "Option ${__pa_value} needed")
			endif (${__pa_want} GREATER 0)

			set (__pa_cur ${__pa_arg})
			set (__pa_cur_index ${__pa_found})
			string (TOLOWER ${__pa_arg} __pa_value)
			set (__pa_cur_base "${__pa_prefix}${__pa_value}")

			list (GET ${WANT} ${__pa_found} __pa_want)
			list (GET ${MORE} ${__pa_found} __pa_more)

			# Skipping control
			list (GET ${SKIP} ${__pa_found} __pa_value)
			math (EXPR __pa_cur_nextmm "${__pa_found} + ${__pa_value}")

			# Option found
			set (${__pa_cur_base} TRUE)

			# Since we read it, it's not needed anymore
			if (${__pa_found} EQUAL ${__pa_need})
				list (INSERT ${NEED} ${__pa_need} "NO")
				math (EXPR __pa_value "${__pa_need} + 1")
				list (REMOVE_AT ${NEED} ${__pa_value})

				list (FIND ${NEED} "YES" __pa_need)
			endif (${__pa_found} EQUAL ${__pa_need})
		# Storing the required parameters for the current option
		elseif (${__pa_want} GREATER 0)
			list (APPEND ${__pa_cur_base}_param ${__pa_arg})
			math (EXPR __pa_want "${__pa_want} - 1")
		# Storing optional parameters for the current option
		elseif (__pa_more)
			list (APPEND ${__pa_cur_base}_param ${__pa_arg})
		else (${__pa_found} GREATER ${__pa_cur_nextmm})
			message (FATAL_ERROR "Unexpected ${__pa_arg}")
		endif (${__pa_found} GREATER ${__pa_cur_nextmm})
	endforeach (__pa_arg ${${ARGS}})

	# Missing arguments at the end of list
	if (${__pa_want} GREATER 0)
		message (FATAL_ERROR "Argument(s) required for ${__pa_cur}")
	# Missing options at the end of list
	elseif (${__pa_need} GREATER -1)
		list (GET ${NAMES} ${__pa_need} __pa_value)
		message (FATAL_ERROR "Option ${__pa_value} needed")
	endif (${__pa_want} GREATER 0)
endmacro (PROCESS_ARGUMENTS)


# Usage: cmake -P Win32Depends.cmake

# Only CMake 3.1+ supports XZ archives, not sure when 7z support comes from
cmake_minimum_required (VERSION 3.9)

# Directories
set (working_dir ${CMAKE_CURRENT_BINARY_DIR}/win32-depends)
set (pkg_dir ${working_dir}/packages)
file (MAKE_DIRECTORY ${working_dir})
file (MAKE_DIRECTORY ${pkg_dir})

# Cleanup
message (STATUS "Cleaning up...")
file (GLOB files ${working_dir}/*)
list (REMOVE_ITEM files ${pkg_dir})
if (files)
	file (REMOVE_RECURSE ${files})
endif ()

# Packages
set (pkg_list "geany" "winlibs" "mingw_lua")

# https://sourceforge.net/projects/urlget/files
# /GTK%2B%203%20binary%20for%20Windows/GTK%2B%203.16.6/
# contains a binary bundle that may be more or less simply transplanted over,
# due to ABI compatibility, however something is wrong with icons,
# and it looks alien on Windows XP (use themes) for close to no improvement.
set (pkg_geany_root "https://download.geany.org/contrib/gtk")
set (pkg_geany_urls "${pkg_geany_root}/gtk+-bundle_3.8.2-20131001_win32.zip")
set (pkg_geany_md5 "3f9b159207edf44937f209b4a5e6bb63")

set (pkg_winlibs_root "http://sourceforge.net/projects/winlibs/files")
set (pkg_winlibs_urls "${pkg_winlibs_root}/GTK+/libjson-glib-1.0-1-mingw32.7z")
set (pkg_winlibs_md5 "f06e42c5998dae5fb6245fecc96a403e")

# With luabinaries MinGW-W64 builds the .dll/.a need to be moved to bin/lib
# manually, and note that CMake 3.10.0 FindLua.cmake can't find Lua 5.4;
# in any case there is no pkg-config file
set (pkg_mingw_lua_root "http://sourceforge.net/projects/mingw/files/MinGW/Extension")
set (pkg_mingw_lua_urls
	"${pkg_mingw_lua_root}/lua/lua-5.2.0-1/lua-5.2.0-1-mingw32-dll-52.tar.xz"
	"${pkg_mingw_lua_root}/lua/lua-5.2.0-1/lua-5.2.0-1-mingw32-dev.tar.xz")
set (pkg_mingw_lua_md5
	"150b27cab05b78ba40bbd7225630c00d"
	"6abe77c1e1a783075fe73c53b7c235fb")

# Stage 1: fetch missing packages
foreach (pkg_set ${pkg_list})
	set (pkg_md5 ${pkg_${pkg_set}_md5})

	foreach (url ${pkg_${pkg_set}_urls})
		get_filename_component (basename ${url} NAME)
		set (filename ${pkg_dir}/${basename})

		if (NOT pkg_md5)
			message (WARNING "MD5 checksum missing for ${basename}")
			set (pkg_md5_sum)
			set (pkg_md5_param)
		else ()
			list (GET pkg_md5 0 pkg_md5_sum)
			list (REMOVE_AT pkg_md5 0)
			set (pkg_md5_param EXPECTED_MD5 ${pkg_md5_sum})
		endif ()

		if (NOT EXISTS ${filename})
			message (STATUS "Downloading ${url}...")
			# TODO: on Windows XP, we can't download https://curl.se/windows/
			# but it would be somewhat nice to be able to detect it in PATH
			file (DOWNLOAD ${url} ${filename} STATUS status ${pkg_md5_param})

			list (GET status 0 status_errno)
			list (GET status 1 status_msg)
			if (status_errno)
				file (REMOVE ${filename})
				message (FATAL_ERROR "Download failed: ${status_msg}")
			endif ()
		elseif (pkg_md5_sum)
			execute_process (COMMAND ${CMAKE_COMMAND} -E md5sum ${filename}
				OUTPUT_VARIABLE output)
			if (NOT output MATCHES "^${pkg_md5_sum}")
				message (FATAL_ERROR "MD5 mismatch for ${basename}")
			endif ()
		endif ()
	endforeach ()
endforeach ()

# Stage 2: extract the rest of packages
foreach (pkg_set ${pkg_list})
	foreach (url ${pkg_${pkg_set}_urls})
		get_filename_component (filename ${url} NAME)
		message (STATUS "Extracting ${filename}...")

		set (filename ${pkg_dir}/${filename})
		execute_process (COMMAND ${CMAKE_COMMAND} -E tar xf ${filename}
			WORKING_DIRECTORY ${working_dir}
			RESULT_VARIABLE status)
		if (status)
			message (FATAL_ERROR "Extraction failed: ${status}")
		endif ()
	endforeach ()
endforeach ()

# Stage 3: final touches
# We have to fix the prefix path as it is completely wrong everywhere
file (GLOB files ${working_dir}/lib/pkgconfig/*.pc)
foreach (file ${files})
	message (STATUS "Fixing ${file}...")
	file (READ ${file} file_content)
	string (REGEX REPLACE "prefix=[^\r\n]*(.*)" "prefix=${working_dir}\\1"
		file_content_fixed "${file_content}")
	file (WRITE ${file} "${file_content_fixed}")
endforeach ()

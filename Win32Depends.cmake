# Usage: cmake -P Win32Depends.cmake

# Directories
set (working_dir ${CMAKE_CURRENT_BINARY_DIR}/win32-depends)
set (pkg_dir ${working_dir}/packages)
set (tools_dir ${working_dir}/tools)
set (sevenzip_executable ${tools_dir}/7za)
set (tmp_dir ${working_dir}/tmp)
file (MAKE_DIRECTORY ${working_dir})
file (MAKE_DIRECTORY ${pkg_dir})

# Cleanup
message (STATUS "Cleaning up...")
file (GLOB files ${working_dir}/*)
list (REMOVE_ITEM files ${pkg_dir})
if (files)
	file (REMOVE_RECURSE ${files})
endif (files)

# Packages
set (pkg_list "7za" "gtk" "winlibs" "mingw_lua")

set (pkg_7za_root "http://sourceforge.net/projects/sevenzip/files")
set (pkg_7za_urls "${pkg_7z_root}/7-Zip/9.20/7za920.zip")
set (pkg_7za_md5 "2fac454a90ae96021f4ffc607d4c00f8")

set (pkg_gtk_root "http://ftp.gnome.org/pub/gnome/binaries/win32")
set (pkg_gtk_urls
	"${pkg_gtk_root}/gtk+/2.24/gtk+-bundle_2.24.10-20120208_win32.zip"
	"${pkg_gtk_root}/dependencies/gettext-tools-0.17.zip")
set (pkg_gtk_md5
	"7ae20007b76e8099b05edc789bb23e54"
	"09baff956ebd1c391c7f71e9bd768edd")

set (pkg_winlibs_root "http://sourceforge.net/projects/winlibs/files")
set (pkg_winlibs_urls "${pkg_winlibs_root}/GTK+/libjson-glib-1.0-1-mingw32.7z")
set (pkg_winlibs_md5 "f06e42c5998dae5fb6245fecc96a403e")

set (pkg_mingw_lua_root "http://sourceforge.net/projects/mingw-cross/files/%5BLIB%5D%20Lua")
set (pkg_mingw_lua_name "mingw32-lua-5.1.4-2")
set (pkg_mingw_lua_urls "${pkg_mingw_lua_root}/${pkg_mingw_lua_name}/${pkg_mingw_lua_name}.zip")
set (pkg_mingw_lua_strip ${pkg_mingw_lua_name})
set (pkg_mingw_lua_md5 "7deb1f62a9631871e9b90c0419c2e2bb")

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
		else (NOT pkg_md5)
			list (GET pkg_md5 0 pkg_md5_sum)
			list (REMOVE_AT pkg_md5 0)
			set (pkg_md5_param EXPECTED_MD5 ${pkg_md5_sum})
		endif (NOT pkg_md5)

		if (NOT EXISTS ${filename})
			message (STATUS "Downloading ${url}...")
			file (DOWNLOAD ${url} ${filename} STATUS status ${pkg_md5_param})

			list (GET status 0 status_errno)
			list (GET status 1 status_msg)
			if (status_errno)
				file (REMOVE ${filename})
				message (FATAL_ERROR "Download failed: ${status_msg}")
			endif (status_errno)
		elseif (pkg_md5_sum)
			execute_process (COMMAND ${CMAKE_COMMAND} -E md5sum ${filename}
				OUTPUT_VARIABLE output)
			if (NOT output MATCHES "^${pkg_md5_sum}")
				message (FATAL_ERROR "MD5 mismatch for ${basename}")
			endif (NOT output MATCHES "^${pkg_md5_sum}")
		endif (NOT EXISTS ${filename})
	endforeach (url)
endforeach (pkg_set)

if (NOT WIN32)
	message (FATAL_ERROR "Must run on Windows to extract packages; aborting")
endif (NOT WIN32)

# Stage 2: setup 7za first
file (MAKE_DIRECTORY ${tmp_dir})
foreach (url ${pkg_7za_urls})
	get_filename_component (filename ${url} NAME)
	message (STATUS "Extracting ${filename}...")

	set (filename ${pkg_dir}/${filename})
	execute_process (COMMAND ${CMAKE_COMMAND} -E tar xf ${filename}
		WORKING_DIRECTORY ${tmp_dir}
		RESULT_VARIABLE status)
	if (status)
		message (FATAL_ERROR "Extraction failed: ${status}")
	endif (status)
endforeach (url)

file (MAKE_DIRECTORY ${tools_dir})
file (COPY ${tmp_dir}/7za.exe DESTINATION ${tools_dir})
file (REMOVE_RECURSE ${tmp_dir})
list (REMOVE_ITEM pkg_list "7za")

# Stage 4: extract the rest of packages
foreach (pkg_set ${pkg_list})
	foreach (url ${pkg_${pkg_set}_urls})
		get_filename_component (filename ${url} NAME)
		message (STATUS "Extracting ${filename}...")

		if (filename MATCHES "7z$")
			set (extract_command ${sevenzip_executable} x)
			set (quiet OUTPUT_QUIET)
		else (filename MATCHES "7z$")
			set (extract_command ${CMAKE_COMMAND} -E tar xf)
			set (quiet)
		endif (filename MATCHES "7z$")

		set (filename ${pkg_dir}/${filename})
		if (pkg_${pkg_set}_strip)
			file (MAKE_DIRECTORY ${tmp_dir})
			execute_process (COMMAND ${extract_command} ${filename}
				WORKING_DIRECTORY ${tmp_dir}
				RESULT_VARIABLE status ${quiet})
			file (COPY ${tmp_dir}/${pkg_${pkg_set}_strip}/
				DESTINATION ${working_dir})
			file (REMOVE_RECURSE ${tmp_dir})
		else (pkg_${pkg_set}_strip)
			execute_process (COMMAND ${extract_command} ${filename}
				WORKING_DIRECTORY ${working_dir}
				RESULT_VARIABLE status ${quiet})
		endif (pkg_${pkg_set}_strip)

		if (status)
			message (FATAL_ERROR "Extraction failed: ${status}")
		endif (status)
	endforeach (url)
endforeach (pkg_set)

# Stage 5: final touches
file (WRITE ${working_dir}/etc/gtk-2.0/gtkrc
	"gtk-theme-name = \"MS-Windows\"")

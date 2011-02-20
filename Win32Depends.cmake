# Usage: cmake -P Win32Depends.cmake

# Directories
set (working_dir ${CMAKE_CURRENT_BINARY_DIR}/win32-depends)
set (pkg_dir ${working_dir}/packages)
set (bsdtar_dir ${working_dir}/bsdtar)
set (bsdtar_executable ${bsdtar_dir}/bsdtar)
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
set (pkg_list "bsdtar" "gtk" "opensuse" "mingw_lua")

set (pkg_bsdtar_root "http://sourceforge.net/projects/mingw/files/MinGW")
set (pkg_bsdtar_urls
	"${pkg_bsdtar_root}/libarchive/libarchive-2.8.3-1/bsdtar-2.8.3-1-mingw32-bin.tar.bz2"
	"${pkg_bsdtar_root}/libarchive/libarchive-2.8.3-1/libarchive-2.8.3-1-mingw32-dll-2.tar.bz2"
	"${pkg_bsdtar_root}/expat/expat-2.0.1-1/libexpat-2.0.1-1-mingw32-dll-1.tar.gz"
	"${pkg_bsdtar_root}/xz/xz-4.999.9beta_20100401-1/liblzma-4.999.9beta_20100401-1-mingw32-dll-1.tar.bz2"
	"${pkg_bsdtar_root}/bzip2/release%201.0.5-2/libbz2-1.0.5-2-mingw32-dll-2.tar.gz"
	"${pkg_bsdtar_root}/zlib/zlib-1.2.3-1-mingw32/libz-1.2.3-1-mingw32-dll-1.tar.gz")
set (pkg_bsdtar_md5
	"160168b10075bf11a6405d43d98b1612"
	"8409b7e9138423b491a41faff742a362"
	"deb721ecbcb723d5d3ac4b7dc0860402"
	"5f98e85610656cfcfa68c45e601bad0e"
	"86a00cac65439ef3e3cb5c466cf6695f"
	"4ccd26ac32ad3ffdef5e78cdc770ef12")

set (pkg_gtk_root "http://ftp.gnome.org/pub/gnome/binaries/win32")
set (pkg_gtk_urls
	"${pkg_gtk_root}/gtk+/2.22/gtk+-bundle_2.22.1-20101227_win32.zip"
	"${pkg_gtk_root}/librsvg/2.32/librsvg_2.32.1-1_win32.zip"
	"${pkg_gtk_root}/librsvg/2.32/librsvg-dev_2.32.1-1_win32.zip"
	"${pkg_gtk_root}/librsvg/2.32/svg-gdk-pixbuf-loader_2.32.1-1_win32.zip"
	"${pkg_gtk_root}/libcroco/0.6/libcroco_0.6.2-1_win32.zip"
	"${pkg_gtk_root}/dependencies/libxml2_2.7.7-1_win32.zip"
	"${pkg_gtk_root}/dependencies/libxml2-dev_2.7.7-1_win32.zip"
	"${pkg_gtk_root}/dependencies/gettext-tools-0.17.zip")
set (pkg_gtk_md5
	"a545c0ca1e6a312701d42a18e49572af"
	"2c712a8d7a652363241c0967098515db"
	"b09662bc99c5c1b8edb8af32a1722477"
	"bf4e34f1f175b88430159d33e01d0c49"
	"2d90c71404be0de4e5f3259f63a3e278"
	"bd6b3d8c35e06a00937db65887c6e287"
	"b6f59b70eef0992df37f8db891d4b283"
	"09baff956ebd1c391c7f71e9bd768edd")

set (pkg_opensuse_root "http://download.opensuse.org/repositories/windows:/mingw:/win32/openSUSE_Factory/noarch")
set (pkg_opensuse_urls
	"${pkg_opensuse_root}/mingw32-libjson-glib-0.12.0-3.1.noarch.rpm"
	"${pkg_opensuse_root}/mingw32-json-glib-devel-0.12.0-3.1.noarch.rpm")
set (pkg_opensuse_strip "usr/i686-pc-mingw32/sys-root/mingw")
set (pkg_opensuse_md5
	"df41a913984b865a46d234ad59703c8c"
	"13702ee3e22f79f42948ec5cbb6e23cb")

set (pkg_mingw_lua_root "http://sourceforge.net/projects/mingw-cross/files/%5BLIB%5D%20Lua")
set (pkg_mingw_lua_name "mingw32-lua-5.1.4-2")
set (pkg_mingw_lua_urls
	"${pkg_mingw_lua_root}/${pkg_mingw_lua_name}/${pkg_mingw_lua_name}.zip")
set (pkg_mingw_lua_strip ${pkg_mingw_lua_name})
set (pkg_mingw_lua_md5
	"7deb1f62a9631871e9b90c0419c2e2bb")

# Stage 1: fetch missing packages
foreach (pkg_set ${pkg_list})
	set (pkg_md5 ${pkg_${pkg_set}_md5})

	foreach (url ${pkg_${pkg_set}_urls})
		get_filename_component (basename ${url} NAME)
		set (filename ${pkg_dir}/${basename})

		if (NOT pkg_md5)
			message (WARNING "Checksum missing for ${basename}")
			set (pkg_md5_param)
		else (NOT pkg_md5)
			list (GET pkg_md5 0 pkg_md5_param)
			list (REMOVE_AT pkg_md5 0)
			set (pkg_md5_param EXPECTED_MD5 ${pkg_md5_param})
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
		endif (NOT EXISTS ${filename})
	endforeach (url)
endforeach (pkg_set)

if (NOT WIN32)
	message (FATAL_ERROR "Must run on Windows to extract packages; aborting")
endif (NOT WIN32)

# Stage 2: setup bsdtar first (RPM support)
file (MAKE_DIRECTORY ${tmp_dir})
foreach (url ${pkg_bsdtar_urls})
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

file (COPY ${tmp_dir}/bin/ DESTINATION ${bsdtar_dir})
file (REMOVE_RECURSE ${tmp_dir})
list (REMOVE_ITEM pkg_list "bsdtar")

# Stage 3: extract the rest of packages
foreach (pkg_set ${pkg_list})
	foreach (url ${pkg_${pkg_set}_urls})
		get_filename_component (filename ${url} NAME)
		message (STATUS "Extracting ${filename}...")
		set (filename ${pkg_dir}/${filename})

		if (pkg_${pkg_set}_strip)
			file (MAKE_DIRECTORY ${tmp_dir})
			execute_process (COMMAND ${bsdtar_executable} -xf ${filename}
				WORKING_DIRECTORY ${tmp_dir}
				RESULT_VARIABLE status)
			file (COPY ${tmp_dir}/${pkg_${pkg_set}_strip}/
				DESTINATION ${working_dir})
			file (REMOVE_RECURSE ${tmp_dir})
		else (pkg_${pkg_set}_strip)
			execute_process (COMMAND ${bsdtar_executable} -xf ${filename}
				WORKING_DIRECTORY ${working_dir}
				RESULT_VARIABLE status)
		endif (pkg_${pkg_set}_strip)

		if (status)
			message (FATAL_ERROR "Extraction failed: ${status}")
		endif (status)
	endforeach (url)
endforeach (pkg_set)

# Stage 4: final touches
file (WRITE ${working_dir}/etc/gtk-2.0/gtkrc
	"gtk-theme-name = \"MS-Windows\"")

set (gdk_pixbuf_libdir lib/gdk-pixbuf-2.0/2.10.0)
set (ENV{GDK_PIXBUF_MODULE_FILE} ${gdk_pixbuf_libdir}/loaders.cache)
set (ENV{GDK_PIXBUF_MODULEDIR} ${gdk_pixbuf_libdir}/loaders)
execute_process (COMMAND
	${working_dir}/bin/gdk-pixbuf-query-loaders --update-cache
	WORKING_DIRECTORY "${working_dir}"
	RESULT_VARIABLE result)
if (result)
	message (FATAL_ERROR "gdk-pixbuf-query-loaders failed")
endif (result)


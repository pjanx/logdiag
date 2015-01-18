# logdiag

logdiag is a simple multiplatform schematic editor written in GTK+.

__This software is considered to be of alpha quality and isn't recommended for
regular usage.__

## Requirements

Runtime dependencies:

 - GTK+ &gt;= 2.12
 - json-glib &gt;= 0.10.4
 - lua = 5.2

Build dependencies:

 - CMake &gt;= 2.8

## Installation from sources on Unix-like systems

First check that you have all the required dependencies installed, including
all development packages, if your distribution provides them.

Reserve a directory for an out-of-source build:

    $ mkdir build
    $ cd build

Let CMake prepare the build. You may change the directory where you want the
application to be installed. The default is _/usr/local_.

    $ cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr

Now you have two basic choices of installing the application.

#### Using _make install_

    # make install

#### Using _cpack_

You have to choose a package format understood by your system package manager.
CMake offers DEB and RPM.

After _cpack_ finishes making the package, install this file.

    $ fakeroot cpack -G DEB
    # dpkg -i logdiag-version-system-arch.deb

Leave out the fakeroot for CMake >= 2.8.9, it's been fixed since.

## Building from sources on Windows

First install CMake 2.8 and MinGW. Add both to your system path. If you want to
build an installation package, also install NSIS.

Run the following command in the directory with source files to automatically
fetch and setup all dependencies (contact me if the script becomes obsolete,
it's easy to fix but I usually update it only just a short while before
releasing a new version in order to resolve compatibility issues):

    > cmake -P Win32Depends.cmake

Reserve a directory for an out-of-source build:

    > mkdir build
    > cd build

Let CMake prepare the build:

    > cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

Now you can generate a package with CPack. You may choose between:

1. An NSIS-based installation package:

    > cpack -G NSIS

2. A portable ZIP package:

    > cpack -G ZIP

By default, that is if you specify no generator, both packages are built.


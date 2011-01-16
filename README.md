# logdiag

logdiag is a schematic editor written in GTK+. It focuses on simplicity,  
usability and openness.  

## Requirements

Runtime dependencies:  

 - GTK+ &gt;= 2.12
 - json-glib &gt;= 0.10
 - lua = 5.1

Build dependencies:  

 - CMake &gt;= 2.6

## Installation on Unix-like systems

First check that you have all the required dependencies installed.  

Reserve a directory for an out-of-source build:  
    $ mkdir build
    $ cd build

Let CMake prepare the build. You may change the directory where you want  
the application to be installed. The default is _/usr/local_.  
	$ cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr

Now you have two basic choices of installing the application:  

1. Using _make install_:
       # make install

2. Using _cpack_; you have to choose a package format understood by your  
   system package manager. CMake offers DEB and RPM.  

   After _cpack_ finishes making the package, install this file.  
       $ cpack -G DEB
       # dpkg -i logdiag-0.0-Linux-x86_64.deb


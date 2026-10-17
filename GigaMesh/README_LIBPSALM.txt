# the boostless version of libpsalm by Bastian Rieck is available for GigaMesh inside the external directory
# If you have a fresh clone of GigaMesh:
# Goto the libpsalm subdirectory of the boostless version:
	cd external/libpsalmBoostless

# generate subdirectory to build libPsalm:
	mkdir build
	cd build

# run cmake and build:
	cmake ..
	cmake --build . --config Release

# GigaMesh expects libpsalm.a to be in the root directory of psalm, so copy it there:
	cd ..
	cp build/libpsalm.a .

# optional: remove the build directory, as it is not needed anymore:
	rm -rf build

# note for windows unsing MSVC:
# cmake has to be configured to match the configuration of the GigaMesh Build
# - for debug build, change config to Debug
# - for 64-bit build, select a 64-bit generator, e.g.:
	cmake -G "Visual Studio 16 2019 Win64" ..

# please adapt the Visual Studio version to match your installed version



#OUTDATED:

# libpsalm by Bastian Rieck is available as git submodule for GigaMesh.
# If you have a fresh clone of GigaMesh:

        git clone gitosis@gitte.iwr.uni-heidelberg.de:GigaMesh
# or (newer):
        git clone https://github.com/Submanifold/psalm

# try:

        git submodule sync
        git submodule update --init libpsalm  # for the first time (on windows)
        git submodule update libpsalm         # otherwise

Seems outdated by 01/2018 using the version from github:
WARNING: do not execute 'cmake' in the libpsalm root directory 
         as it will overwrite the manually written 'Makefile'

# linux:
        cd libpsalm
        make libpsalm
	# continue in Qt(Creator) or:
	cd ..
	qmake && make

# windows: (following the linux Makefile)
	cd libpsalm
	mkdir build
	cd build
	cmake -G "MinGW Makefiles" ../. # Attention: this has to be done in the 'build' directory!
	mingw32-make psalm
	copy libpsalm.a ..
	# continue in Qt(Creator)
	

# The latter should now show a message, that the library is present.

# If not try to remove the submodule:

	pico .gitmodules -> remove section about libpsalm
	pico .git/config -> remove section about libpsalm
	git rm --cached libpsalm

# Remark - This will not do the job: git submodule rm libpsalm

# and add it:

	git submodule add gitosis@gitte.iwr.uni-heidelberg.de:psalm libpsalm
 	git submodule init

# and build it:

        cd libpsalm
        make libpsalm
	cd ..
	qmake

# Thanks to: http://chrisjean.com/2009/04/20/git-submodules-adding-using-removing-and-updating/
# and Bastian Rieck

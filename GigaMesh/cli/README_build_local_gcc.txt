#==================================================
# Build GigaMesh CLI tools on debian stable
# e.g. for your compute server
#--------------------------------------------------
# Choose a directory in your home (~)
#==================================================
DIR_INSTALL_GCC='~/gcc8'

#--------------------------------------------------
# fetch, configure and build gcc version 8
#--------------------------------------------------
wget ftp://ftp.gwdg.de/pub/misc/gcc/releases/gcc-8.3.0/gcc-8.3.0.tar.gz
tar xvf gcc-8.3.0.tar.gz 
mkdir gcc-8.3.0/build
cd gcc-8.3.0/build
../configure --prefix=${DIR_INSTALL_GCC} --disable-multilib
make -j
make install

#--------------------------------------------------
# adapt your account
#--------------------------------------------------
echo 'export LD_LIBRARY_PATH=${DIR_INSTALL_GCC}/lib64:$LD_LIBRARY_PATH' >> ~/.profile

#--------------------------------------------------
# LOGOUT and LOGIN again !!!
#--------------------------------------------------
# Clone GigaMesh from the Repo
cd GigaMesh
qmake
make CC=~/gcc8/bin/gcc CXX=~/gcc8/bin/g++ LINK=~/gcc8/bin/g++ -j

#--------------------------------------------------
# OPTIONAL: set the new GCC GLOBAL for your account
# to avoid the lengthy make command above.
#--------------------------------------------------
echo 'PATH=~/gcc8/bin:$PATH' >> ~/.profile

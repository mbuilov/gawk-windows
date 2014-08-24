CMake is a build automation system
  http://en.wikipedia.org/wiki/Cmake

We try to use it as a replacement for the established GNU build system.
This attempt is currently only experimental. If you wonder why anyone
should do this, read

  Why the KDE project switched to CMake -- and how 
  http://lwn.net/Articles/188693/
  Escape from GNU Autohell!
  http://www.shlomifish.org/open-source/anti/autohell

- How can I get GNU Awk compiled with CMake as fast as possible ?
  git clone git://git.savannah.gnu.org/gawk.git
  cd gawk
  git checkout cmake
  mkdir build
  cd build
  cmake ..
  make
  ./gawk --version
  make test
Notice that this git-checkout allows you to read the source code,
track the cmake branch and get updates. You will not be able to
commit anything.

- How can I use git to contribute source code ?
You need an account at Savannah. Read this to understand the first steps:
  http://savannah.gnu.org/maintenance/UsingGit
  README.git
Use your account there to register your public ssh key at Savannah.
Then you are ready to checkout. Remember that (when cloning) you are
setting up your own local repository and make sure you configure it
properly.
  git clone ssh://my_account_name@git.sv.gnu.org/srv/git/gawk.git
  git config --global user.name "first-name last-name"
  git config --global user.email First.Last@email.com
  git config --global color.ui auto

- What is the current status of the cmake branch ?
It has just begun, pre-alpha, unclear if it will ever be taken up
by the maintainer. We want to study if using CMake with such a
basic tool like gawk is feasible and if it easier to use than
the GNU build system.

- Where can I find a tutorial on CMake basics ?
Use the "official tutorial":
  http://www.cmake.org/cmake/help/cmake_tutorial.html

- Where is the reference of all commands and variables ?
Depending on the CMake version you use, select one of these:
  http://www.cmake.org/cmake/help/v2.8.10/cmake.html

- How can I cross-compile ?
Proceed in the same way as explained above for native compilation,
but use a different build directory. When using CMake, do this:
  cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain_mingw32.cmake ..
Write a new Toolchain file for your cross-compiler and use it.

- How can I build an installable file ?
By default, installable files will not be generated.
But if you instruct CMake about the kind of installable file you want,
then some kinds of files can be generated.
The exact kind of installable file depends on your operating system.
Possible kinds are TGZ (.tar.gz file), RPM (.rpm file), and DEB (.deb file).
   cmake -DCPACK_GENERATOR=DEB ..
   make package

- Can I build an executable that runs on any Win32 platform ?
Yes, there are two ways of doing this.
In both cases you need a MinGW compiler and the NSIS package builder
installed on the host that shall do the build.
  http://sourceforge.net/projects/mingw
  http://sourceforge.net/projects/nsis
When installed properly, the NSIS tool can even build an installer file
(a single .exe file that unpacks, registers and installs the gawk executable
and several other files).
1. way: native build on a Win32 platform 
   http://www.cmake.org/cmake/help/runningcmake.html
   After clicking "Configure" select the MinGW option with the default native compiler
   In the build directory, the command "mingw32-make" will build the gawk.exe
   The command "mingw32-make package" will build installer file
2. way: build with cross-compiler on a Linux platform like Ubuntu 12.04 LTS
   Proceed as describe above for cross-compilers.
   The command "make ; make package" will build gawk.exe and the installer file

- How can I run test cases ?
You can run all the test cases that are defined in test/Makefile.am.
These test case scripts were not changed, but the way they are invoked has
been adapted to CMake habits.
See http://cmake.org/Wiki/CMake/Testing_With_CTest#Simple_Testing
  cmake ..
  make
  make test          # run all test cases
  ctest -N           # list all test cases but don't run them
  ctest -R BASIC     # run all test cases belonging to group BASIC
  ctest -R MPFR      # run all test cases belonging to group MPFR
  ctest -E SHLIB.filefunc # run all tests, except the SHLIB.filefunc test case
Remember that running test cases is possible only after a native build.


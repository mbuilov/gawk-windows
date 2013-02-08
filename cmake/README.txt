CMake is a build automation system
  http://en.wikipedia.org/wiki/Cmake

We try to use it as a replacement for the established GNU build system.
This attempt is currently only experimental. If you wonder why anyone
should do this, read

  Why the KDE project switched to CMake -- and how 
  http://lwn.net/Articles/188693/

- How can I get gawk compiled with CMake as fast as possible ?
  git clone git://git.savannah.gnu.org/gawk.git
  cd gawk
  git checkout cmake
  mkdir build
  cd build
  cmake ..
  make
  ./gawk --version
Notice that this git-checkout allows you to read the source code,
track the cmake branch and get updates. You will not be able to
commit anything. http://savannah.gnu.org/maintenance/UsingGit

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


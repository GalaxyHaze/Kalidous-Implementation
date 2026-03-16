# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/diogo/CLionProjects/Kalidous-Implementation/cmake-build-debug/_deps/kalidousabi-src")
  file(MAKE_DIRECTORY "/home/diogo/CLionProjects/Kalidous-Implementation/cmake-build-debug/_deps/kalidousabi-src")
endif()
file(MAKE_DIRECTORY
  "/home/diogo/CLionProjects/Kalidous-Implementation/cmake-build-debug/_deps/kalidousabi-build"
  "/home/diogo/CLionProjects/Kalidous-Implementation/cmake-build-debug/_deps/kalidousabi-subbuild/kalidousabi-populate-prefix"
  "/home/diogo/CLionProjects/Kalidous-Implementation/cmake-build-debug/_deps/kalidousabi-subbuild/kalidousabi-populate-prefix/tmp"
  "/home/diogo/CLionProjects/Kalidous-Implementation/cmake-build-debug/_deps/kalidousabi-subbuild/kalidousabi-populate-prefix/src/kalidousabi-populate-stamp"
  "/home/diogo/CLionProjects/Kalidous-Implementation/cmake-build-debug/_deps/kalidousabi-subbuild/kalidousabi-populate-prefix/src"
  "/home/diogo/CLionProjects/Kalidous-Implementation/cmake-build-debug/_deps/kalidousabi-subbuild/kalidousabi-populate-prefix/src/kalidousabi-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/diogo/CLionProjects/Kalidous-Implementation/cmake-build-debug/_deps/kalidousabi-subbuild/kalidousabi-populate-prefix/src/kalidousabi-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/diogo/CLionProjects/Kalidous-Implementation/cmake-build-debug/_deps/kalidousabi-subbuild/kalidousabi-populate-prefix/src/kalidousabi-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()

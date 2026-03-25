# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/ASUS/OneDrive/Masaüstü/Uygulama_Projem/KHE_Public/build/_deps/sqlite3_src-src")
  file(MAKE_DIRECTORY "C:/Users/ASUS/OneDrive/Masaüstü/Uygulama_Projem/KHE_Public/build/_deps/sqlite3_src-src")
endif()
file(MAKE_DIRECTORY
  "C:/Users/ASUS/OneDrive/Masaüstü/Uygulama_Projem/KHE_Public/build/_deps/sqlite3_src-build"
  "C:/Users/ASUS/OneDrive/Masaüstü/Uygulama_Projem/KHE_Public/build/_deps/sqlite3_src-subbuild/sqlite3_src-populate-prefix"
  "C:/Users/ASUS/OneDrive/Masaüstü/Uygulama_Projem/KHE_Public/build/_deps/sqlite3_src-subbuild/sqlite3_src-populate-prefix/tmp"
  "C:/Users/ASUS/OneDrive/Masaüstü/Uygulama_Projem/KHE_Public/build/_deps/sqlite3_src-subbuild/sqlite3_src-populate-prefix/src/sqlite3_src-populate-stamp"
  "C:/Users/ASUS/OneDrive/Masaüstü/Uygulama_Projem/KHE_Public/build/_deps/sqlite3_src-subbuild/sqlite3_src-populate-prefix/src"
  "C:/Users/ASUS/OneDrive/Masaüstü/Uygulama_Projem/KHE_Public/build/_deps/sqlite3_src-subbuild/sqlite3_src-populate-prefix/src/sqlite3_src-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/ASUS/OneDrive/Masaüstü/Uygulama_Projem/KHE_Public/build/_deps/sqlite3_src-subbuild/sqlite3_src-populate-prefix/src/sqlite3_src-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/ASUS/OneDrive/Masaüstü/Uygulama_Projem/KHE_Public/build/_deps/sqlite3_src-subbuild/sqlite3_src-populate-prefix/src/sqlite3_src-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()

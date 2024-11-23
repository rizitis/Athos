# Install script for directory: /root/Desktop/AthOS/REPO-AthOS/massif-visualizer/work/massif-visualizer-0.8.0/app

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "massif-visualizer" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/massif-visualizer" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/massif-visualizer")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/massif-visualizer"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/root/Desktop/AthOS/REPO-AthOS/massif-visualizer/work/massif-visualizer-0.8.0/build/bin/massif-visualizer")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/massif-visualizer" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/massif-visualizer")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/massif-visualizer")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "massif-visualizer" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/config.kcfg" TYPE FILE FILES "/root/Desktop/AthOS/REPO-AthOS/massif-visualizer/work/massif-visualizer-0.8.0/app/massif-visualizer-settings.kcfg")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "massif-visualizer" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications" TYPE PROGRAM FILES "/root/Desktop/AthOS/REPO-AthOS/massif-visualizer/work/massif-visualizer-0.8.0/app/org.kde.massif-visualizer.desktop")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "massif-visualizer" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/metainfo" TYPE FILE FILES "/root/Desktop/AthOS/REPO-AthOS/massif-visualizer/work/massif-visualizer-0.8.0/app/org.kde.massif-visualizer.appdata.xml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "massif-visualizer" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/mime/packages" TYPE FILE FILES "/root/Desktop/AthOS/REPO-AthOS/massif-visualizer/work/massif-visualizer-0.8.0/app/massif.xml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "massif-visualizer" OR NOT CMAKE_INSTALL_COMPONENT)
  
set(DESTDIR_VALUE "$ENV{DESTDIR}")
if (NOT DESTDIR_VALUE)
    # under Windows relative paths are used, that's why it runs from CMAKE_INSTALL_PREFIX
    message(STATUS "Updating MIME database at ${CMAKE_INSTALL_PREFIX}/share/mime")
    execute_process(COMMAND "/usr/bin/update-mime-database" -n "share/mime"
                    WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
endif (NOT DESTDIR_VALUE)

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/root/Desktop/AthOS/REPO-AthOS/massif-visualizer/work/massif-visualizer-0.8.0/build/app/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()

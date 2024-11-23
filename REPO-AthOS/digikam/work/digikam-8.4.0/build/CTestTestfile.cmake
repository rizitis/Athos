# CMake generated Testfile for 
# Source directory: /root/Desktop/AthOS/REPO-AthOS/digikam/work/digikam-8.4.0
# Build directory: /root/Desktop/AthOS/REPO-AthOS/digikam/work/digikam-8.4.0/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(appstreamtest "/usr/bin/cmake" "-DAPPSTREAMCLI=/usr/bin/appstreamcli" "-DINSTALL_FILES=/root/Desktop/AthOS/REPO-AthOS/digikam/work/digikam-8.4.0/build/install_manifest.txt" "-P" "/usr/share/ECM/kde-modules/appstreamtest.cmake")
set_tests_properties(appstreamtest PROPERTIES  _BACKTRACE_TRIPLES "/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;168;add_test;/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;187;appstreamtest;/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;0;;/root/Desktop/AthOS/REPO-AthOS/digikam/work/digikam-8.4.0/CMakeLists.txt;100;include;/root/Desktop/AthOS/REPO-AthOS/digikam/work/digikam-8.4.0/CMakeLists.txt;0;")
subdirs("core")

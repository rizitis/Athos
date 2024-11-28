#!/bin/bash

# List of folder names to check
folders=(
    attica baloo bluez-qt breeze-icons extra-cmake-modules frameworkintegration kapidox
    karchive kauth kbookmarks kcalendarcore kcmutils kcodecs kcolorscheme kcompletion
    kconfig kconfigwidgets kcontacts kcoreaddons kcrash kdav kdbusaddons kdeclarative
    kded kdesu kdnssd kdoctools kfilemetadata kglobalaccel kguiaddons kholidays ki18n
    kiconthemes kidletime kimageformats kio kirigami kitemmodels kitemviews kjobwidgets
    knewstuff knotifications knotifyconfig kpackage kparts kpeople kplotting kpty
    kquickcharts krunner kservice kstatusnotifieritem ksvg ktexteditor ktexttemplate
    ktextwidgets kunitconversion kuserfeedback kwallet kwidgetsaddons kwindowsystem
    kxmlgui modemmanager-qt networkmanager-qt prison purpose qqc2-desktop-style solid
    sonnet syndication syntax-highlighting threadweaver zzz-meta
)

# Loop through each folder and check existence
for folder in "${folders[@]}"; do
    if [ -d "../REPO-AthOS/$folder" ]; then
        echo "$folder exists"
    else
        echo "$folder does NOT EXIST"
    fi
done

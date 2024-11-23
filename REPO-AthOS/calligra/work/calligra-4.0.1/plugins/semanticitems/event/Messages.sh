#! /bin/sh
# Messages.sh files must have one instance of the line with:
# 'potfilename=<potfile>.pot'
# potfilename= must be at the start of the line and without spaces.
# It must refer to one pot file only.
# Release scripts rely on this.
potfilename=calligra_semanticitem_event.pot

source ../../../kundo2_aware_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp
kundo2_aware_xgettext $potfilename `find . -name \*.cpp`
rm -f rc.cpp

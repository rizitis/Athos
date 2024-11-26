     config() {
       for infile in $1; do
         NEW="$infile"
         OLD="$(dirname $NEW)/$(basename $NEW .new)"
         if [ ! -r $OLD ]; then
           mv $NEW $OLD
         elif [ "$(cat $OLD | md5sum)" = "$(cat $NEW | md5sum)" ]; then
         rm $NEW
         fi
         done
        }

     preserve_perms() {
       NEW="$1"
       OLD="$(dirname $NEW)/$(basename $NEW .new)"
       if [ -e $OLD ]; then
         cp -a $OLD ${NEW}.incoming
         cat $NEW > ${NEW}.incoming
         mv ${NEW}.incoming $NEW
       fi
       config $NEW
       }


#        preserve_perms etc/rc.d/rc.INIT.new
#        config etc/configfile.new

     if [ -x /usr/bin/fc-cache ]; then
       /usr/bin/fc-cache -fv /usr/share/fonts/
     fi

     if [ -x usr/bin/update-desktop-database ]; then
       chroot . /usr/bin/update-desktop-database usr/share/applications > /dev/null 2>&1
     fi

     if [ -d usr/share/icons/hicolor ]; then
       if [ -x /usr/bin/gtk-update-icon-cache ]; then
         chroot . /usr/bin/gtk-update-icon-cache -f -t usr/share/icons/hicolor 1> /dev/null 2> /dev/null
       fi
     fi

     if [ -e usr/share/glib-2.0/schemas ]; then
       if [ -x /usr/bin/glib-compile-schemas ]; then
         /usr/bin/glib-compile-schemas usr/share/glib-2.0/schemas >/dev/null 2>&1
       fi
     fi

     if [ -x usr/bin/update-mime-database ]; then
       chroot . /usr/bin/update-mime-database usr/share/mime >/dev/null 2>&1
     fi


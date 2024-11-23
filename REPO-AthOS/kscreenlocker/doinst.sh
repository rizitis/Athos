#!/bin/sh

# Backup the existing system-auth file
if [ -f /etc/pam.d/system-auth ]; then
    cp /etc/pam.d/system-auth /etc/pam.d/system-auth.bak
fi

# Backup the current kde PAM file before replacing it
if [ -f /etc/pam.d/kde ]; then
    cp /etc/pam.d/kde /etc/pam.d/kde.bak
fi

# Replace kde.new with kde if it exists
if [ -f /etc/pam.d/kde.new ]; then
    mv /etc/pam.d/kde.new /etc/pam.d/kde
fi

# Remove any existing base-* symlinks or files
for file in base-auth base-account base-password base-session; do
    [ -e /etc/pam.d/$file ] && rm -rf /etc/pam.d/$file
done

# Create symlinks pointing base-* to system-auth
for file in base-auth base-account base-password base-session; do
    ln -s system-auth /etc/pam.d/$file
done


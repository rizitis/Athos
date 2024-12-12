#!/bin/bash

# Set variables for the Asterisk user and group
ASTERISK_USER="asterisk"
ASTERISK_GROUP="asterisk"
ASTERISK_HOME="/var/lib/asterisk"

# Create the asterisk group (only if it doesn't exist)
getent group $ASTERISK_GROUP > /dev/null || sudo groupadd $ASTERISK_GROUP

# Create the asterisk user (only if it doesn't exist)
getent passwd $ASTERISK_USER > /dev/null || sudo useradd -r -d $ASTERISK_HOME -s /sbin/nologin -g $ASTERISK_GROUP -m $ASTERISK_USER

# Add the asterisk user to the dialout group if it doesn't exist
getent group dialout > /dev/null || sudo groupadd dialout
sudo usermod -aG dialout $ASTERISK_USER

# Create the Asterisk home directory
sudo mkdir -p $ASTERISK_HOME

# Ensure the correct ownership and permissions
sudo chown -R $ASTERISK_USER:$ASTERISK_GROUP $ASTERISK_HOME
sudo chmod 755 $ASTERISK_HOME

# Create a directory for logs and ensure correct permissions
sudo mkdir -p /var/log/asterisk
sudo chown $ASTERISK_USER:$ASTERISK_GROUP /var/log/asterisk
sudo chmod 755 /var/log/asterisk


echo "Asterisk setup completed successfully."

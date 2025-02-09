#!/bin/sh
# Pre-installation script for Gerbera on Slackware

GERBERA_USER="gerbera"
GERBERA_HOME="/var/lib/gerbera"
CONFIG_DIR="/etc/gerbera"

echo ":: Preparing system for Gerbera installation..."

# Create the Gerbera system user if it does not exist
if ! id "$GERBERA_USER" >/dev/null 2>&1; then
    echo ":: Creating user '$GERBERA_USER'..."
    useradd -r -s /bin/false -d "$GERBERA_HOME" -c "Gerbera Server" "$GERBERA_USER"
fi

# Create necessary directories
echo ":: Creating necessary directories..."
mkdir -p "$GERBERA_HOME"
mkdir -p "$CONFIG_DIR"

# Set correct ownership and permissions
echo ":: Setting permissions..."
chown -R "$GERBERA_USER:$GERBERA_USER" "$GERBERA_HOME" "$CONFIG_DIR"
chmod 0750 "$GERBERA_HOME"
chmod 0755 "$CONFIG_DIR"

echo ":: Pre-installation setup complete."

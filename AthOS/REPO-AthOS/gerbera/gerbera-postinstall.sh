#!/bin/sh
# Post-installation script for Gerbera on Slackware

CONFIG_DIR="/etc/gerbera"
CONFIG_FILE="$CONFIG_DIR/config.xml"
GERBERA_USER="gerbera"

echo ":: Running Gerbera post-installation steps..."

# Generate configuration file if it doesn't exist
if [ ! -f "$CONFIG_FILE" ]; then
    echo ":: Generating default Gerbera configuration..."
    gerbera --create-config | tee "$CONFIG_FILE"
fi

# Set correct ownership and permissions
echo ":: Setting permissions..."
chown "$GERBERA_USER:$GERBERA_USER" "$CONFIG_FILE"
chmod 0660 "$CONFIG_FILE"

# Ensure <home> is set correctly in the configuration file
if ! grep -q "<home>/var/lib/gerbera</home>" "$CONFIG_FILE"; then
    echo ":: Updating <home> directory in config.xml..."
    sed -i 's|<home>.*</home>|<home>/var/lib/gerbera</home>|' "$CONFIG_FILE"
fi

# Create an rc.d startup script
RC_SCRIPT="/etc/rc.d/rc.gerbera"
if [ ! -f "$RC_SCRIPT" ]; then
    echo ":: Creating startup script..."
    cat <<EOF > "$RC_SCRIPT"
#!/bin/sh
case "\$1" in
    start)
        echo "Starting Gerbera..."
        su -s /bin/sh -c "gerbera --config $CONFIG_FILE --daemon" $GERBERA_USER
        ;;
    stop)
        echo "Stopping Gerbera..."
        pkill gerbera
        ;;
    restart)
        \$0 stop
        \$0 start
        ;;
    *)
        echo "Usage: \$0 {start|stop|restart}"
        exit 1
        ;;
esac
EOF
    chmod +x "$RC_SCRIPT"
fi

# Enable startup at boot
if ! grep -q "rc.gerbera start" /etc/rc.d/rc.local; then
    echo ":: Adding Gerbera to startup..."
    echo "/etc/rc.d/rc.gerbera start" >> /etc/rc.d/rc.local
fi

echo ":: Post-installation complete! Start Gerbera with: /etc/rc.d/rc.gerbera start"

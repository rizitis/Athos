#!/bin/bash
# shellcheck disable=SC2129
APPL=AthOS

# track AthOS HOME DIR
CWD=$(pwd)

# setup env file
echo "TAG=${TAG:-_ath}" > athos.env
echo 'Packager="Jonny Propper <example@mail.com>" ' >> athos.env
echo "" >> athos.env
echo "A_HOME=$CWD" >> athos.env
echo "W_DIR=$CWD/REPO-$APPL" >> athos.env
echo "SH_DIR=/usr/share/$APPL" >> athos.env
echo "ENV_DIR=/etc/$APPL" >> athos.env
echo "DOC_DIR=/usr/doc/$APPL" >> athos.env
echo "BIN_APP=/usr/local/bin/athos" >> athos.env
mkdir -p /etc/"$APPL"
mv athos.env /etc/"$APPL"/athos.env

# prepare scripts permissions and symlink to /usr/local/bin
chmod +x athos
rm -rf /usr/local/bin/athos || true
ln -sr athos /usr/local/bin/athos
chmod +x slackdesc
rm -rf /usr/local/bin/slackdesc || true
ln -sr slackdesc /usr/local/bin/slackdesc

# create docs folder
mkdir -p /usr/doc/"$APPL"
cp -R UNLICENSE setup.sh DOCS \
 /usr/doc/"$APPL"
cp ../README.md /usr/doc/"$APPL"

# create share template dir
mkdir -p /usr/share/"$APPL"
cp -R TEMPLATES /usr/share/"$APPL"/

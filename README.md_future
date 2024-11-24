# ATHOS

> We\'re living the future so
> the present is our past.



## What is AthOS

AthOS is **not**:

* a package manager
- a package manager
+ a package manager

AthOS **is**:

1. A different way to build packages for Slackware
2. A more simple way to maintain a local repo of your build scripts


#### HELP FILES

More informations you can find in [HOWTO.md][1], [INSTALL.md][2], [required.md][3] and [special-cases.md][4]

[1]: ./DOCS/HOWTO.md        'HOWTO.md'
[2]: ./DOCS/INSTALL.md  "INSTALL.md"
[3]: ./DOCS/required.md    "required.md"
[4]: ./DOCS/special-cases.md (special-cases.md)

#### TEMPLATE

**Example: 1**

```
Packager="YOUR NAME email@example.com"

Homepage="http://example.org"
        #|--------------------------------------|#
SortDesc="Write few words but respect handy-rule^"
LongDesc="Here is the log slack-desc description. You can write here all informations needed as log as you have slackdesk app installed. Do not write project url it will automatic added in last line of slack-desc."
VERSION=0.1.2
PRGNAM=foo
SRCNAM=foo
# Source download link
SRCURL=http://site.com/project/pup/release/foo-0.1.2.tar.xz
COMPRESS="tar.gz"
GITCHEK=
# Empty if not mdsum check needed.
MDSCHK=12345abcd54321opqrf
BIN_PKG=
# Add required and optional deps if needed in parenthesis. (foo foo1 foo2) else empty.
REQUIRED=(foo foo1 foo2)
OPTDEPS=

BUILD=${BUILD:-1}
TAG=${TAG:-_rtz}


# This must be always here even if not patches needed.
patch_build() {
if [ -e $CWD/patches/ ]; then
  for PATCH in $CWD/patches/*.patch; do
    patch -p1 -i $PATCH
  done
fi
}



slackbuild() {
# Fix error with gcc5
sed 's/^inline int/extern inline int/' -i database.*

autoreconf -i
LDFLAGS="-L/usr/lib${LIBDIRSUFFIX}" \
CFLAGS="$SLKCFLAGS" \
CXXFLAGS="$SLKCFLAGS" \
./configure \
  --prefix=/usr \
  --libdir=/usr/lib${LIBDIRSUFFIX} \
  --sysconfdir=/etc \
  --localstatedir=/var \
  --mandir=/usr/man \
  --infodir=/usr/info \
  --host=$ARCH-slackware-linux \
  --build=$ARCH-slackware-linux

make
make DESTDIR=$PKG install
}

```
---

**Example: 2**

```
Packager="YOUR NAME email@example.com"

Homepage="http://github.com/dev/project"
        #|--------------------------------------|#
SortDesc="Write few words but respect handy-rule^"
LongDesc="Here is the log slack-desc description. You can write here all informations needed as log as you have slackdesk app installed. Do not write project url it will automatic added in last line of slack-desc."
VERSION=0.1.2
PRGNAM=foo
SRCNAM=foo
# Source download link
SRCURL=https://github.com/name/project-name
COMPRESS="git"
GITCHEK=47a983e21ab67a867c617b7064bc0778f7c30b7a
# Empty if not mdsum check needed.
MDSCHK=
BIN_PKG=
# Add required and optional deps if needed in parenthesis. (foo foo1 foo2) else empty.
REQUIRED=
OPTDEPS=

BUILD=${BUILD:-1}
TAG=${TAG:-_rtz}


# This must be always here even if not patches needed.
patch_build() {
if [ -e $CWD/patches/ ]; then
  for PATCH in $CWD/patches/*.patch; do
    patch -p1 -i $PATCH
  done
fi
}



slackbuild() {
mkdir -p build
cd build
   CFLAGS="$SLKCFLAGS" \
   CXXFLAGS="$SLKCFLAGS" \
   meson .. \
     --buildtype=release \
     --infodir=/usr/info \
     --libdir=/usr/lib${LIBDIRSUFFIX} \
     --localstatedir=/var \
     --mandir=/usr/man \
     --prefix=/usr \
     --sysconfdir=/etc \
     -Dstrip=true

     "${NINJA:=ninja}" "$NUMJOBS"
   DESTDIR=$PKG $NINJA install
   cd ..
}

```
---

##### Commands

1. `athos package_name` (will build the package if exist in your repo)
2. `athos package_name mkbuild` (will create a package folder and in folder a template package.ini)

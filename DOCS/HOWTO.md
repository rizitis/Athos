# ATHOS

First lets explain what is not **AthOS** (athos).
1. athos is not a package manager.
2. athos is not a package manager
3. athos is not a package manager

Now that its clear that athos **is not a package manager**, lets say what is athos!<br>

Athos is a simple way to build from source packages for Slackware.<br>
You can also extract rpm,deb,AppImages etc and repackage them in Slackware binaries format. <br>  
You can use your own _tags for output packages and everything else you can do with a SlackBuild.<br>
So yes the best name for what athos is, is one different SlackBuid script way. 

---

## How-To

In order to **build and install** with AthOS template you must create a **pkg.ini**<br>
This is the only file that athos command read and source for building.

## pkg.ini

Structure of this file is very simple. Simmilar with Templates of Arch, Void, Chimera etc...<p>

**Example:**
```
Packager="Anagnostakis Ioannis (rizitis@gmail.com) GR"

mkdesc="$(which slackdesc)" || true
Homepage="https://www.gimp.org/downloads/devel/"
         #|--------------------------------------|#
SortDesc="GIMP-3 Development"
LongDesc="This is the download page for the development version of GIMP.That means these builds are a work-in-progress. Proceed with caution because they may be unstable. If a crash does occur, please report it!"
VERSION="3.0.0-RC1"
PRGNAM=gimp
SRCNAM=gimp
MDSCHK="4df6918b906033b72ede09ef09454da1"
SRCURL="https://download.gimp.org/gimp/v3.0/gimp-3.0.0-RC1.tar.xz"
COMPRESS="tar.xz"
BIN_PKG=



BUILD=${BUILD:-1}

   patch_build() {
   true
}


slackbuild() {
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
     -Denable-multiproc=true \
     -Denable-console-bin=false \
     -Drelocatable-bundle=platform-default \
     -Dcheck-update=no \
     -Dvala=enabled \
     -Dwebp=enabled \
     -Djpeg2000=enabled \
     -Djpeg-xl=enabled \
     -Dghostscript=enabled \
     -Dfits=enabled \
     -Dheif=enabled \
     -Denable-default-bin=disabled \
     -Dopenmp=enabled \
     -Dprint=true \
     -Dheadless-tests=enabled \
     -Dlibunwind=true \
     -Dlibbacktrace=true \
     -Dvector-icons=true \
     -Djavascript=enabled \
     -Dlua=true \
     -Dwith-sendmail="" \
     -Dicc-directory="" \
     -Dstrip=true
   "${NINJA:=ninja}" "$NUMJOBS"
   DESTDIR=$PKG $NINJA install
   cd ..
}
```

This was an example of **gimp.ini**

<br>So:
1. we create a pkg.ini which must have the name of the package we are buidling **gimp.ini**
2. Packager = your name or nickname etc...
3. mkdesc =  this never change dont touch it.
4. Homepage = Project HOMEPAGE URL (for the last line of slack-desc)
5. SortDesc = This is what will be in (short description) at first line of slack-desc
6. LongDesc = Discription of package for slack-desk
7. VERSION = version number source
8. PRGNAM = name of package we will install
9. SRCNAM = name of source tarball we download (sometimes is different than PRGNAM else write the same name, not empty in any case.)
10. MDSCHK = if we have the md5sum of source else empty is ok...
11. SRCURL = full url of source for download (http://www.site.com/path/to/appname-version.tar.gz)
12. COMPRESS= how is compressed source. example "tar.xz", "tar.gz" , "deb", etc..
13. BIN_PKG = If and when the extracted source code provide not a fodler (pkgname-version) but files directly, then add BIN_PKG=YES else NO or empty is ok
14. BUILD = if we rebuild the same version change it to 2 or 3 or what needed..
15. patch_build function must be "true" if we dont need patches or add patch commands
16. slackbuild_function is the build function. By build meaning everything needed to build (compile etc) package and every other command needed for $PKG but no cp docs, remove .la etc... Only what package need.

And Thats all.   
  
  For special cases there are more flexible commands and vars for pkg.ini but 90% of cases not needed. If you face any special case read special-cases.md


### build-install
As root:
`athos pkg_name`<br>
assume you want to build gimp
```
athos gimp
```

Thats all.


### depentencies resolution

For now there is an auto-track and auto-build for deps. <br>
Because its very easy to make it **but** there is one small problem. Every dep must also be included in the **REPO-AthOS**. That said, I mention that athos is not provided as an alternative of SBo (respect) and **AthOS is not a package manager**. <br>
So if you really want to create a complete rersonal repo for you or share it to others then its fine as log as all deps included in repo.
```
REQUIRED=(foo1 foo2 foo3)
```
That means:<br>
17. REQUIRED=(foo1 foo2 foo3) every foo is a folder which inclued a foo.ini<br>
18. OPTDEP=(zong zong1 zong2) if you have otpional deps just add one more line like this<br>
  
 **NOTE**
 The reason of AthOS is to make slackbuilds easy for personal packaging **in case you need a package that not exist in the usual trusted repos...**<br>
 Because **athos** is not *SlackBuilds.org* and its `not a package manager`...
 
 Enjoy!
 
 -r
 



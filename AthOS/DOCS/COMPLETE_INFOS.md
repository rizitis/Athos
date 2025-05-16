# ATHOS

Athos is a simple way to build from source packages for Slackware.<br>
You can also extract rpm,deb,AppImages etc and repackage them in Slackware binaries format. <br>  
You can use your own _tags for output packages and everything else you can do with a SlackBuild.<br>
So yes the best name for what athos is, is one different SlackBuid script way. 

---

## How-To

In order to **build and install** with AthOS template you must create a **pkg.ini**<br>
This is the file that athos command read for download source, build and install it.

## pkg.ini

Structure of this file is very simple. Simmilar with Templates of Arch, Void, Chimera, Alpine etc...<p>

**Example:**
```
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

<br>
Now lets see all options a `pkg.ini` file can support.

> everything has `must` means must exist in all `pkg.ini` files.
>
> everything has `if_need` means if `pkg.ini` need it.
>
> everything has `opt` means its optional.


1. Homepage: Project HOMEPAGE URL (for the last line of slack-desc) `must`
2. SortDesc: This is what will be in (short description) at first line of slack-desc `opt` (else you must have a slak-desc)
3. LongDesc: Discription of package for slack-desk `opt` (else you must have a slak-desc)
4. VERSION: version number source `must`
5. PRGNAM: name of package we will install `must`
6. SRCNAM: name of source tarball we download (sometimes is different than PRGNAM else write the same name, not empty in any case.) `must`
7. MDSCHK: if we have the md5sum of source `opt`
8. SHA512SUM: if we have the SHA512SUM of source `opt`
9. SHA256SUM: if we have the SHA256SUM of source `opt`
10. SRCURL: full url of source tarball to download (http://www.site.com/path/to/appname-version.tar.gz) `must`
11. COMPRESS: type of source tarball. example "tar.xz", "tar.gz" , "deb", git, etc.. `must`
12. BIN_PKG: If and when the extracted source code provide not a fodler (pkgname-version) but files directly, then add `BIN_PKG=YES`. `if_need`
13. BUILD: if we rebuild the same version change it to 2 or 3 or what needed..`must`
14. GITCHEK: when we git clone (`COMPRESS=git`) and we also need a `git checkout`. `if_need`
15. `SKIP_INSTALL=YES` if you only want to build but not install. `opt`
16. `LOCAL_FILE=YES` Assume you already have source tarball in same directory with $PRGNAM.ini so skip download source. `if_need`
17. `STUCK_PKG=YES` That mean this package dont accept updates, we also have source tarball in $CWD. This flag works together with  `LOCAL_FILE=YES` only. `if_need`
18. `META_PKG=YES` This convert pkg.ini to a fake package-meta.ini that only build all packages listed in `REQUIRED=` and exit. `opt`
19. `CHROOT=NO` Some times no chroot needed, use this in ini file. Note that means you also run athos as user no root. `if_need`
20. REQUIRED="foo1 foo2 foo3" every foo is a folder which inclued a foo.ini if pkg its not already installed. `if_need`
21. OPTDEP="zong zong1 zong2" if you have otpional deps just add one more line like this `if_need`
22. patch_build function must be there even if we dont need patches or add patch commands `must`
23. slackbuild_function is the build function. By build meaning everything needed to build (compile etc) package and every other command needed for $PKG but no cp docs, remove .la etc... Only what package need. `must`
24. `MAKEPKG_FLAGS` if its not set, then by default it use: `makepkg --remove-tmp-rpaths -l y -c n "$OUTPUT"/$PRGNAM-"$VERSION"-"$ARCH"-"$BUILD""$TAG"."$PKGTYPE"` `if_need`
> If you want to set other `MAKEPKG_FLAGS= ` please [READ](https://github.com/rizitis/PLASMA_WORLD/blob/main/AthOS/DOCS/MAKEPKG_FLAGS) . Example `MAKEPKG_FLAGS="-l y -c n"` or `MAKEPKG_FLAGS="--remove-rpaths"` etc..

---

## slack-desc
When you ran `sudo bash setup.sh` app (script) slackdesc will be symlinked in /usr/local/bin/slackdesc so you are fine. 
You may read [here](https://slack-desc.sourceforge.net) about it.<br>
If this tool is installed then no separate slack-desc files needed for your builds. Just add discription in pkg.ini and app slackdesc will create and install a valid slack-desc for the specific pkg you are building.<br>
You can always skip or rm  slackdesk script from your system and continue creatings slack-desc manually, athos will work with them then.<br>


### doinst.sh

If your build need a usual doinst.sh then **you dont** have to create it, athos is prepare for that also. <br>
Only in very special cases (update conf files etc...) that a specific doinst.sh must ran after installation, then you can create/place one manually in folder before build start and athos will respect it. 

### dounist.sh
If you need a dounist.sh create one in $CWD/dounist.sh and athos know what to do...

### patches

If patches needed then you must create a folder named **patches** and put patches in. athos will find them and apply to build as long they have extension .patch. Just be sure that folder is named **patches** and not `patch` or `patche`<br> 
If you deal with `diff`, `patch.gz`, etc; just add your patch commands on top of slackbuild() function before build commands.<br>

**NOTE**: athos know how to deal with Docs, .la files, man pages, Strip binaries - libraries, etc...<br>
In slackbuild() function DONT add these type of commands. Only add there what needed to build/repackage source tarbal is enough. 

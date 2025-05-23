![](./IMAGES/Commitsovertime.png)

Athos is Slackware package builder.<br>

---

## Install

*To install athos in a Slackware system as regular user follow these 3 steps*:

> Chose a place you want to clone repo (NOTE: this is permanent!)
0. `git clone https://github.com/rizitis/Athos.git`
> Get in folder and command
1. `sudo bash setup.sh`
> Add/Replace **Packager** and **TAG** names with yours
2. `nano /etc/Athos/athos.env`

Thats all.

---

## Usage

```
Usage:
  athos package_name             # To build an existing package
  athos package_name mkbuild     # To create a new package build template
  athos package_name mkedit      # To edit package ini file
  athos package_name mklog       # To view PackageBuild.log
  athos packane_name mkopen-dir  # To open package folder
  athos package_name mkdel-dir   # To delete package folder
  athos package_name mkls        # To view package folder contents
  athos package_name mkv         # To read package version and tarball url
  athos update                   # To rsync/update your local repository with mine here (optional)

  
```
**NOTE**: `athos update` is like `sbopkg -r` or `slpkg -u`, meaning it will *update/edit/remove* your local scripts (.ini files) and that is something you might do not want.<br>
     So this command is ok **ONLY** if you follow exactly my builds here and use it as remote repo, else if you use athos in a personal local repo please ignore this command.

- - -

### Explain Better

To build the package voo from *REPO-AthOS* 
- Just command `athos voo` 
It will build and install it (deps also if not already installed).

Assume you want to build the foo package that not exist in *REPO-AthOS*. 
1. STEP:
- Just command `athos foo mkbuild` 
The `mkbuild` option will create in *REPO-AthOS* a folder `foo` and a `foo.ini` file in it.<br>
You can say that `foo.ini` is the `foo.SlackBuild` ;)<br>

2. STEP
- Command `athos foo mkedit`
The `mkedit` option will open in your default text editor the `foo.ini` file for edit it.

3. STEP
- foo.ini is a very very simple SlackBuild TEMPLATE which I will now "split" it in 2 parts to be more explained:
Here is the first part. (informations for package tarball). <br>
```
Homepage="https://foo-project/homepage.com"
        #|--------------------------------------|#
SortDesc="foo sort description here respect rulle"
LongDesc="Foo long description here. You write everything in one line and you are fine. A valid slack-desc will be created"
VERSION=1.0.0
PRGNAM=foo
SRCNAM=foo
SRCURL=https://foo-project/downloads/foo-1.0.0.tar.gz
COMPRESS=tar.gz

# If has dependencies else empty or delete it.
REQUIRED="foo1 foo2 foo3"

BUILD=${BUILD:-1}
```

Following the second part, which is just 2 functions (commands for package build only).<br>

- One function for patches (even if not patches needed just leave it there)<br>
- And the main slackbuild() function.

We add in slackbuild() function **only** build commands. Nothing else needed.<br> 
Again, just build commands, everything else is athos automagicaly job, because athos know how to deal with *Docs*, *.la files*, *man pages*, *Strip binaries - libraries*, etc...

```
# This must be always here even if not patches needed.
patch_build() {
if [ -e $CWD/patches/ ]; then
  for PATCH in $CWD/patches/*.patch; do
    patch -p1 -i $PATCH || exit
  done
fi
}


# Just the build commands...
slackbuild() {

CFLAGS="$SLKCFLAGS" \
CXXFLAGS="$SLKCFLAGS" \
./configure \
  --prefix=/usr \
  --libdir=/usr/lib${LIBDIRSUFFIX} \
  --sysconfdir=/etc \
  --localstatedir=/var \
  --mandir=/usr/man \
  --docdir=/usr/doc/$PRGNAM-$VERSION \
  --infodir=/usr/info \
  --host=$ARCH-slackware-linux \
  --build=$ARCH-slackware-linux

make
make DESTDIR=$PKG install

}
```
*Everything you see above MUST always be there in every `ini` file. Its the minimum request for a valid foo.ini file.*

4. STEP
When ready foo.ini, build foo package.
- Just command `athos foo`


--- 

### HELP FILES
This is the minimum request for a vali foo.ini, but athos support many, many more options for *.ini files.
Complete informations you can find in [DOCS](https://github.com/rizitis/PLASMA_WORLD/tree/main/AthOS/DOCS)


---

### OUTPUT

When for example you build the package-1.6.2 by using `athos package` command, then in `/tmp` dir you can find:
- `/tmp/package`
- `/tmp/package.tar.lz4`
- `/tmp/package-build.log`

If you `ls /tmp/package` will show:
```
BIN_NAME  README  package-1.6.2-x86_64-1_rtz.tlz  package-1.6.2-x86_64-1_rtz.tlz.md5  package-1.6.2-x86_64-1_rtz.tlz.so.file  package-1.6.2-x86_64-1_rtz.tlz.txt  makepkg-package.log
```
This folder is compressed in `/tmp/package.tar.lz4` too.<br>
`/tmp/package-build.log` is also copied in `REPO-AthOS/package/` folder.


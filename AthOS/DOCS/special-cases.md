## special-cases 

Some time we face special cases for download source code...<br>
For example we might need to clone and checkout a repo for build,<br> or the source tar ball has a name like MY_PROJECT_NAME_0_18_9.tar.gz <br> or we have to build-repackage deb,rpm, AppImage etc...<br>  
Other case we have to create a package.desktop or when we download package-version.tar.?z and extract it, we dont see a folder package-version to cd in and build but we see direclty project's files-fodlers. <br>
All these not often happens, but happens. 
<br>
athos can face most of these cases, but in order to do it pkg.ini has to mention them.<p>

**Examples:**<br>
1. To git clone and git url in SRCURL and mention that `COMPRESS=git`<br> If you also want to git checout then add one line `GITCHEK="12345qwer4321sdfg"`
2. If source name is something like `MY_PROJECT_NAME_0_18_9.tar.gz` then in SRCNAM type all this ugly name `SRCNAM=MY_PROJECT_NAME_0_18_9` and ofcource in `COMPRESS=tar.gz`
3. For deb, rpm etc mention in `COMPRESS="deb"` and if needed also add one line `BIN_PKG=YES`
4. `BIN_PKG=YES` needed for sure when try build a package which `source.tar.?z*` after extracted  output provide directly the files/folders of source.
5. For package.desktop things are very simple just create a desktop_entry function and call it in pkg build, example:

```
desktop_entry() {
mkdir -p "$PKG"/usr/share/applications
cat <<EOF > "$PKG"/usr/share/applications/aegisub.desktop
[Desktop Entry]
Version=1.0
Name=Aegisub
GenericName=Subtitle Editor
Comment=Aegisub AppImage repackage
Type=Application
PATH=/usr/local/bin/aegisub/usr/bin
Exec=sh -c "LD_LIBRARY_PATH=/usr/local/bin/aegisub/usr/lib /usr/local/bin/aegisub/usr/bin/aegisub $SHELL"
Icon=/usr/share/icons/hicolor/48x48/apps/aegisub.png
Terminal=false
Categories=AudioVideo;AudioVideoEditing;
StartupNotify=true
MimeType=application/x-srt;text/plain;text/x-ass;text/x-microdvd;text/x-ssa;
StartupWMClass=aegisub
EOF
}
desktop_entry
```
6. `META_PKG=YES` This is a fake package-meta.ini that only build requireds and exist. See plasma6-meta kde6-meta etc...
7. `CHROOT=NO` Some times no chroot needed, use this in ini file. Note that means you also run athos as user no root.
8. Accepted sums are: `SHA512SUM=xxxxxxxxx`, `SHA256SUM=xxxxxxx` and `MDSCHK=xxxxx` or EMPTY for skip checksum
9. `SKIP_INSTALL=YES` if you only want to build but not install.
10. `MAKEPKG_FLAGS` if its not set, then by default it use: `makepkg --remove-tmp-rpaths -l y -c n "$OUTPUT"/$PRGNAM-"$VERSION"-"$ARCH"-"$BUILD""$TAG"."$PKGTYPE"`  
<br> If you want to set other `MAKEPKG_FLAGS= ` please [READ](https://github.com/rizitis/PLASMA_WORLD/blob/main/AthOS/DOCS/MAKEPKG_FLAGS) . Example `MAKEPKG_FLAGS="-l y -c n"` or `MAKEPKG_FLAGS="--remove-rpaths"` etc..
11. `LOCAL_FILE=YES` Assume you already have $SRCNAM.$COMPRESS in same directory with $PRGNAM.ini and skip download source.
---
====================================================
---

There is also one more very special case you probably never will need it but in some cases we are not allowed to download using wget or any other terminal command. But we can download from browser... Some remote repos yes do it :(<br>
For this case there is an option that make athos to interact remote as browser agent.
So add in your pkg.ini these lines:
```
AGENT=ON
# If AGENT is ON Define the download agent or special wget option...
DOWNLOAD_AGENT () {
wget -O "aegisub-v3.3.3-x86-64.AppImage" "$SRCURL" \
     --header="Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" \
     --header="Accept-Language: en-US,en;q=0.5" \
     --user-agent="Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:89.0) Gecko/20100101 Firefox/89.0"
}
```

If you can understand what this command do then you can modify it for your needs... As I said probably you will never face it. <br>
For the history SRCURL for the above example was this:
```
SRCURL="https://ocs-dl.fra1.cdn.digitaloceanspaces.com/data/files/1683370141/aegisub-v3.3.3-x86-64.AppImage?response-content-disposition=attachment%3B%2520aegisub-v3.3.3-x86-64.AppImage&X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=RWJAQUNCHT7V2NCLZ2AL%2F20241103%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20241103T200807Z&X-Amz-SignedHeaders=host&X-Amz-Expires=3600&X-Amz-Signature=7e13176b2e7def3ea861cdac150e55fc851b556a888c3ac26e1b3341c4eafb68"
```





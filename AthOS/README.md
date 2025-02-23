# ATHOS

> We're living the future so the present is our past.

## What is AthOS

AthOS is **not**:

* a package manager
- a package manager
+ a package manager

AthOS **is**:

1.  A different way to build packages for Slackware
2.  A more simple way to maintain a local repo of your build scripts

#### HELP FILES

More informations you can find in [DOCS](https://github.com/rizitis/PLASMA_WORLD/tree/main/AthOS/DOCS)

- - -
##### Commands

```
Usage:
  athos package_name        # To build an existing package
  athos package_name mkbuild # To create a new package build template
  athos package_name mkedit # To edit package ini file
  athos package_name mklog # To view PackageBuild.log
  athos packane_name mkopen-dir        # To open package folder
  athos package_name mkdel-dir      # To delete package folder
  athos package_name mkls   # To view package folder contents

  
```
To replace my name for Packager in all ini files open a terminal here in this
folder and command:

```
find ./ -type f -name "*.ini" -exec sed -i 's/Packager="Anagnostakis Ioannis <rizitis@gmail.com>"/Packager="New Name <newemail@example.com>"/g' {} +
```
Edit above command as needed for your name and email... `New Name
\<newemail@example.com>`

To change my $TAG `_rtz` with yours:

```
find ./ -type f -name "*.ini" -exec sed -i 's/_rtz/_newtag"/g' {} +
```
Also edit `_newtag` with your in cli^

- - -
In dev-stuff are some scripts that help me to maintain athos for plasma6
something that you might need is the  
`edit-ini-files-for-new-versions.shCURRENT-VERSIONS.envNEW-VERSIONS.env` 

1.  `6.2.5NEW-VERSIONS.env5`
2.  `sudo bash edit-ini-files-for-new-versions.shfile_list.log`
3.  `sudo bash edit-ini-files-for-new-versions.sh --edit`

- - -
#### `/usr/doc/$PRGNAM-$VERSION/slack-required`       

> 

- - -
![image](./cptn.png)


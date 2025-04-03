## step 1
If you want to build a new app named foo, command: 
<br> `athos foo mkbuild`
1. It will create a folder foo in REPO-AthOS
2. It will create the foo.ini in this folder

Now you can edit foo.ini and make it ready for build - install<p>
3. `athos foo mkedit`

---

## step 2
Assume foo.ini is ready, to build and install foo package, command:<br>
`athos foo`<br>


Note: If you have patches for foo, create a **patches** folder next to foo.ini and place them there.<br>
      If foo have depentencies make sure that you have them installed or create .ini following **step 1** so athos can find them and build them before foo. 

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

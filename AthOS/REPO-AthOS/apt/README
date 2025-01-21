Before build run pre-install script or manually create group and user for apt:

1. Add the _apt group

groupadd -r _apt

    The -r flag creates a system group (UID/GID < 1000).

2. Add the _apt user

useradd -r -M -d /var/lib/apt -s /sbin/nologin -g _apt -c "APT account for owning persistent & cache data" _apt

    -r → Creates a system user.
    -M → Does not create a home directory.
    -d /var/lib/apt → Sets the home directory to /var/lib/apt (but doesn't create it).
    -s /sbin/nologin → Prevents the user from logging in.
    -g _apt → Assigns the user to the _apt group.
    -c → Adds a description ("APT account for owning persistent & cache data").

After installation finish you must setup a sources.list in /etc/apt/
Assume you want Debian Testing , then add this line:
`deb http://http.us.debian.org/debian/ testing non-free contrib main`


If you need apt for your Slackware Doecker then this file must be configure in binary (apt.t?z)
In such case dependencies are more than what apt.ini discribe...
For bare metal this package not suggested because apt cant read your installed packages list so it will try to install everything discrided as dep in your system...and boom!
If you try to install apt packages in bare metal (thats crazy but...) then its better to use `apt download pkg_name` and then by using
`dpkg  --force-depends -i pkg_name*.deb` you are skipping dep resolution.... But still all these are stupid.
any way good luck...

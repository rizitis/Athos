#!/bin/sh

groupadd -r _apt
useradd -r -M -d /var/lib/apt -s /sbin/nologin -g _apt -c "APT account for owning persistent & cache data" _apt

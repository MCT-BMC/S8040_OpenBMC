#! /bin/bash

if [ -z "$1" ]
then
	        echo ""
		        echo "Syntax: fw_update.sh <file>"
			        exit 1
fi

echo -n "Update ESM firmware with: "
read -r -p "$(echo $@) ? [y/N] " YESNO
if [ "$YESNO" != "y" ]; then
	        echo >&2 "Aborting"
		        exit 1
fi

fw_setenv openbmconce copy-files-to-ram copy-base-filesystem-to-ram
fw_setenv openbmconce copy-files-to-ram copy-base-filesystem-to-ram
cp $1 /run/initramfs/image-bmc
reboot

#
# These file add version 

python() {
        d.setVar('VERSION', "v0.04-S8040")
        d.setVar('VERSION_ID', "v0.04-S8040")
        d.setVar('BMC_NAME', "Tyan-S8040")
}

#OS_RELEASE_FIELDS:append = "BMC_NAME"

# Ensure the git commands run every time bitbake is invoked.
BB_DONT_CACHE = "1"

# Make os-release available to other recipes.
#SYSROOT_DIRS:append = " ${sysconfdir}"

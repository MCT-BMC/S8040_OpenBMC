SUMMARY = "AMD LCD Display Interface Library"
DESCRIPTION = "AMD LCD display Interface Library to display user data"

FILESEXTRAPATHS_prepend := "${THISDIR}:"

LICENSE = "CLOSED"

DEPENDS += "i2c-tools"

SRC_URI = "git://git@github.com:/AMDESE/amd-lcd-lib.git;protocol=https"
#SRCREV = "${AUTOREV}"
SRCREV = "c3d79cbc0a5b43a9c5270471c3c33bcd86736df1"

S="${WORKDIR}/git"

inherit cmake

do_install () {
        install -d ${D}${libdir}
        cp --preserve=mode,timestamps -R ${B}/liblcdlib32* ${D}${libdir}/

        install -d ${D}${includedir}
        install -m 0644 ${S}/include/lcdlib/* ${D}${includedir}/
}

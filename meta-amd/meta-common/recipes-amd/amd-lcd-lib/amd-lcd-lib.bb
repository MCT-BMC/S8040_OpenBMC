SUMMARY = "AMD LCD Display Interface Library"
DESCRIPTION = "AMD LCD display Interface Library to display user data"

FILESEXTRAPATHS_prepend := "${THISDIR}:"

LICENSE = "CLOSED"

DEPENDS += "i2c-tools"

#SRC_URI = "git://git@github.com:/AMDESE/amd-lcd-lib.git;protocol=https"
SRC_URI = "git://git@micgitlab1.mic.com.tw/grdc/rdc5/rdd2/bmc/openbmc/amd/amd-lcd-lib.git;protocol=https"
SRCREV = "${AUTOREV}"

S="${WORKDIR}/git"

inherit cmake

do_install () {
        install -d ${D}${libdir}
        cp --preserve=mode,timestamps -R ${B}/liblcdlib32* ${D}${libdir}/

        install -d ${D}${includedir}
        install -m 0644 ${S}/include/lcdlib/* ${D}${includedir}/
}

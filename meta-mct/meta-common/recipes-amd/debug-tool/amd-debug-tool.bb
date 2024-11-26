SUMMARY = "AMD YAAPd Jtag server Application"
DESCRIPTION = "Yet Another AMD Protocol for Jtag communication with main Processor"

LICENSE = "CLOSED"

PV = "1.0+git${SRCPV}"

S="${WORKDIR}/git"

inherit systemd obmc-phosphor-systemd meson pkgconfig

# AMD Github repository
#SRC_URI  = "git://git@github.com/AMDESE/YAAP.git;branch=master;protocol=ssh"
SRC_URI  = "git://git@micgitlab1-ssh.mic.com.tw:1022/grdc/rdc5/rdd2/bmc/openbmc/amd/YAAP.git;branch=master;protocol=ssh"
SRC_URI += "file://0001-Revert-bmc-disable-unsafe-yaapPrivate-class.patch \
            file://0002-MCT-Set-specified-GPIO-setting-for-using-platform.patch \
            file://yaapd.service \
           "
#SRCREV = "${AUTOREV}"
SRCREV = "30852ddaeb8ebb94e34a02cef98215b2ed903732"

DEPENDS += "libgpiod"
DEPENDS += "boost"

SYSTEMD_SERVICE_${PN} = "yaapd.service "


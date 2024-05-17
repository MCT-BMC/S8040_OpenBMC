SUMMARY = "AMD RAS application to handle RAS errors from BMC"
DESCRIPTION = "The applications harvests and handles the RAS errors from the processor"

LICENSE = "CLOSED"

FILESEXTRAPATHS_prepend := "${THISDIR}:"

inherit meson
inherit pkgconfig
inherit systemd
inherit phosphor-mapper

def get_service(d):
    return "com.amd.crashdump.service"

SYSTEMD_SERVICE_${PN} = "${@get_service(d)}"
#SRC_URI = "git://git@github.com:/AMDESE/amd-bmc-ras.git;branch=main;protocol=ssh"
SRC_URI = "git://git@github.com:/AMDESE/amd-bmc-ras.git;branch=main;protocol=https"
SRCREV = "2ef58f0a1411ac188d72f13b047ac9c0a155b086"

S = "${WORKDIR}/git"

DEPENDS += " \
    amd-apml \
    i2c-tools \
    phosphor-dbus-interfaces \
    phosphor-logging \
    sdbusplus \
    libgpiod \
    boost \
    nlohmann-json \
    "

FILES_${PN} += "${systemd_unitdir}/system/com.amd.crashdump.service"

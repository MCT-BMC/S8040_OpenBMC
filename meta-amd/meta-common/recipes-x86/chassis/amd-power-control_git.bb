SUMMARY = "Chassis Power Control service for AMD platforms"
DESCRIPTION = "Chassis Power Control service for AMD platforms"

#SRC_URI = "git://git@github.com/AMDESE/amd-power-control.git;branch=integ;protocol=https"
SRC_URI = "git://git@micgitlab1.mic.com.tw/grdc/rdc5/rdd2/bmc/openbmc/amd/amd-power-control.git;branch=integ;protocol=https"
SRCREV = "${AUTOREV}"

PV = "1.0+git${SRCPV}"

S = "${WORKDIR}/git"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

inherit cmake systemd
inherit obmc-phosphor-dbus-service

SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.Chassis.Control.Power.service \
                         chassis-system-reset.service \
                         chassis-system-reset.target"

TARGET_LDFLAGS = " -lpthread -li2c"
DEPENDS += " \
    boost \
    i2c-tools \
    libgpiod \
    sdbusplus \
    phosphor-logging \
  "

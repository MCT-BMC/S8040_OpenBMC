SUMMARY = "AMD CPU Info"
DESCRIPTION = "CPU Info monitors the dbus interface\
xyz.openbmc_project.Inventory.Item.Cpu_info.service for Processor property \
and applies the CPU values to the SOC using esmi oob library API's"

#SRC_URI = "git://git@github.com/AMDESE/bmc-cpuinfo.git;branch=main;protocol=ssh"
SRC_URI = "git://git@github.com/AMDESE/bmc-cpuinfo.git;branch=main;protocol=https"
#SRCREV = "${AUTOREV}"
SRCREV = "f2f84a7f72fd8a6e15c1fad7b0a22d4178c6aa44"

S = "${WORKDIR}/git"

LICENSE = "CLOSED"

inherit cmake pkgconfig systemd

def get_service(d):
      return "xyz.openbmc_project.Inventory.Item.Cpu_info.service"

SYSTEMD_SERVICE_${PN} = "${@get_service(d)}"

DEPENDS += " \
    amd-apml \
    i2c-tools \
    libgpiod \
    phosphor-dbus-interfaces \
    phosphor-logging \
    sdbusplus \
    "

RDEPENDS_${PN} += "amd-apml"

FILES_${PN}  += "${systemd_system_unitdir}/xyz.openbmc_project.Inventory.Item.Cpu_info.service"




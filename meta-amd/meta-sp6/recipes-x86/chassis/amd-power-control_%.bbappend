FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
    file://0001-amd-power-control-sync-code-to-amd-power-control.patch \
    file://power-config-host0.json \
"
DEPENDS += " nlohmann-json "

do_install_append() {
    install -d ${D}/usr/share/x86-power-control/
    install -m 0644 ${WORKDIR}/power-config-host0.json ${D}/usr/share/x86-power-control/
}

SYSTEMD_SERVICE_${PN}_remove = " xyz.openbmc_project.Chassis.Control.Power.service "
SYSTEMD_SERVICE_${PN} += " xyz.openbmc_project.Chassis.Control.Power@0.service "

FILES_${PN} += " /lib/systemd/system/xyz.openbmc_project.Chassis.Control.Power@.service \
                 /usr/share/x86-power-control/power-config-host0.json"
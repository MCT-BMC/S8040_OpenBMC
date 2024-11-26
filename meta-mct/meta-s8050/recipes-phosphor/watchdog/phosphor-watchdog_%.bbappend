#FILESEXTRAPATHS_prepend_${PN} := "${THISDIR}/${PN}:"
FILESEXTRAPATHS_prepend_s8050 := "${THISDIR}/${PN}:"
SYSTEMD_OVERRIDE_${PN}_remove = "poweron.conf:phosphor-watchdog@poweron.service.d/poweron.conf"

SRC_URI += "    file://timeout@.service \
                file://timeout  \
                "
SYSTEMD_SERVICE_${PN} = "phosphor-watchdog.service"

do_install_append(){
        install -d ${systemd_unitdir}/system/
        install -m 0644 ${WORKDIR}/timeout@.service ${D}${systemd_unitdir}/system/
        install -d ${bindir}
        install -m 0766 ${WORKDIR}/timeout ${D}${bindir}/
}

FILES_${PN} += " ${systemd_unitdir}/system/timeout@.service ${bindir}/timeout"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://server.ttyVUART0.conf file://obmc-console@.service "


do_install_append() {
	rm -rf ${D}${sysconfdir}/${BPN}
	install -m 0755 -d ${D}${sysconfdir}/${BPN}
	install -m 0644 ${WORKDIR}/*.conf ${D}${sysconfdir}/${BPN}/
    install -m 0644 ${WORKDIR}/${PN}@.service ${D}${systemd_system_unitdir}
}

SYSTEMD_SERVICE_${PN}_remove = " obmc-console@ttyS0.service "
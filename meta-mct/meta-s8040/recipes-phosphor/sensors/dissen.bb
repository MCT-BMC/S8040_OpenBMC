FILESEXTRAPATHS_prepend_s8040 := "${THISDIR}/${PN}:"
DESCRIPTION = "application for fault monitor"
LICENSE = "CLOSED"
TARGET_CC_ARCH += "${LDFLAGS}"

SRC_URI += " \
            file://dissen-monitor.cpp \
            file://Makefile \
            file://dissen-monitor.service \
            file://sensors.json \
          "
S="${WORKDIR}"

DEPENDS += " boost sdbusplus systemd libgpiod nlohmann-json"
inherit pkgconfig obmc-phosphor-systemd

do_compile () {
    oe_runmake
}
do_print () {
  bberror "dissen..."
  bberror "${D}$/etc/DISSEN/"  
}
do_install () {


  install -d ${D}${bindir}/
	install -m 0755 ${S}/dissen-monitor ${D}${bindir}/

  install -d ${D}${datadir}/dissen/
	install -m 0644 ${S}/sensors.json ${D}${datadir}/dissen/



}

SYSTEMD_SERVICE_${PN} += " dissen-monitor.service "

FILES_${PN} += " ${bindir}/dissen-monitor ${datadir}/dissen/sensors.json"
addtask do_print after do_install

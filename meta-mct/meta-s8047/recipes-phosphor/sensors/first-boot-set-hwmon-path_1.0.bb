SUMMARY = "Setting Hwmon config path"
DESCRIPTION = "Setup Hwmon soft link for current platform"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit allarch systemd

SYSTEMD_SERVICE_${PN} = "first-boot-set-hwmon-path.service bind-hwmon.service"

SRC_URI = "file://${BPN}.sh file://${BPN}.service file://bind-hwmon.sh file://bind-hwmon.service"

S = "${WORKDIR}"

do_install() {
    sed "s/{MACHINE}/${MACHINE}/" -i ${BPN}.sh
    sed "s/{MACHINE}/${MACHINE}/" -i bind-hwmon.sh
    install -d ${D}${bindir} ${D}${systemd_system_unitdir}
    install ${BPN}.sh ${D}${bindir}/
    install bind-hwmon.sh ${D}${bindir}/
    install -m 644 ${BPN}.service ${D}${systemd_system_unitdir}/
    install -m 644 bind-hwmon.service ${D}${systemd_system_unitdir}/
}

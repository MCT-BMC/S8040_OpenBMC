FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
LICENSE = "CLOSED"
SUMMARY = "FRU Write IPMI Command"

TARGET_CC_ARCH += "${LDFLAGS}"

SRC_URI += " \
            file://ipmi-fru.cpp \
            file://Makefile \
           "
SRCREV = "${AUTOREV}"

PR = "r1"
SO_VERSION = "1"
S = "${WORKDIR}"

DEPENDS = " \
         boost \
         sdbusplus \
         phosphor-ipmi-host \
         phosphor-logging \
         "

inherit pkgconfig
inherit obmc-phosphor-ipmiprovider-symlink

do_compile () {
    oe_runmake
}

do_install() {
    install -d ${D}${libdir}/ipmid-providers
    install -m 0755 ${S}/libipmi-fru.so.${SO_VERSION} ${D}/${libdir}/ipmid-providers/
}


HOSTIPMI_PROVIDER_LIBRARY += "libipmi-fru.so"
FILES_${PN} += " ${libdir}/ipmid-providers/lib*${SOLIBS}"
FILES_${PN} += " ${libdir}/host-ipmid/lib*${SOLIBS}"

RDEPENDS_${PN} += " phosphor-ipmi-host"
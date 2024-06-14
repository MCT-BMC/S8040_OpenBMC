DEPENDS_append_sp6 = " sp6-yaml-config"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-set-property-to-dbus-about-fwchanged.patch \
            "
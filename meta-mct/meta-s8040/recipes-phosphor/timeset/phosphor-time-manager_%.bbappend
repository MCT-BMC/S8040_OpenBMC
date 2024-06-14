DEPENDS_append_s8040 = " sp6-yaml-config"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-set-property-to-dissen-dbus-when-run-setTime.patch \
            "

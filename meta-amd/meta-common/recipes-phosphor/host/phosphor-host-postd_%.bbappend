FILESEXTRAPATHS_prepend := "${THISDIR}/phosphor-host-postd:"

S = "${WORKDIR}/git"

SNOOP_DEVICE = "aspeed-lpc-pcc"
POST_CODE_BYTES = "4"

SRC_URI += "file://0001-espi-post-code-capture-handler.patch \
            file://0002-fix-to-return-four-byte-post-code.patch \
            file://0003-Adjust-for-2-and-4-byte-POST-code-pat.patch \
            "


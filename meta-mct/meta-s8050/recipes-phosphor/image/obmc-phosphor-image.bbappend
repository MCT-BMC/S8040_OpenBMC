inherit extrausers

IMAGE_INSTALL += "phosphor-ipmi-ipmb"

DEFAULT_OPENBMC_PASSWORD = "'\$1\$KdSxTq5i\$W7g5v8sWXDsF0zvh.8Rk61'"

# AMD
OBMC_IMAGE_EXTRA_INSTALL_append = " \
    amd-debug-tool \
"

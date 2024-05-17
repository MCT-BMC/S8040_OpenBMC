FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRCREV = "44a8c618c1215e0faac0f335f0afd56ed4240e76"

SRC_URI += "file://amd-ast2600-u-boot.cfg \
            file://0001-u-boot-aspeed-sdk-add-Initial-Hawaii-device-tree.patch \
            file://0002-u-boot-aspeed-sdk-add-spi-nor-MT25Q01G-set-MAC-addr-BMC-Root-cause.patch \
            file://0003-u-boot-aspeed-sdk-add-platform-based-board_id-and-hostname.patch \
            file://0004-u-boot-aspeed-sdk-modify-Platform-based-device-tree-selection.patch \
            file://0005-u-boot-add-new-env-param-for-all-platforms.patch \
            file://0006-u-boot-Turin-APML-over-i2c-i3c.patch  \
            file://0007-u-boot-SP5-APML-over-i2c-i3c.patch \
            file://0008-u-boot-Add-Turin-Volcano-with-4-Pump-Fans.patch \
            file://0009-Fix-execution-path-for-non-NCSI-MAC-controller.patch \
            file://0010-only-enable-LAN-related-dts-and-necessary-configuatr.patch \
            file://0011-fix-board-id-to-63-Cinnabar-CRB-and-set-related-boot.patch \
            file://0012-RM-129419-ACPI-Bootloader-Enable-ACPI-and-GPIO0123-pass-thru-in-bootloader.patch \
            "

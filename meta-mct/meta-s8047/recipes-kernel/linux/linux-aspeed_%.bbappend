FILESEXTRAPATHS_prepend := "${THISDIR}/linux-aspeed:"

SRC_URI += "file://sp6.cfg \
            file://0001-Add-adc-hwmon-in-dts.patch \
            file://0002-upgrade_aspeed_adc_for_AMD.patch \
            file://0003-Assign-GPIO-line-name.patch \
	    file://0004-Add-AST2600-fan-driver-and-config-fan-dts.patch \
	    file://0005-RM-131314-Linux-Enable-NCSI-Feature.patch \
            file://0006-RM-132989-kernel-Enable-BMC-vuart1.patch \
            file://0007-GPIOS5-use-gpio-hog-for-VBAT-sensor.patch \
            file://0008-If-fan-speed-is-no-reading-return-ENXIO.patch \
            file://0009-Remove-1-fan-for-mitac-platform.patch \
            file://0010-Kernel-Setting-for-Bios-Update.patch \
            file://0011-Resolve-list-corruption-issue-in-PWM-driver-re-bind.patch \
            file://0012-fix-s8047-fan-mapping-in-dts.patch \
           "

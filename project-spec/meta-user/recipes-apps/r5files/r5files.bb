#
# This file is the r5files recipe.
#

SUMMARY = "Simple r5files application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://r5control.sh \
	file://r5-openamp-echo.elf \
	file://sensorhub.elf \
	"

S = "${WORKDIR}"
INSANE_SKIP_${PN} = "arch"

do_install() {
		install -d ${D}${sysconfdir}/common
		install -m 0755 ${S}/r5control.sh ${D}${sysconfdir}/common
		install -d ${D}/lib/firmware
		install -m 0644 ${S}/r5-openamp-echo.elf ${D}/lib/firmware
		install -m 0644 ${S}/sensorhub.elf ${D}/lib/firmware
}

RDEPENDS_${PN}_append += "bash"

FILES_${PN} = "${sysconfdir}/common/r5control.sh \
		/lib/firmware/r5-openamp-echo.elf \
		/lib/firmware/sensorhub.elf \
		"
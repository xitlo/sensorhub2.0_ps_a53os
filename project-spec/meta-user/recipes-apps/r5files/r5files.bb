#
# This file is the r5files recipe.
#

SUMMARY = "Simple r5files application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://r5control.sh \
	file://sensorhub.elf \
	file://amptest-0.elf \
	file://amptest-50.elf \
	"

S = "${WORKDIR}"
INSANE_SKIP_${PN} = "arch"

do_install() {
		install -d ${D}${sysconfdir}/common
		install -m 0755 ${S}/r5control.sh ${D}${sysconfdir}/common
		install -d ${D}/lib/firmware
		install -m 0644 ${S}/sensorhub.elf ${D}/lib/firmware
		install -m 0644 ${S}/amptest-0.elf ${D}/lib/firmware
		install -m 0644 ${S}/amptest-50.elf ${D}/lib/firmware
}

RDEPENDS_${PN}_append += "bash"

FILES_${PN} = "${sysconfdir}/common/r5control.sh \
		/lib/firmware/sensorhub.elf \
		/lib/firmware/amptest-0.elf \
		/lib/firmware/amptest-50.elf \
		"
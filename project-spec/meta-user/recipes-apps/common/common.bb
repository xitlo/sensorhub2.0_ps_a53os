#
# This file is the common recipe.
#

SUMMARY = "Simple common application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://deploy.sh \
	file://sensorhub2-config.json \
	file://zlog.conf \
	file://test_time.sh \
	file://autostart.sh-time \
	file://sensorhub-2.elf \
	file://openamp-script.sh \
	"

S = "${WORKDIR}"
INSANE_SKIP_${PN} = "arch"

do_install() {
		install -d ${D}${sysconfdir}/common
		install -m 0755 ${S}/deploy.sh ${D}${sysconfdir}/common
		install -m 0644 ${S}/sensorhub2-config.json ${D}${sysconfdir}/common
		install -m 0644 ${S}/zlog.conf ${D}${sysconfdir}/common
		install -m 0755 ${S}/test_time.sh ${D}${sysconfdir}/common
		install -m 0755 ${S}/autostart.sh-time ${D}${sysconfdir}/common
		install -m 0755 ${S}/openamp-script.sh ${D}${sysconfdir}/common
		install -d ${D}/lib/firmware
		install -m 0644 ${S}/sensorhub-2.elf ${D}/lib/firmware/sensorhub-2.elf
}

RDEPENDS_${PN}_append += "bash"
FILES_${PN} = "/lib/firmware/sensorhub-2.elf"

FILES_${PN} = "${sysconfdir}/common/deploy.sh \
		${sysconfdir}/common/sensorhub2-config.json \
		${sysconfdir}/common/zlog.conf \
		${sysconfdir}/common/test_time.sh \
		${sysconfdir}/common/autostart.sh-time \
		/lib/firmware/sensorhub-2.elf \
		${sysconfdir}/common/openamp-script.sh \
		"
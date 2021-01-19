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
	"

S = "${WORKDIR}"

do_install() {
		install -d ${D}${sysconfdir}/common
		install -m 0755 ${S}/deploy.sh ${D}${sysconfdir}/common
		install -m 0644 ${S}/sensorhub2-config.json ${D}${sysconfdir}/common
		install -m 0644 ${S}/zlog.conf ${D}${sysconfdir}/common
		install -m 0755 ${S}/test_time.sh ${D}${sysconfdir}/common
}

#
# This file is the zlog recipe.
#

SUMMARY = "Simple zlog application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://libzlog.so \
	file://libzlog.so.1 \
	file://libzlog.so.1.1.0 \
	"

S = "${WORKDIR}"

do_install() {
	     install -d ${D}/${libdir}
	     install -m 0755 ${S}/libzlog.so ${D}/${libdir}
	     install -m 0755 ${S}/libzlog.so.1 ${D}/${libdir}
	     install -m 0755 ${S}/libzlog.so.1.1.0 ${D}/${libdir}
}

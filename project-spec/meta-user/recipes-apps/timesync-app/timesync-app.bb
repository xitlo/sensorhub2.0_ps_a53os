#
# This file is the timesync-app recipe.
#

SUMMARY = "Simple timesync-app application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "zlog"

SRC_URI = "file://timesync-app.c \
	   file://log.c \
	   file://log.h \
	   file://zlog.h \
	   file://Makefile \
		  "

S = "${WORKDIR}"

do_compile() {
	     oe_runmake
}

do_install() {
	     install -d ${D}${bindir}
	     install -m 0755 timesync-app ${D}${bindir}
}

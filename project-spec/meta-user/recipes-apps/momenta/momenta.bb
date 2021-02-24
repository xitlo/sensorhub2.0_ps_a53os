#
# This file is the momenta recipe.
#

SUMMARY = "Simple momenta application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "zlog"

SRC_URI = "file://dir-common/log.c \
		file://dir-common/log.h \
		file://dir-common/zlog.h \
		file://dir-common/common.h \
		file://dir-common/cJSON.h \
		file://dir-common/cJSON.c \
		file://dir-task-data/task-data.c \
		file://dir-task-state/task-state.c \
		file://dir-timesync-app/timesync-app.c \
		file://dir-config-parse/config-parse.c \
		file://Makefile \
		  "

S = "${WORKDIR}"

do_compile() {
	     oe_runmake
}

do_install() {
	     install -d ${D}${bindir}
	     install -m 0755 task-data ${D}${bindir}
	     install -m 0755 task-state ${D}${bindir}
	     install -m 0755 timesync-app ${D}${bindir}
	     install -m 0755 config-parse ${D}${bindir}
}

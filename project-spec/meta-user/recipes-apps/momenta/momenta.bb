#
# This file is the momenta recipe.
#

SUMMARY = "Simple momenta application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "zlog"

SRC_URI = "file://common/log.c \
		file://common/log.h \
		file://common/zlog.h \
		file://common/common.h \
		file://common/cJSON.h \
		file://common/cJSON.c \
		file://common/mem-mmap.h \
		file://common/mem-mmap.c \
		file://task-data/task-data.c \
		file://task-state/task-state.c \
		file://timesync-app/timesync-app.c \
		file://config-parse/config-parse.c \
		file://Makefile \
		  "

S = "${WORKDIR}"

do_compile() {
	     oe_runmake
}

do_install() {
	     install -d ${D}${bindir}
	     install -m 0755 task-data/task-data ${D}${bindir}
	     install -m 0755 task-state/task-state ${D}${bindir}
	     install -m 0755 timesync-app/timesync-app ${D}${bindir}
	     install -m 0755 config-parse/config-parse ${D}${bindir}
}

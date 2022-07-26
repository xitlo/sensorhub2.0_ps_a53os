#
# This file is the isp-monitor recipe.
#

SUMMARY = "Simple isp monitor application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://api_cmd.c \
	   file://api.h \
	   file://base_types.h \
	   file://connection_ftdi4222.c \
	   file://connection_serial.c \
	   file://connection.h \
	   file://gw5_failures.h \
	   file://Makefile \
	   file://platform.c \
	   file://platform.h \
	   file://protocol_i2c.c \
	   file://protocol_uart.c \
	   file://protocol.h \
	   file://tools_rev.h \
	   file://w5_com_platform.h \
	   file://w5_com.c \
	   file://w5_com.h \
	   file://w5_connection.h \
		  "

S = "${WORKDIR}"

do_compile() {
#	     oe_runmake
}

do_install() {
#	     install -d ${D}${bindir}
#	     install -m 0755 isp_monitor ${D}${bindir}
}

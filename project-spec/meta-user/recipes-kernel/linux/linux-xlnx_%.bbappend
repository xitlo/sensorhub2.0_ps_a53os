FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://devtool-fragment.cfg"
SRC_URI += "file://virtio_rpmsg_bus.c"
SRC_URI += "file://mailbox_controller.h"

do_configure_append () {
	install ${WORKDIR}/virtio_rpmsg_bus.c ${S}/drivers/rpmsg/
	install ${WORKDIR}/mailbox_controller.h ${S}/include/linux/
}


FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://devtool-fragment.cfg"
SRC_URI += "file://virtio_rpmsg_bus.c"

do_configure_append () {
	install ${WORKDIR}/virtio_rpmsg_bus.c ${S}/drivers/rpmsg/
}


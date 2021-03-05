FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://platform-top.h"
SRC_URI += "file://bsp.cfg"
SRC_URI += "file://cmds.c"
SRC_URI += "file://source.c"
SRC_URI += "file://phy.c"

do_configure_append () {
	if [ "${U_BOOT_AUTO_CONFIG}" = "1" ]; then
		install ${WORKDIR}/platform-auto.h ${S}/include/configs/
		install ${WORKDIR}/platform-top.h ${S}/include/configs/
	else
		install ${WORKDIR}/bsp.cfg ${S}/include/configs/
	fi
	install ${WORKDIR}/cmds.c ${S}/board/xilinx/zynqmp/
	install ${WORKDIR}/source.c ${S}/cmd/
	install ${WORKDIR}/phy.c ${S}/drivers/net/phy/
}

do_configure_append_microblaze () {
	if [ "${U_BOOT_AUTO_CONFIG}" = "1" ]; then
		install -d ${B}/source/board/xilinx/microblaze-generic/
		install ${WORKDIR}/config.mk ${B}/source/board/xilinx/microblaze-generic/
	fi
}

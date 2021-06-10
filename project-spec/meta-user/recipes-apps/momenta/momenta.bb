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
		file://ampperf/ampperf.c \
		file://ampecho/ampecho.c \
		file://Makefile \
		file://tools/app-control.sh \
		file://tools/deploy-emmc.sh \
		file://tools/deploy-qspi.sh \
		file://tools/test_time.sh \
		file://tools/zlog.conf \
		file://tools/sensorhub2-config.json \
		file://tools/autostart.sh-time \
		file://tools/api_cmd_test.sh \
		file://tools/cam_ver.sh \
		file://tools/ota-update.sh \
		file://tools/ispota.sh \
		file://tools/table_ispota.sh \
		file://tools/authorized_keys \
		file://tools/selftest.sh \
		file://tools/irq-affinity.sh \
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
		install -m 0755 ampperf/ampperf ${D}${bindir}
		install -m 0755 ampecho/ampecho ${D}${bindir}
		install -d ${D}${sysconfdir}/common
		install -m 0755 tools/app-control.sh ${D}${sysconfdir}/common
		install -m 0755 tools/deploy-emmc.sh ${D}${sysconfdir}/common
		install -m 0755 tools/deploy-qspi.sh ${D}${sysconfdir}/common
		install -m 0755 tools/test_time.sh ${D}${sysconfdir}/common
		install -m 0644 tools/zlog.conf ${D}${sysconfdir}/common
		install -m 0644 tools/sensorhub2-config.json ${D}${sysconfdir}/common
		install -m 0755 tools/autostart.sh-time ${D}${sysconfdir}/common
		install -m 0755 tools/api_cmd_test.sh ${D}${sysconfdir}/common
		install -m 0755 tools/cam_ver.sh ${D}${sysconfdir}/common
		install -m 0755 tools/ota-update.sh ${D}${sysconfdir}/common
		install -m 0755 tools/ispota.sh ${D}${sysconfdir}/common
		install -m 0755 tools/table_ispota.sh ${D}${sysconfdir}/common
		install -m 0644 tools/authorized_keys ${D}${sysconfdir}/common
		install -m 0755 tools/selftest.sh ${D}${sysconfdir}/common
		install -m 0755 tools/irq-affinity.sh ${D}${sysconfdir}/common
}

RDEPENDS_${PN}_append += "bash"

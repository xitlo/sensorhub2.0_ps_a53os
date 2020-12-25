#
# This file is the iperf3 recipe.
#

SUMMARY = "Simple iperf3 application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://iperf3 \
        file://libiperf.so \
        file://libiperf.so.0 \
        file://libiperf.so.0.0.0 \
        "

S = "${WORKDIR}"
#DEPENDS = "libssl.so.1.1 libcrypto.so.1.1 libcrypt.so.2"

do_install() {
             install -d ${D}/${bindir}
             install -m 0755 ${S}/iperf3 ${D}/${bindir}
             install -d ${D}/${libdir}
             install -m 0644 ${S}/libiperf.so ${D}/${libdir}
             install -m 0644 ${S}/libiperf.so.0 ${D}/${libdir}
             install -m 0644 ${S}/libiperf.so.0.0.0 ${D}/${libdir}
}

FILES_${PN} = "${bindir}/iperf3 \
        ${libdir}/libiperf.so \
        ${libdir}/libiperf.so.0 \
        ${libdir}/libiperf.so.0.0.0 \
        "


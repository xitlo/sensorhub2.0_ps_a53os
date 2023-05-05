#
# This file is the momenta recipe.
#

SUMMARY = "Simple momenta application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://tools/deploy-emmc.sh \
		file://tools/deploy-qspi.sh \
		file://tools/ota-update.sh \
		"

S = "${WORKDIR}"

do_install() {
		install -d ${D}${bindir}	
		install -d ${D}${sysconfdir}/common		
		install -m 0755 tools/deploy-emmc.sh ${D}${sysconfdir}/common
		install -m 0755 tools/deploy-qspi.sh ${D}${sysconfdir}/common				
		install -m 0755 tools/ota-update.sh ${D}${sysconfdir}/common
	  
}


# The dropbear_%.bbappend looks like this:
# Dropbear: suppress gen_keys.

SRC_URI += " \
		file://dropbear_init \
		file://dropbear_pam \
		file://dropbear_rsa_host_key \
		"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

# Overwrite the dropbear configuration with my configuration.
do_install_append() {
	install -m 0755 ${WORKDIR}/dropbear_init ${D}${sysconfdir}/init.d/dropbear
	install -m 0644 ${WORKDIR}/dropbear_pam ${D}${sysconfdir}/pam.d/dropbear
	install -m 0644 ${WORKDIR}/dropbear_rsa_host_key ${D}${sysconfdir}/dropbear
}
# The dropbear_%.bbappend looks like this:
# Dropbear: suppress gen_keys.

SRC_URI += " \
		file://dropbear \
		"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

# Overwrite the dropbear configuration with my configuration.
do_install_append() {
	install -m 0755 ${WORKDIR}/dropbear ${D}${sysconfdir}/init.d/dropbear
}
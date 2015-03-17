deps_config := \
	util-linux/Config.in \
	sysklogd/Config.in \
	shell/Config.in \
	procps/Config.in \
	networking/udhcp/Config.in \
	networking/Config.in \
	modutils/Config.in \
	miscutils/Config.in \
	loginutils/Config.in \
	init/Config.in \
	findutils/Config.in \
	editors/Config.in \
	e2fsprogs/Config.in \
	debianutils/Config.in \
	console-tools/Config.in \
	coreutils/Config.in \
	archival/Config.in \
	/home/shearer/Projects/WAP4410N/apps/busybox-1.1.0/sysdeps/linux/Config.in

.config include/config.h: $(deps_config)

$(deps_config):

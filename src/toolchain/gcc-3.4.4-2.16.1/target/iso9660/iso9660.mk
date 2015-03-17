#############################################################
#
# mkisofs to build to target iso9660 filesystems
#
#############################################################
MKISOFS_SOURCE:=cdrtools-2.01.tar.bz2
MKISOFS_SITE:=ftp://ftp.berlios.de/pub/cdrecord/
MKISOFS_DIR:=$(BUILD_DIR)/cdrtools-2.01
MKISOFS_TARGET:=$(MKISOFS_DIR)/mkisofs/OBJ/i686-linux-cc/mkisofs

$(DL_DIR)/$(MKISOFS_SOURCE):
	$(WGET) -P $(DL_DIR) $(MKISOFS_SITE)/$(MKISOFS_SOURCE)

mkisofs-source: $(DL_DIR)/$(MKISOFS_SOURCE)

$(MKISOFS_DIR)/.unpacked: $(DL_DIR)/$(MKISOFS_SOURCE)
	bzcat $(DL_DIR)/$(MKISOFS_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	toolchain/patch-kernel.sh $(MKISOFS_DIR) target/iso9660/ \*.patch
	touch $(MKISOFS_DIR)/.unpacked

$(MKISOFS_DIR)/.configured: $(MKISOFS_DIR)/.unpacked
	(cd $(MKISOFS_DIR); rm -rf config.cache; \
	);
	touch  $(MKISOFS_DIR)/.configured

$(MKISOFS_TARGET): $(MKISOFS_DIR)/.configured
	$(MAKE) -C $(MKISOFS_DIR)
	touch -c $(MKISOFS_DIR)/mkisofs

mkisofs: $(MKISOFS_TARGET)

mkisofs-clean:
	-$(MAKE) -C $(MKISOFS_DIR) clean

mkisofs-dirclean:
	rm -rf $(MKISOFS_DIR)


#############################################################
#
# Build the iso96600 root filesystem image
#
#############################################################

ISO9660_TARGET_DIR=$(BUILD_DIR)/iso9660
ISO9660_TARGET:=$(subst ",,$(BR2_TARGET_ROOTFS_ISO9660_OUTPUT))
ISO9660_BOOT_MENU:=$(subst ",,$(BR2_TARGET_ROOTFS_ISO9660_BOOT_MENU))

ISO9660_OPTS:=

ifeq ($(strip $(BR2_TARGET_ROOTFS_ISO9660_SQUASH)),y)
ISO9660_OPTS+=-U
endif

$(ISO9660_TARGET): host-fakeroot $(EXT2_TARGET) grub mkisofs
	mkdir -p $(ISO9660_TARGET_DIR)
	mkdir -p $(ISO9660_TARGET_DIR)/boot/grub
	cp $(GRUB_DIR)/stage2/stage2_eltorito $(ISO9660_TARGET_DIR)/boot/grub/
	cp $(ISO9660_BOOT_MENU) $(ISO9660_TARGET_DIR)/boot/grub/menu.lst
	cp $(LINUX_KERNEL) $(ISO9660_TARGET_DIR)/kernel
	cp $(EXT2_TARGET) $(ISO9660_TARGET_DIR)/initrd
	# Use fakeroot to pretend all target binaries are owned by root
	rm -f $(STAGING_DIR)/_fakeroot.$(notdir $(ISO9660_TARGET))
	touch $(STAGING_DIR)/.fakeroot.00000
	cat $(STAGING_DIR)/.fakeroot* > $(STAGING_DIR)/_fakeroot.$(notdir $(ISO9660_TARGET))
	echo "chown -R root:root $(ISO9660_TARGET_DIR)" >> $(STAGING_DIR)/_fakeroot.$(notdir $(ISO9660_TARGET))
	# Use fakeroot so mkisofs believes the previous fakery
	echo "$(MKISOFS_TARGET) -R -b boot/grub/stage2_eltorito -no-emul-boot " \
		"-boot-load-size 4 -boot-info-table -o $(ISO9660_TARGET) $(ISO9660_TARGET_DIR)" \
		>> $(STAGING_DIR)/_fakeroot.$(notdir $(ISO9660_TARGET))
	chmod a+x $(STAGING_DIR)/_fakeroot.$(notdir $(ISO9660_TARGET))
	$(STAGING_DIR)/usr/bin/fakeroot -- $(STAGING_DIR)/_fakeroot.$(notdir $(ISO9660_TARGET))
	-@rm -f $(STAGING_DIR)/_fakeroot.$(notdir $(ISO9660_TARGET))

iso9660root: $(ISO9660_TARGET)
	echo $(ISO9660_TARGET)
	@ls -l $(ISO9660_TARGET)

iso9660root-source: mkisofs-source

iso9660root-clean: mkisofs-clean

iso9660root-dirclean: mkisofs-dirclean
	rm -rf $(ISO9660_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_TARGET_ROOTFS_ISO9660)),y)
TARGETS+=iso9660root
endif

include $(TOPDIR)/rules.mk

PKG_NAME:=prog_for_mint
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/prog_for_mint
	CATEGORY:=Base system
	TITLE:=prog_for_mint
	DEPENDS:=+libmosquitto +libubus +libubox +libblobmsg-json
endef

define Package/prog_for_mint/description
	My lil' ol' prog for my home assistant integration
endef

define Package/prog_for_mint/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/prog_for_mint $(1)/usr/bin
	$(INSTALL_BIN) ./files/prog_for_mint.init $(1)/etc/init.d/prog_for_mint
	$(INSTALL_CONF) ./files/prog_for_mint.config $(1)/etc/config/prog_for_mint
endef

$(eval $(call BuildPackage,prog_for_mint))

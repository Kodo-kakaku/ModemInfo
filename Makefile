include $(TOPDIR)/rules.mk

PKG_NAME:=ModemInfo
PKG_VERSION:=1.7
PKG_RELEASE:=1

PKG_MAINTAINER:=Vladislav Kadulin <spanky@yandex.ru>
PKG_LICENSE:=GPLv3


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)
        SECTION:=utils
        DEFAULT:=y
        CATEGORY:=Utilities
        TITLE:=The $(PKG_NAME) was specially developed for the luci-app-modeminfo backend.
        DEPENDS:=$(DRV_DEPENDS) +libstdcpp +cJSON +glib2 +libqmi
endef

define Package/$(PKG_NAME)/description
        The $(PKG_NAME) was specially developed for the luci-app-modeminfo backend.
endef

define Build/Prepare
        mkdir -p $(PKG_BUILD_DIR)
        $(CP) ./$(PKG_NAME)/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
        $(INSTALL_DIR) $(1)/usr/bin
        $(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
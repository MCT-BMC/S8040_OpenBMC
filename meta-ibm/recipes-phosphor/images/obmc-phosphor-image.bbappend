OBMC_IMAGE_EXTRA_INSTALL_append_ibm-ac-server = " mboxd max31785-msl phosphor-msl-verify liberation-fonts uart-render-controller first-boot-set-hostname"
OBMC_IMAGE_EXTRA_INSTALL_append_p10bmc = " mboxd ibmtpm2tss"
OBMC_IMAGE_EXTRA_INSTALL_append_witherspoon-tacoma = " ibmtpm2tss"
OBMC_IMAGE_EXTRA_INSTALL_append_mihawk = " mboxd liberation-fonts uart-render-controller "

# remove so things fit in available flash space
IMAGE_FEATURES_remove_witherspoon = "obmc-user-mgmt-ldap"

# Optionally configure IBM service accounts
#
# To configure your distro, add the following line to its config:
#     DISTRO_FEATURES += "ibm-service-account-policy"
#
# The service account policy is as follows:
#   root - The root account remains present.  It is needed for internal
#     accounting purposes and for debugging service access.
#   admin - Provides administrative control over the BMC.  The role is
#     SystemAdministrator.  Admin users have access to interfaces including:
#     Redfish, REST APIs, Web.  No access to the BMC via: the BMC's physical
#     console, SSH to the BMC's command line.
#     IPMI access is not granted by default, but admins can authorize
#     themselves and enable the IPMI service.
#     The admin has access to the host console: ssh -p2200 admin@${bmc}.
#     The admin account does not have a home directory.
#   service - Provides IBM service and support representatives (SSRs, formerly
#     known as customer engineers or CEs) access to the BMC.  The role is
#     OemIBMServiceAgent.  The service user has full admin access, plus access
#     to BMC interfaces intended only to service the BMC and host, including
#     SSH access to the BMC's command line.
#     The service account is not authorized to IPMI because of the inherent
#     security weakness in the IPMI spec and also because the IPMI
#     implementation was not enhanced to use the ACF support.
#     The service account does not have a home directory.  The home directory is
#     set to / (the root directory) to allow dropbear ssh connections.

inherit extrausers

# The password hash used here is the traditional 0penBmc password.

#IBM_EXTRA_USERS_PARAMS += " \
#  usermod -p '\$1\$UGMqyqdG\$FZiylVFmRRfl9Z0Ue8G7e/' root; \
#  "

# Add group "wheel" (before adding the "service" account).
IBM_EXTRA_USERS_PARAMS += " \
  groupadd wheel; \
  "

# Add the "admin" account.
IBM_EXTRA_USERS_PARAMS += " \
  useradd -M -d / --groups priv-admin,redfish,web -s /sbin/nologin admin; \
  usermod -p '\$1\$UGMqyqdG\$FZiylVFmRRfl9Z0Ue8G7e/' admin; \
  "

# Add the "service" account.
IBM_EXTRA_USERS_PARAMS += " \
  useradd -M -d / --groups priv-admin,redfish,web,wheel service; \
  usermod -p '\$1\$UGMqyqdG\$FZiylVFmRRfl9Z0Ue8G7e/' service; \
  "

# This is recipe specific to ensure it takes effect.
EXTRA_USERS_PARAMS_pn-obmc-phosphor-image += "${@bb.utils.contains('DISTRO_FEATURES', 'ibm-service-account-policy', "${IBM_EXTRA_USERS_PARAMS}", '', d)}"

# The service account needs sudo.
IMAGE_INSTALL_append = " ${@bb.utils.contains('DISTRO_FEATURES', 'ibm-service-account-policy', 'sudo', '', d)}"
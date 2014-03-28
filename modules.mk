mod_steg.la: mod_steg.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_steg.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_steg.la

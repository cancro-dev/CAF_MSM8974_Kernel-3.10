# MSM8974
   zreladdr-$(CONFIG_ARCH_MSM8974)	:= 0x00008000

ifeq ($(CONFIG_MACH_LGE),n)
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v1-cdp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v1-fluid.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v1-liquid.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v1-mtp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v1-rumi.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v1-sim.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v2.0-1-cdp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v2.0-1-fluid.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v2.0-1-liquid.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v2.0-1-mtp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= apq8074-v2.0-1-cdp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= apq8074-v2.0-1-liquid.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= apq8074-v2.0-1-dragonboard.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= apq8074-v2.2-cdp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= apq8074-v2.2-liquid.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= apq8074-v2.2-dragonboard.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v2.2-cdp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v2.2-fluid.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v2.2-liquid.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974-v2.2-mtp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974pro-ab-pm8941-cdp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974pro-ab-pm8941-fluid.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974pro-ab-pm8941-fluid-hbtp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974pro-ab-pm8941-liquid.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974pro-ab-pm8941-mtp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974pro-ac-pm8941-cdp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974pro-ac-pm8941-liquid.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974pro-ac-pm8941-mtp.dtb
        dtb-$(CONFIG_ARCH_MSM8974)	+= msm8974pro-ac-pma8084-pm8941-mtp.dtb
else ifneq ($(CONFIG_DTS_TARGET),"")
        dtb-$(CONFIG_MACH_MSM8974_G3_KR)   += msm8974pro-g3-kr.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_KR)   += msm8974pro-ac-g3-kr.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_VZW)  += msm8974pro-g3-vzw.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_VZW)  += msm8974pro-ac-g3-vzw.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_ATT)  += msm8974pro-g3-att.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_ATT)  += msm8974pro-ac-g3-att.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_TMO_US)  += msm8974pro-g3-tmo_us.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_TMO_US)  += msm8974pro-ac-g3-tmo_us.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_SPR_US)  += msm8974pro-g3-spr_us.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_SPR_US)  += msm8974pro-ac-g3-spr_us.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_GLOBAL_COM)  += msm8974pro-g3-global_com.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_GLOBAL_COM)  += msm8974pro-ac-g3-global_com.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_CA)  += msm8974pro-g3-ca.dtb
        dtb-$(CONFIG_MACH_MSM8974_G3_CA)  += msm8974pro-ac-g3-ca.dtb
endif

# APQ8084
   zreladdr-$(CONFIG_ARCH_APQ8084)	:= 0x00008000

# MDM9630
   zreladdr-$(CONFIG_ARCH_MDM9630)	:= 0x00008000

# MDM9640
   zreladdr-$(CONFIG_ARCH_MDM9640)	:= 0x80008000

# MSMVPIPA
   zreladdr-$(CONFIG_ARCH_MSMVPIPA)	:= 0x80008000

# MSM8226
   zreladdr-$(CONFIG_ARCH_MSM8226)	:= 0x00008000

# FSM9900
   zreladdr-$(CONFIG_ARCH_FSM9900)	:= 0x0b608000

# FSM9010
   zreladdr-$(CONFIG_ARCH_FSM9010)	:= 0x0b608000

# MSM8610
   zreladdr-$(CONFIG_ARCH_MSM8610)	:= 0x00008000


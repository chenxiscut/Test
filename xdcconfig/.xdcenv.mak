#
_XDCBUILDCOUNT = 0
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = D:/dvsdk_1_11_00_00/codec_engine_1_20_02/packages;D:/dvsdk_1_11_00_00/codec_engine_1_20_02/examples;D:/dvsdk_1_11_00_00/framework_components_1_20_03/packages;D:/dvsdk_1_11_00_00/xdais_5_21/packages;D:/dvsdk_1_11_00_00/codecs_1_10/packages;D:/dvsdk_1_11_00_00/ndk_1_92_00_22_eval/packages;D:/dvsdk_1_11_00_00/biosutils_1_00_02/packages;D:/dvsdk_1_11_00_00/pspdrivers_1_10_00/packages;D:/dvsdk_1_11_00_00/edma3_lld_1_05_00/packages;D:/dvsdk_1_11_00_00/examples/common/evmDM6437;C:/CCStudio_v3.3/bios_5_31_08/packages;D:/dvsdk_1_11_00_00/xdc_2_95_02/packages;E:/project/dsp_pd
override XDCROOT = D:/dvsdk_1_11_00_00/xdc_2_95_02
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = D:/dvsdk_1_11_00_00/codec_engine_1_20_02/packages;D:/dvsdk_1_11_00_00/codec_engine_1_20_02/examples;D:/dvsdk_1_11_00_00/framework_components_1_20_03/packages;D:/dvsdk_1_11_00_00/xdais_5_21/packages;D:/dvsdk_1_11_00_00/codecs_1_10/packages;D:/dvsdk_1_11_00_00/ndk_1_92_00_22_eval/packages;D:/dvsdk_1_11_00_00/biosutils_1_00_02/packages;D:/dvsdk_1_11_00_00/pspdrivers_1_10_00/packages;D:/dvsdk_1_11_00_00/edma3_lld_1_05_00/packages;D:/dvsdk_1_11_00_00/examples/common/evmDM6437;C:/CCStudio_v3.3/bios_5_31_08/packages;D:/dvsdk_1_11_00_00/xdc_2_95_02/packages;E:/project/dsp_pd;D:/dvsdk_1_11_00_00/xdc_2_95_02/packages;..
HOSTOS = Windows
endif

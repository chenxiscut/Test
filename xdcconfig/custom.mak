## THIS IS A GENERATED FILE -- DO NOT EDIT
.configuro: linker.cmd

linker.cmd: \
  package/cfg/dsp_pd_x64P.o64P \
  package/cfg/dsp_pd_x64Pcfg.o64P \
  package/cfg/dsp_pd_x64Pcfg_c.o64P \
  package/cfg/dsp_pd_x64P.xdl
	$(SED) 's"^\"\(package/cfg/dsp_pd_x64Pcfg.cmd\)\"$""\"E:/project/dsp_pd/xdcconfig/\1\""' package/cfg/dsp_pd_x64P.xdl > $@

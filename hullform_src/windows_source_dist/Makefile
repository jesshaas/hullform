# Wedit Makefile for project hullform
SRCDIR=z:\home\peter\hullform\source
CFLAGS=-Ic:\lcc\include -DPROF -DHF9 -DPLATEDEV -DHULLSTAT -DEXT_OR_PROF 
CC=$(LCCROOT)\bin\lcc.exe
LINKER=$(LCCROOT)\bin\lcclnk.exe
OBJS=\
	dxf_mesh.obj \
	orc_offsets.obj \
	balanc.obj \
	builders.obj \
	calcar.obj \
	calcrm.obj \
	cleara.obj \
	conf_men.obj \
	curvar.obj \
	deve_men.obj \
	drag_men.obj \
	drasec.obj \
	drawline.obj \
	dxf_frames_output.obj \
	dxf_input.obj \
	dxf_outp.obj \
	dxf_overlay.obj \
	dxf_plates_output.obj \
	edit_men.obj \
	edit_sec.obj \
	file_men.obj \
	findrm.obj \
	gcc_subs.obj \
	gerrit.obj \
	gerrit96.obj \
	getdlg.obj \
	getnorm.obj \
	ghs.obj \
	glsurf.obj \
	graf_hfw.obj \
	graphdxf.obj \
	graphgl.obj \
	graphhew.obj \
	graphpos.obj \
	graphreg.obj \
	graphsub.obj \
	graphtek.obj \
	graphwmf.obj \
	hcio_hfw.obj \
	help_hfw.obj \
	hf8.res \
	hfw.obj \
	hidd_sub.obj \
	hidden.obj \
	hlsm_hf8.obj \
	holtro.obj \
	huldis.obj \
	hullar.obj \
	hullmome.obj \
	hullpa.obj \
	hullstat.obj \
	hulsur.obj \
	import.obj \
	makecat8.obj \
	memo_hfw.obj \
	mesh3d.obj \
	naca.obj \
	newhull8.obj \
	norm.obj \
	oortme.obj \
	overlay.obj \
	parse.obj \
	plot_var.obj \
	plotw.obj \
	reline.obj \
	resect.obj \
	revs_hfw.obj \
	rollout.obj \
	savits.obj \
	sca_view.obj \
	shellexp.obj \
	spline.obj \
	stat_men.obj \
	statedit.obj \
	strake.obj \
	stringer.obj \
	tabu_out.obj \
	tanks.obj \
	tbicons.obj \
	tool_hfw.obj \
	toolbar.obj \
	transom.obj \
	view_men.obj \
	view_su1.obj \
	view_sub.obj

LIBS=glu32.lib lz32.lib opengl32.lib htmlhelp.lib
EXE=hullform.exe

$(EXE):	$(OBJS) Makefile
	$(LINKER)  -s -subsystem windows -o $(SRCDIR)\hullform.exe $(OBJS) $(LIBS)

# Build dxf_mesh.c
DXF_MESH_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

dxf_mesh.obj: $(DXF_MESH_C) $(SRCDIR)\dxf_mesh.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\dxf_mesh.c

# Build orc_offsets.c
ORC_OFFSETS_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

orc_offsets.obj: $(ORC_OFFSETS_C) $(SRCDIR)\orc_offsets.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\orc_offsets.c

# Build balanc.c
BALANC_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

balanc.obj: $(BALANC_C) $(SRCDIR)\balanc.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\balanc.c

# Build builders.c
BUILDERS_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

builders.obj: $(BUILDERS_C) $(SRCDIR)\builders.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\builders.c

# Build calcar.c
CALCAR_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

calcar.obj: $(CALCAR_C) $(SRCDIR)\calcar.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\calcar.c

# Build calcrm.c
CALCRM_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

calcrm.obj: $(CALCRM_C) $(SRCDIR)\calcrm.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\calcrm.c

# Build cleara.c
CLEARA_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

cleara.obj: $(CLEARA_C) $(SRCDIR)\cleara.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\cleara.c

# Build conf_men.c
CONF_MEN_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

conf_men.obj: $(CONF_MEN_C) $(SRCDIR)\conf_men.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\conf_men.c

# Build curvar.c
CURVAR_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

curvar.obj: $(CURVAR_C) $(SRCDIR)\curvar.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\curvar.c

# Build deve_men.c
DEVE_MEN_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\
	$(SRCDIR)\plate.h\

deve_men.obj: $(DEVE_MEN_C) $(SRCDIR)\deve_men.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\deve_men.c

# Build drag_men.c
DRAG_MEN_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

drag_men.obj: $(DRAG_MEN_C) $(SRCDIR)\drag_men.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\drag_men.c

# Build drasec.c
DRASEC_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

drasec.obj: $(DRASEC_C) $(SRCDIR)\drasec.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\drasec.c

# Build drawline.c
DRAWLINE_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

drawline.obj: $(DRAWLINE_C) $(SRCDIR)\drawline.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\drawline.c

# Build dxf_frames_output.c
DXF_FRAMES_OUTPUT_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

dxf_frames_output.obj: $(DXF_FRAMES_OUTPUT_C) $(SRCDIR)\dxf_frames_output.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\dxf_frames_output.c

# Build dxf_input.c
DXF_INPUT_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

dxf_input.obj: $(DXF_INPUT_C) $(SRCDIR)\dxf_input.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\dxf_input.c

# Build dxf_outp.c
DXF_OUTP_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

dxf_outp.obj: $(DXF_OUTP_C) $(SRCDIR)\dxf_outp.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\dxf_outp.c

# Build dxf_overlay.c
DXF_OVERLAY_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

dxf_overlay.obj: $(DXF_OVERLAY_C) $(SRCDIR)\dxf_overlay.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\dxf_overlay.c

# Build dxf_plates_output.c
DXF_PLATES_OUTPUT_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

dxf_plates_output.obj: $(DXF_PLATES_OUTPUT_C) $(SRCDIR)\dxf_plates_output.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\dxf_plates_output.c

# Build edit_men.c
EDIT_MEN_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

edit_men.obj: $(EDIT_MEN_C) $(SRCDIR)\edit_men.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\edit_men.c

# Build edit_sec.c
EDIT_SEC_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\
	$(SRCDIR)\graf-hfw.h\

edit_sec.obj: $(EDIT_SEC_C) $(SRCDIR)\edit_sec.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\edit_sec.c

# Build file_men.c
FILE_MEN_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

file_men.obj: $(FILE_MEN_C) $(SRCDIR)\file_men.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\file_men.c

# Build findrm.c
FINDRM_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

findrm.obj: $(FINDRM_C) $(SRCDIR)\findrm.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\findrm.c

# Build gcc_subs.c
GCC_SUBS_C=\

gcc_subs.obj: $(GCC_SUBS_C) $(SRCDIR)\gcc_subs.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\gcc_subs.c

# Build gerrit.c
GERRIT_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

gerrit.obj: $(GERRIT_C) $(SRCDIR)\gerrit.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\gerrit.c

# Build gerrit96.c
GERRIT96_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

gerrit96.obj: $(GERRIT96_C) $(SRCDIR)\gerrit96.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\gerrit96.c

# Build getdlg.c
GETDLG_C=\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

getdlg.obj: $(GETDLG_C) $(SRCDIR)\getdlg.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\getdlg.c

# Build getnorm.c
GETNORM_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

getnorm.obj: $(GETNORM_C) $(SRCDIR)\getnorm.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\getnorm.c

# Build ghs.c
GHS_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

ghs.obj: $(GHS_C) $(SRCDIR)\ghs.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\ghs.c

# Build glsurf.c
GLSURF_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

glsurf.obj: $(GLSURF_C) $(SRCDIR)\glsurf.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\glsurf.c

# Build graf_hfw.c
GRAF_HFW_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\
	$(SRCDIR)\graf-hfw.h\

graf_hfw.obj: $(GRAF_HFW_C) $(SRCDIR)\graf_hfw.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\graf_hfw.c

# Build graphdxf.c
GRAPHDXF_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

graphdxf.obj: $(GRAPHDXF_C) $(SRCDIR)\graphdxf.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\graphdxf.c

# Build graphgl.c
GRAPHGL_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

graphgl.obj: $(GRAPHGL_C) $(SRCDIR)\graphgl.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\graphgl.c

# Build graphhew.c
GRAPHHEW_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

graphhew.obj: $(GRAPHHEW_C) $(SRCDIR)\graphhew.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\graphhew.c

# Build graphpos.c
GRAPHPOS_C=\

graphpos.obj: $(GRAPHPOS_C) $(SRCDIR)\graphpos.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\graphpos.c

# Build graphreg.c
GRAPHREG_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

graphreg.obj: $(GRAPHREG_C) $(SRCDIR)\graphreg.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\graphreg.c

# Build graphsub.c
GRAPHSUB_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

graphsub.obj: $(GRAPHSUB_C) $(SRCDIR)\graphsub.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\graphsub.c

# Build graphtek.c
GRAPHTEK_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

graphtek.obj: $(GRAPHTEK_C) $(SRCDIR)\graphtek.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\graphtek.c

# Build graphwmf.c
GRAPHWMF_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

graphwmf.obj: $(GRAPHWMF_C) $(SRCDIR)\graphwmf.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\graphwmf.c

# Build hcio_hfw.c
HCIO_HFW_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\
	$(SRCDIR)\graf-hfw.h\

hcio_hfw.obj: $(HCIO_HFW_C) $(SRCDIR)\hcio_hfw.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\hcio_hfw.c

# Build help_hfw.c
HELP_HFW_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\
	$(SRCDIR)\..\version.h\

help_hfw.obj: $(HELP_HFW_C) $(SRCDIR)\help_hfw.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\help_hfw.c

# Build hf8.res
HF8_RC=\

hf8.res:	$(HF8_RC) $(SRCDIR)\hf8.rc
	$(LCCROOT)\bin\lrc.exe -I$(SRCDIR) -Ic:\lcc\include -DPROF -DHF9 -DPLATEDEV -DHULLSTAT -DEXT_OR_PROF  $(SRCDIR)\hf8.rc

# Build hfw.c
HFW_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

hfw.obj: $(HFW_C) $(SRCDIR)\hfw.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\hfw.c

# Build hidd_sub.c
HIDD_SUB_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

hidd_sub.obj: $(HIDD_SUB_C) $(SRCDIR)\hidd_sub.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\hidd_sub.c

# Build hidden.c
HIDDEN_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\
	$(SRCDIR)\graf-hfw.h\

hidden.obj: $(HIDDEN_C) $(SRCDIR)\hidden.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\hidden.c

# Build hlsm_hf8.c
HLSM_HF8_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

hlsm_hf8.obj: $(HLSM_HF8_C) $(SRCDIR)\hlsm_hf8.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\hlsm_hf8.c

# Build holtro.c
HOLTRO_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

holtro.obj: $(HOLTRO_C) $(SRCDIR)\holtro.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\holtro.c

# Build huldis.c
HULDIS_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

huldis.obj: $(HULDIS_C) $(SRCDIR)\huldis.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\huldis.c

# Build hullar.c
HULLAR_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

hullar.obj: $(HULLAR_C) $(SRCDIR)\hullar.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\hullar.c

# Build hullmome.c
HULLMOME_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

hullmome.obj: $(HULLMOME_C) $(SRCDIR)\hullmome.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\hullmome.c

# Build hullpa.c
HULLPA_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

hullpa.obj: $(HULLPA_C) $(SRCDIR)\hullpa.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\hullpa.c

# Build hullstat.c
HULLSTAT_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\
	$(SRCDIR)\hst-help.h\

hullstat.obj: $(HULLSTAT_C) $(SRCDIR)\hullstat.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\hullstat.c

# Build hulsur.c
HULSUR_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

hulsur.obj: $(HULSUR_C) $(SRCDIR)\hulsur.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\hulsur.c

# Build import.c
IMPORT_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

import.obj: $(IMPORT_C) $(SRCDIR)\import.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\import.c

# Build makecat8.c
MAKECAT8_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

makecat8.obj: $(MAKECAT8_C) $(SRCDIR)\makecat8.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\makecat8.c

# Build memo_hfw.c
MEMO_HFW_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

memo_hfw.obj: $(MEMO_HFW_C) $(SRCDIR)\memo_hfw.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\memo_hfw.c

# Build mesh3d.c
MESH3D_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

mesh3d.obj: $(MESH3D_C) $(SRCDIR)\mesh3d.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\mesh3d.c

# Build naca.c
NACA_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

naca.obj: $(NACA_C) $(SRCDIR)\naca.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\naca.c

# Build newhull8.c
NEWHULL8_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

newhull8.obj: $(NEWHULL8_C) $(SRCDIR)\newhull8.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\newhull8.c

# Build norm.c
NORM_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\
	$(SRCDIR)\plate.h\

norm.obj: $(NORM_C) $(SRCDIR)\norm.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\norm.c

# Build oortme.c
OORTME_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

oortme.obj: $(OORTME_C) $(SRCDIR)\oortme.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\oortme.c

# Build overlay.c
OVERLAY_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

overlay.obj: $(OVERLAY_C) $(SRCDIR)\overlay.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\overlay.c

# Build parse.c
PARSE_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

parse.obj: $(PARSE_C) $(SRCDIR)\parse.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\parse.c

# Build plot_var.c
PLOT_VAR_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

plot_var.obj: $(PLOT_VAR_C) $(SRCDIR)\plot_var.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\plot_var.c

# Build plotw.c
PLOTW_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

plotw.obj: $(PLOTW_C) $(SRCDIR)\plotw.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\plotw.c

# Build reline.c
RELINE_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

reline.obj: $(RELINE_C) $(SRCDIR)\reline.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\reline.c

# Build resect.c
RESECT_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

resect.obj: $(RESECT_C) $(SRCDIR)\resect.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\resect.c

# Build revs_hfw.c
REVS_HFW_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

revs_hfw.obj: $(REVS_HFW_C) $(SRCDIR)\revs_hfw.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\revs_hfw.c

# Build rollout.c
ROLLOUT_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\
	$(SRCDIR)\plate.h\

rollout.obj: $(ROLLOUT_C) $(SRCDIR)\rollout.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\rollout.c

# Build savits.c
SAVITS_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

savits.obj: $(SAVITS_C) $(SRCDIR)\savits.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\savits.c

# Build sca_view.c
SCA_VIEW_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

sca_view.obj: $(SCA_VIEW_C) $(SRCDIR)\sca_view.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\sca_view.c

# Build shellexp.c
SHELLEXP_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

shellexp.obj: $(SHELLEXP_C) $(SRCDIR)\shellexp.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\shellexp.c

# Build spline.c
SPLINE_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

spline.obj: $(SPLINE_C) $(SRCDIR)\spline.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\spline.c

# Build stat_men.c
STAT_MEN_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

stat_men.obj: $(STAT_MEN_C) $(SRCDIR)\stat_men.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\stat_men.c

# Build statedit.c
STATEDIT_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\
	$(SRCDIR)\hst-help.h\

statedit.obj: $(STATEDIT_C) $(SRCDIR)\statedit.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\statedit.c

# Build strake.c
STRAKE_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

strake.obj: $(STRAKE_C) $(SRCDIR)\strake.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\strake.c

# Build stringer.c
STRINGER_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

stringer.obj: $(STRINGER_C) $(SRCDIR)\stringer.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\stringer.c

# Build tabu_out.c
TABU_OUT_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

tabu_out.obj: $(TABU_OUT_C) $(SRCDIR)\tabu_out.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\tabu_out.c

# Build tanks.c
TANKS_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

tanks.obj: $(TANKS_C) $(SRCDIR)\tanks.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\tanks.c

# Build tbicons.c
TBICONS_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

tbicons.obj: $(TBICONS_C) $(SRCDIR)\tbicons.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\tbicons.c

# Build tool_hfw.c
TOOL_HFW_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

tool_hfw.obj: $(TOOL_HFW_C) $(SRCDIR)\tool_hfw.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\tool_hfw.c

# Build toolbar.c
TOOLBAR_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

toolbar.obj: $(TOOLBAR_C) $(SRCDIR)\toolbar.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\toolbar.c

# Build transom.c
TRANSOM_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

transom.obj: $(TRANSOM_C) $(SRCDIR)\transom.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\transom.c

# Build view_men.c
VIEW_MEN_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

view_men.obj: $(VIEW_MEN_C) $(SRCDIR)\view_men.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\view_men.c

# Build view_su1.c
VIEW_SU1_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

view_su1.obj: $(VIEW_SU1_C) $(SRCDIR)\view_su1.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\view_su1.c

# Build view_sub.c
VIEW_SUB_C=\
	$(SRCDIR)\hulldesi.h\
	$(SRCDIR)\getdlg.h\
	$(SRCDIR)\win_getd.h\

view_sub.obj: $(VIEW_SUB_C) $(SRCDIR)\view_sub.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\view_sub.c

link:
	$(LINKER)  -s -subsystem windows -o $(SRCDIR)\hullform.exe $(OBJS) $(LIBS)

clean:
	del $(OBJS) hullform.exe

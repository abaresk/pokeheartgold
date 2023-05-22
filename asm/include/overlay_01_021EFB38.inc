#include <nitro/fs/overlay.h>
#pragma once
.public UnloadOverlayByID
.public HandleLoadOverlay
.public sub_02007200
.public sub_02007234
.public NARC_ctor
.public NARC_dtor
.public GfGfxLoader_LoadCharDataFromOpenNarc
.public GfGfxLoader_GXLoadPalFromOpenNarc
.public GfGfxLoader_GetScrnDataFromOpenNarc
.public GfGfxLoader_LoadFromOpenNarc
.public CreateSpriteResourcesHeader
.public G2dRenderer_Init
.public Create2DGfxResObjMan
.public Destroy2DGfxResObjMan
.public AddCharResObjFromOpenNarcWithAtEndFlag
.public AddPlttResObjFromOpenNarc
.public AddCellOrAnimResObjFromOpenNarc
.public sub_0200A740
.public DestroySingle2DGfxResObj
.public sub_0200ADA4
.public sub_0200AEB0
.public sub_0200B00C
.public sub_0200B0A8
.public CreateSysTask
.public sub_0200E33C
.public sub_0200E374
.public DestroySysTask
.public sub_02014AD8
.public sub_02014B08
.public sub_02014B9C
.public sub_02014BD8
.public sub_02014BF8
.public sub_02014C08
.public sub_02014C40
.public sub_02014DA0
.public sub_02014DB4
.public sub_02014EBC
.public sub_02014F84
.public sub_0201526C
.public sub_0201543C
.public sub_02015460
.public sub_02015484
.public sub_020154B0
.public sub_02015524
.public sub_02015528
.public AllocFromHeap
.public FreeToHeap
.public FreeToHeapExplicit
.public SetBothScreensModesAndDisable
.public InitBgFromTemplate
.public FreeBgTilemapBuffer
.public SetBgPriority
.public BG_ClearCharDataRange
.public LoadRectToBgTilemapRect
.public BgTilemapRectChangePalette
.public BgClearTilemapBufferAndCommit
.public GetBgTilemapBuffer
.public FillWindowPixelRect
.public ScheduleBgTilemapBufferTransfer
.public sub_0201F988
.public GX_SetBanks
.public GX_EngineAToggleLayers
.public Camera_SetClipBounds
.public sub_02024504
.public sub_02024714
.public GF_AssertFail
.public NNS_G3dGlbSetBaseTrans
.public NNS_G3dGlbSetBaseScale
.public MTX_Identity22_
.public MTX_Identity33_
.public FX_Div
.public GXx_SetMasterBrightness_
.public GX_ResetBankForBG
.public G2x_SetBGyAffine_
.public OS_SetTick
.public OS_GetTick
.public OS_SetTick
.public MIi_CpuClearFast
.public MI_Copy36B
.public memset
.public _ll_mul
.public _s32_div_f
.public _u32_div_f
.public NNS_G3dGlb
.public ov01_021E6048
.public ov01_021E6050
.public ov01_021E631C
.public ov01_021E6340
.public ov01_021EFB64
.public ov01_021EFC04
.public ov01_021EFC94
.public ov01_021EFCDC
.public ov01_021EFCF8
.public ov01_021EFE30
.public ov01_021EFE34
.public ov01_021EFE44
.public ov01_021EFE70
.public ov01_021EFE80
.public ov01_021EFEC8
.public ov01_021EFF28
.public ov01_021EFFEC
.public ov01_021F0028
.public ov01_021F0050
.public ov01_021F0250
.public ov01_021F029C
.public ov01_021F02C4
.public ov01_021F0454
.public ov01_021F0500
.public ov01_021F05C4
.public ov01_021F05F4
.public ov01_021F0614
.public ov01_021F06EC
.public ov01_021F0718
.public ov01_021F074C
.public ov01_021F0768
.public ov01_021F0780
.public ov01_021F0788
.public ov01_021F07E0
.public ov01_021F0960
.public ov01_021F09BC
.public ov01_021F09EC
.public ov01_021F0A0C
.public ov01_021F0A4C
.public ov01_021F0B44
.public ov01_021F0B5C
.public ov01_021F0B78
.public ov01_021F0D20
.public ov01_021F0DC8
.public ov01_021F0DDC
.public ov01_021F0E74
.public ov01_021F0E90
.public ov01_021F0EAC
.public ov01_021F0EC0
.public ov01_021F0F08
.public ov01_021F0FB8
.public ov01_021F1008
.public ov01_021F1044
.public ov01_021F1060
.public ov01_021F12B4
.public ov01_021F12D0
.public ov01_021F12E8
.public ov01_021F1310
.public ov01_021FB530
.public ov01_021FB554
.public ov114_0225F020
.public ov114_0225F280
.public ov115_0225F704
.public ov115_0225F728
.public ov115_0225F74C
.public ov115_0225F770
.public ov115_0225F794
.public ov115_0225F7B8
.public ov115_0225F7DC
.public ov115_0225F800
.public ov115_0225F824
.public ov115_0225F848
.public ov115_0225F86C
.public ov115_0225F890
.public ov115_0225F8B4
.public ov115_0225F8D8
.public ov115_0225F8FC
.public ov115_0225F920
.public ov115_0225F944
.public ov115_022602C0
.public ov115_022602E4
.public ov115_02260308
.public ov115_0226032C
.public ov115_02260350
.public ov116_0225F054
.public ov116_0225F364
.public ov116_0225F374
.public ov117_0225F020
.public ov117_0225F4D4
.public ov117_0225F4E4
.public ov117_0225F4F4
.public ov117_0225F504
.public ov117_0225F514
.public ov118_0225F020
.public ov119_0225F020
.public ov119_0225F37C
.public ov119_0225F670
.public ov119_0225FA2C
.public ov119_0225FF9C
.public ov119_02260258
.public ov120_0225F714
.public ov120_0225FA5C
.public ov120_0225FA68
.public ov120_0225FBAC
.public ov120_0225FBB8
.public ov120_0225FCFC
.public ov120_0225FD08
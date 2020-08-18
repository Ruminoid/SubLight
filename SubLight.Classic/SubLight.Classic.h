#pragma once

#ifndef R_SUBLIGHT_CLASSIC_H
#define R_SUBLIGHT_CLASSIC_H

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned short		u_int16;
typedef unsigned long		u_long;
typedef short int			int16;
#define PF_TABLE_BITS	12
#define PF_TABLE_SZ_16	4096

#define PF_DEEP_COLOR_AWARE 0	// make sure we get 16bpc pixels; 
								// AE_Effect.h checks for this.
 
#include "AEConfig.h"

#ifdef AE_OS_WIN
	typedef unsigned short PixelType;
	#include <Windows.h>
#endif

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"

#include "ass.h"

#include "SubLight.Classic.Strings.h"

// Utils

void BlendSingle(PF_EffectWorld* def, int dst_stride, int w, int h, int color, unsigned char* src, int src_stride, int dst_x, int dst_y, int src_w, int src_h);

/* Versioning information */

#define	MAJOR_VERSION	0
#define	MINOR_VERSION	2
#define	BUG_VERSION		5
#define	STAGE_VERSION	PF_Stage_BETA
#define	BUILD_VERSION	8

enum {
	SKELETON_INPUT = 0,
	R_SUBLIGHT_CLASSIC_PARAMS_OPEN,
	R_SUBLIGHT_CLASSIC_PARAMS_RENDER_GROUP_START,
	R_SUBLIGHT_CLASSIC_PARAMS_RENDER,
	R_SUBLIGHT_CLASSIC_PARAMS_RENDER_GROUP_END,
	R_SUBLIGHT_CLASSIC_PARAMS_TIME_GROUP_START,
	R_SUBLIGHT_CLASSIC_PARAMS_OFFSET,
	R_SUBLIGHT_CLASSIC_PARAMS_STRETCH,
	R_SUBLIGHT_CLASSIC_PARAMS_TIME_GROUP_END,
	R_SUBLIGHT_CLASSIC_NUM_PARAMS
};

enum {
	R_SUBLIGHT_CLASSIC_PARAMS_OPEN_DISK_ID = 1,
	R_SUBLIGHT_CLASSIC_PARAMS_RENDER_GROUP_START_DISK_ID,
	R_SUBLIGHT_CLASSIC_PARAMS_RENDER_DISK_ID,
	R_SUBLIGHT_CLASSIC_PARAMS_RENDER_GROUP_END_DISK_ID,
	R_SUBLIGHT_CLASSIC_PARAMS_TIME_GROUP_START_DISK_ID,
	R_SUBLIGHT_CLASSIC_PARAMS_OFFSET_DISK_ID,
	R_SUBLIGHT_CLASSIC_PARAMS_STRETCH_DISK_ID,
	R_SUBLIGHT_CLASSIC_PARAMS_TIME_GROUP_END_DISK_ID,
};

typedef ASS_Library AssLibrary, * AssLibraryP, ** AssLibraryH;

typedef struct GlobalData
{
	AssLibraryP assLibraryP;
} GlobalData, * GlobalDataP, ** GlobalDataH;

typedef struct SequenceData
{
	ASS_Track*		trackP			=	nullptr;
	ASS_Renderer*	rendererP		=	nullptr;
	char*			dataStringP		=	nullptr;
	size_t			len				=	0;
} SequenceData, * SequenceDataP, ** SequenceDataH;

#ifdef __cplusplus
	extern "C" {
#endif
	
DllExport	PF_Err 
EntryPointFunc(	
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra) ;

#ifdef __cplusplus
}
#endif

#endif

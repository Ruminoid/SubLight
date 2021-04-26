#include "SubLight.Classic.h"

typedef struct
{
	A_u_long index;
	A_char str[256];
} TableString;


TableString g_strs[StrID_NUMTYPES] = {
	StrID_NONE, "",
	StrID_Name, "R SubLight Classic",
	StrID_Description, "Load and render subtitles in Adobe softwares.",
	StrID_Params_Subtitle_Name, "Subtitle",
	StrID_Params_Open_Button_Name, "Open",
	StrID_Params_Render_Name, "Render",
	StrID_Params_Time_Name, "Time",
	StrID_Params_Offset_Name, "Offset",
	StrID_Params_Stretch_Name, "Stretch",
	StrID_Params_Blend_Ratio_Name, "Blend Ratio (With Current Layer)"
};


char* GetStringPtr(int strNum)
{
	return g_strs[strNum].str;
}

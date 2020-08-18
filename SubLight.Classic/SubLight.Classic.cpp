#include "SubLight.Classic.h"

#include <atlstr.h>
#include <ShObjIdl.h>
#include <cstdio>
#include <wchar.h>
#include <Windows.h>
#include <string>

#pragma region Add Windows Controls

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma endregion

#pragma region Const Data

const char* DEFAULT_DATA_STRING = "[Script Info]";

#pragma endregion

#pragma region About & GlobalSetup

static PF_Err About(
	PF_InData* in_data,
	PF_OutData* out_data)
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);

	suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg,
	                                      "%s v%d.%d\r%s",
	                                      STR(StrID_Name),
	                                      MAJOR_VERSION,
	                                      MINOR_VERSION,
	                                      STR(StrID_Description));
	return PF_Err_NONE;
}

static PF_Err GlobalSetup(
	PF_InData* in_data,
	PF_OutData* out_data)
{
	out_data->my_version = PF_VERSION(MAJOR_VERSION,
	                                  MINOR_VERSION,
	                                  BUG_VERSION,
	                                  STAGE_VERSION,
	                                  BUILD_VERSION);

	out_data->out_flags = PF_OutFlag_NON_PARAM_VARY |
		PF_OutFlag_PIX_INDEPENDENT |
		PF_OutFlag_SEQUENCE_DATA_NEEDS_FLATTENING;

	if (out_data->global_data)
		PF_DISPOSE_HANDLE(out_data->global_data);

	out_data->global_data = PF_NEW_HANDLE(sizeof(GlobalData));

	GlobalDataP global_data = static_cast<GlobalDataP>(PF_LOCK_HANDLE(out_data->global_data));
	global_data->assLibraryP = ass_library_init();
	PF_UNLOCK_HANDLE(out_data->global_data);

	return PF_Err_NONE;
}

static PF_Err GlobalSetDown(
	PF_InData* in_data)
{
	GlobalDataP global_data = static_cast<GlobalDataP>(PF_LOCK_HANDLE(in_data->global_data));
	if (!global_data) return PF_Err_NONE;

	if (global_data->assLibraryP)
		ass_library_done(global_data->assLibraryP);

	PF_UNLOCK_HANDLE(in_data->global_data);
	PF_DISPOSE_HANDLE(in_data->global_data);

	return PF_Err_NONE;
}

#pragma endregion

#pragma region Params Setup

static PF_Err ParamsSetup(
	PF_InData* in_data,
	PF_OutData* out_data)
{
	PF_Err err = PF_Err_NONE;
	PF_ParamDef def;

	AEFX_CLR_STRUCT(def);

	PF_ADD_BUTTON(STR(StrID_Params_Subtitle_Name),
	              STR(StrID_Params_Open_Button_Name),
	              0,
	              PF_ParamFlag_SUPERVISE,
	              R_SUBLIGHT_CLASSIC_PARAMS_OPEN_DISK_ID);

	AEFX_CLR_STRUCT(def);

	def.flags = PF_ParamFlag_START_COLLAPSED;

	PF_ADD_TOPIC(STR(StrID_Params_Render_Name),
	             R_SUBLIGHT_CLASSIC_PARAMS_RENDER_GROUP_START_DISK_ID);

	AEFX_CLR_STRUCT(def);

	PF_ADD_CHECKBOX(STR(StrID_Params_Render_Name),
	                "",
	                TRUE,
	                0,
	                R_SUBLIGHT_CLASSIC_PARAMS_RENDER_DISK_ID);

	AEFX_CLR_STRUCT(def);

	PF_END_TOPIC(R_SUBLIGHT_CLASSIC_PARAMS_RENDER_GROUP_END_DISK_ID);

	AEFX_CLR_STRUCT(def);

	def.flags = PF_ParamFlag_START_COLLAPSED;

	PF_ADD_TOPIC(STR(StrID_Params_Time_Name),
	             R_SUBLIGHT_CLASSIC_PARAMS_TIME_GROUP_START_DISK_ID);

	AEFX_CLR_STRUCT(def);

	PF_ADD_FLOAT_SLIDER(STR(StrID_Params_Offset_Name),
	                    -100,
	                    100,
	                    -100,
	                    100,
	                    AEFX_AUDIO_DEFAULT_CURVE_TOLERANCE,
	                    0,
	                    1,
	                    0,
	                    1,
	                    R_SUBLIGHT_CLASSIC_PARAMS_OFFSET_DISK_ID);

	AEFX_CLR_STRUCT(def);

	PF_ADD_FLOAT_SLIDER(STR(StrID_Params_Stretch_Name),
	                    -50,
	                    50,
	                    -50,
	                    50,
	                    AEFX_AUDIO_DEFAULT_CURVE_TOLERANCE,
	                    1,
	                    1,
	                    0,
	                    1,
	                    R_SUBLIGHT_CLASSIC_PARAMS_STRETCH_DISK_ID);

	AEFX_CLR_STRUCT(def);

	PF_END_TOPIC(R_SUBLIGHT_CLASSIC_PARAMS_TIME_GROUP_END_DISK_ID);

	out_data->num_params = R_SUBLIGHT_CLASSIC_NUM_PARAMS;

	return err;
}

#pragma endregion

#pragma region Sequence Setup

static PF_Err InitializeSequenceData(
	SequenceDataP sequence_data,
	AssLibraryP ass_library,
	char* data_string,
	size_t len,
	int width,
	int height
)
{
	ass_set_frame_size(sequence_data->rendererP, width, height);
	ass_set_font_scale(sequence_data->rendererP, 1.);
	ass_set_fonts(sequence_data->rendererP, nullptr, "Sans", 1, nullptr, true);
	ass_set_cache_limits(sequence_data->rendererP, 512, 32);
	if (data_string) sequence_data->trackP = ass_read_memory(ass_library, data_string, len, nullptr);

	sequence_data->dataStringP = data_string;
	sequence_data->len = len;

	return PF_Err_NONE;
}

static PF_Err SequenceSetup(
	PF_InData* in_data, // String Data
	PF_OutData* out_data) // Sequence Data
{
	GlobalDataP global_data = static_cast<GlobalDataP>(PF_LOCK_HANDLE(in_data->global_data));
	if (!global_data || !global_data->assLibraryP) return PF_Err_NONE;

	// Initialize Sequence Data

	out_data->sequence_data = PF_NEW_HANDLE(sizeof(SequenceData));
	SequenceDataP sequence_data = static_cast<SequenceDataP>(PF_LOCK_HANDLE(out_data->sequence_data));
	if (!sequence_data) return PF_Err_NONE;

	sequence_data->rendererP = ass_renderer_init(global_data->assLibraryP);
	if (!sequence_data->rendererP) return PF_Err_NONE;

	size_t len = strlen(DEFAULT_DATA_STRING);
	char* data_string = new char[len];
	memcpy(data_string, DEFAULT_DATA_STRING, len);

	InitializeSequenceData(sequence_data, global_data->assLibraryP, data_string,
		len, in_data->width, in_data->height);

	PF_UNLOCK_HANDLE(out_data->sequence_data);

	//in_data->sequence_data = out_data->sequence_data;

	PF_UNLOCK_HANDLE(in_data->global_data);

	return PF_Err_NONE;
}

static PF_Err SequenceSetDown(
	PF_InData* in_data) // Sequence Data
{
	// Dispose Sequence Data

	if (!in_data->sequence_data) return PF_Err_NONE;

	SequenceDataP sequence_data = static_cast<SequenceDataP>(PF_LOCK_HANDLE(in_data->sequence_data));

	if (sequence_data)
	{
		if (sequence_data->rendererP)
			ass_renderer_done(sequence_data->rendererP);
		if (sequence_data->trackP)
			ass_free_track(sequence_data->trackP);
		if (sequence_data->dataStringP)
			delete[] sequence_data->dataStringP;
	}

	PF_UNLOCK_HANDLE(in_data->sequence_data);
	PF_DISPOSE_HANDLE(in_data->sequence_data);

	return PF_Err_NONE;
}

static PF_Err SequenceReSetup(
	PF_InData* in_data, // String Data
	PF_OutData* out_data) // Sequence Data
{
	GlobalDataP global_data = static_cast<GlobalDataP>(PF_LOCK_HANDLE(in_data->global_data));
	if (!global_data || !global_data->assLibraryP) return PF_Err_NONE;

	if (in_data->sequence_data && PF_GET_HANDLE_SIZE(in_data->sequence_data) == sizeof(SequenceData))
	{
		// Use Existing Sequence Data
		// 
		// ...and currently there's nothing to do.

		//out_data->sequence_data = in_data->sequence_data;
	}
	else if (PF_GET_HANDLE_SIZE(in_data->sequence_data))
	{
		// Re-setup - Reload string data

		char* source = static_cast<char*>(PF_LOCK_HANDLE(in_data->sequence_data));
		size_t len = PF_GET_HANDLE_SIZE(in_data->sequence_data);
		char* data_string = new char[len];
		memcpy(data_string, source, len);
		PF_UNLOCK_HANDLE(in_data->sequence_data);
		//PF_DISPOSE_HANDLE(in_data->sequence_data);

		if (!data_string) return PF_Err_NONE;

		// Unflat - Initialize Sequence Data

		if (out_data->sequence_data)
			PF_DISPOSE_HANDLE(out_data->sequence_data);

		out_data->sequence_data = PF_NEW_HANDLE(sizeof(SequenceData));
		SequenceDataP sequence_data = static_cast<SequenceDataP>(PF_LOCK_HANDLE(out_data->sequence_data));
		if (!sequence_data) return PF_Err_NONE;

		sequence_data->rendererP = ass_renderer_init(global_data->assLibraryP);
		if (!sequence_data->rendererP) return PF_Err_NONE;

		InitializeSequenceData(sequence_data, global_data->assLibraryP, data_string, len, in_data->width,
			in_data->height);

		PF_UNLOCK_HANDLE(out_data->sequence_data);

		//in_data->sequence_data = out_data->sequence_data;
	}
	
	PF_UNLOCK_HANDLE(in_data->global_data);
}

static PF_Err SequenceFlatten(
	PF_InData* in_data, // Sequence Data
	PF_OutData* out_data) // String Data
{
	if (!in_data->sequence_data) return PF_Err_NONE;
	SequenceDataP sequence_data = static_cast<SequenceDataP>(PF_LOCK_HANDLE(in_data->sequence_data));
	if (!sequence_data || !sequence_data->dataStringP) return PF_Err_NONE;

	// Flatten Data

	size_t len = sequence_data->len;
	char* tmp = new char[len];

	memcpy(tmp, sequence_data->dataStringP, len);

	if (sequence_data->rendererP)
		ass_renderer_done(sequence_data->rendererP);
	if (sequence_data->trackP)
		ass_free_track(sequence_data->trackP);
	if (sequence_data->dataStringP)
		delete[] sequence_data->dataStringP;

	PF_UNLOCK_HANDLE(in_data->sequence_data);
	PF_DISPOSE_HANDLE(in_data->sequence_data);

	out_data->sequence_data = PF_NEW_HANDLE(len);

	char* target = static_cast<char*>(PF_LOCK_HANDLE(out_data->sequence_data));
	memcpy(target, tmp, len);
	PF_UNLOCK_HANDLE(out_data->sequence_data);

	return PF_Err_NONE;
}

#pragma endregion

#pragma region File Dialog

const COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
	{L"ASS Subtitle (*.ass)", L"*.ass"}
};

static wchar_t* BasicFileOpen()
{
	// CoCreate the File Open Dialog object.
	IFileDialog* pfd = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (FAILED(hr)) return nullptr;
	// Set the options on the dialog.
	//DWORD dwFlags;
	// Before setting, always get the options first in order not to override existing options.
	//hr = pfd->GetOptions(&dwFlags);
	//if (FAILED(hr)) return nullptr;
	// In this case, get shell items only for file system items.
	//hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
	if (FAILED(hr)) return nullptr;
	// Set the file types to display only. Notice that, this is a 1-based array.
	hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
	if (FAILED(hr)) return nullptr;
	// Set the selected file type index to Word Docs for this example.
	hr = pfd->SetFileTypeIndex(1); // 1-based Array
	if (FAILED(hr)) return nullptr;
	// Set the default extension to be ".ass" file.
	hr = pfd->SetDefaultExtension(L"ass");
	if (FAILED(hr)) return nullptr;
	// Show the dialog
	hr = pfd->Show(nullptr);
	if (FAILED(hr)) return nullptr;
	// Obtain the result, once the user clicks the 'Open' button.
	// The result is an IShellItem object.
	IShellItem* psiResult;
	hr = pfd->GetResult(&psiResult);
	if (FAILED(hr)) return nullptr;
	// We are just going to print out the name of the file for sample sake.
	wchar_t* pszFilePath = nullptr;
	hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
	if (FAILED(hr)) return nullptr;
	USES_CONVERSION;
	//int len = WideCharToMultiByte(CP_OEMCP,
	//                              0,
	//                              pszFilePath,
	//                              wcslen(pszFilePath),
	//                              nullptr,
	//                              0,
	//                              NULL,
	//                              NULL);
	//char* result = new char[len + 1];
	//WideCharToMultiByte(CP_OEMCP,
	//                    0,
	//                    pszFilePath,
	//                    wcslen(pszFilePath),
	//                    result,
	//                    len,
	//                    nullptr,
	//                    NULL);

	//char* result = W2A(pszFilePath);
	//delete[] pszFilePath;

	psiResult->Release();
	pfd->Release();
	return pszFilePath;
}

#pragma endregion

#pragma region Button Click Event

static PF_Err
UserChangedParam(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	const PF_UserChangedParamExtra* which_hitP)
{
	PF_Err err = PF_Err_NONE;

	if (which_hitP->param_index == R_SUBLIGHT_CLASSIC_PARAMS_OPEN)
	{
		// Browse File

		wchar_t* file_path = BasicFileOpen();
		if (!file_path) return PF_Err_NONE;

		// Read File

		FILE* fp;
		errno_t fille_open_result = _wfopen_s(&fp, file_path, L"r");
		if (fille_open_result) return PF_Err_NONE;
		//delete[] file_path;

		if (fp == nullptr) return PF_Err_NONE;
		fseek(fp, 0, SEEK_END);

		const int len = ftell(fp);
		char* data_string = new char[len];

		fseek(fp, 0, SEEK_SET);
		fread(data_string, len, 1, fp);
		fclose(fp);

		// Check ASS File

		if (*data_string != '[')
		{
			delete[] data_string;
			return PF_Err_NONE;
		}

		// Replace ASS Track

		if (!in_data->global_data) return PF_Err_NONE;
		GlobalDataP global_data = static_cast<GlobalDataP>(PF_LOCK_HANDLE(in_data->global_data));
		if (!global_data || !global_data->assLibraryP) return PF_Err_NONE;

		if (!out_data->sequence_data) return PF_Err_NONE;
		SequenceDataP sequence_data = static_cast<SequenceDataP>(PF_LOCK_HANDLE(out_data->sequence_data));
		if (!sequence_data) return PF_Err_NONE;

		if (sequence_data->trackP)
			ass_free_track(sequence_data->trackP);

		sequence_data->trackP = ass_read_memory(global_data->assLibraryP, data_string, len, nullptr);
		sequence_data->dataStringP = data_string;
		sequence_data->len = len;

		PF_UNLOCK_HANDLE(out_data->sequence_data);
		PF_UNLOCK_HANDLE(in_data->global_data);
	}

	return err;
}

#pragma endregion

#pragma region Render Core

static PF_Err Render(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	if (!out_data->sequence_data) return PF_Err_NONE;
	SequenceDataP sequence_data = static_cast<SequenceDataP>(PF_LOCK_HANDLE(out_data->sequence_data));
	if (!sequence_data || !sequence_data->dataStringP || !sequence_data->rendererP || !sequence_data->trackP)
		return PF_Err_NONE;

	// Initialize Data

	const int width = in_data->width;
	const int height = in_data->height;
	//const int stride = (width * 32 + 31 & ~31) / 8;
	const int stride = output->rowbytes; // Use stride from output here

	// Calculate Time

	const int time = (in_data->current_time / in_data->time_scale + params[R_SUBLIGHT_CLASSIC_PARAMS_OFFSET]
	                                                                ->u.fs_d.value)
		* params[R_SUBLIGHT_CLASSIC_PARAMS_STRETCH]->u.fs_d.value / 100;

	// Render Image

	ass_set_frame_size(sequence_data->rendererP, width, height);
	ASS_Image* image = ass_render_frame(sequence_data->rendererP, sequence_data->trackP, time, nullptr);

	// Blend Image

	while (image)
	{
		BlendSingle(
			output, stride, width, height,
			image->color,
			image->bitmap, image->stride, image->dst_x, image->dst_y, image->w, image->h);

		image = image->next;
	}

	PF_UNLOCK_HANDLE(out_data->sequence_data);

	return PF_Err_NONE;
}

#pragma endregion

#pragma region Entry Point Function

DllExport PF_Err EntryPointFunc(
	PF_Cmd cmd,
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	void* extra)
{
	PF_Err err = PF_Err_NONE;

	try
	{
		switch (cmd)
		{
		case PF_Cmd_ABOUT:

			err = About(in_data,
			            out_data);
			break;

		case PF_Cmd_GLOBAL_SETUP:

			err = GlobalSetup(in_data,
			                  out_data);
			break;

		case PF_Cmd_GLOBAL_SETDOWN:

			err = GlobalSetDown(in_data);
			break;

		case PF_Cmd_SEQUENCE_SETUP:

			err = SequenceSetup(in_data, out_data);
			break;

		case PF_Cmd_SEQUENCE_SETDOWN:

			err = SequenceSetDown(in_data);
			break;

		case PF_Cmd_SEQUENCE_RESETUP:

			err = SequenceReSetup(in_data, out_data);
			break;

		case PF_Cmd_SEQUENCE_FLATTEN:

			err = SequenceFlatten(in_data, out_data);
			break;

		case PF_Cmd_USER_CHANGED_PARAM:

			err = UserChangedParam(in_data,
			                       out_data,
			                       params,
			                       static_cast<const PF_UserChangedParamExtra*>(extra));
			break;

		case PF_Cmd_PARAMS_SETUP:

			err = ParamsSetup(in_data,
			                  out_data);
			break;

		case PF_Cmd_RENDER:

			err = Render(in_data,
			             out_data,
			             params,
			             output);
			break;
		}
	}
	catch (PF_Err& thrown_err)
	{
		err = thrown_err;
	}
	return err;
}

#pragma endregion

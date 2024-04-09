//////////////////////////////////////////////////////////////////////
/// \file  dg_model_parameters_client.inc
/// \brief List of Json model parameters, visible to client side.
///
/// Copyright 2021 DeGirum Corporation
///
/// This file contains declaration of Json model parameters, visible to client side.
/// This file is included into dg_model_parameters.h for client-side builds.
/// 


///
/// List of all configuration parameters
/// Format:
///	_( name,						section,				C-type,						default,		req'd,	runtime,visible, fallback )
///
#define DG_MODEL_PARAMS_LIST_CLIENT \
	_( ConfigVersion,				SECT_TOP,				int,						0,				true,	false,	false,	None	) \
	_( MeasureTime,					SECT_TOP,				bool,						false,			false,	true,	true,	None	) \
	_( CloudToken,					SECT_TOP,				std::string,				"",				false,	true,	false,	None	) \
	\
	_( DeviceType,					SECT_DEVICE,			std::string,				"CPU",			false,	true,	true,	None	) \
	_( RuntimeAgent,				SECT_DEVICE,			std::string,				"N2X",			false,	true,	true,	None	) \
	_( EagerBatchSize,				SECT_DEVICE,			size_t,						8,				false,	true,	true,	None	) \
	\
	_( ModelInputN,					SECT_MODEL_PARAMETERS,	int,						1,				false,	false,	false,	None	) \
	_( ModelInputH,					SECT_MODEL_PARAMETERS,	int,						1,				false,	false,	false,	None	) \
	_( ModelInputW,					SECT_MODEL_PARAMETERS,	int,						1,				false,	false,	false,	None	) \
	_( ModelInputC,					SECT_MODEL_PARAMETERS,	int,						1,				false,	false,	false,	None	) \
	_( ModelQuantEn,				SECT_MODEL_PARAMETERS,	bool,						false,			false,	false,	false,	None	) \
	_( ModelFmt,					SECT_MODEL_PARAMETERS,	std::string,				"auto",			false,	false,	false,	None	) \
	\
	_( InputN,						SECT_PRE_PROCESS,		int,						1,				false,	false,	true,	ModelInputN ) \
	_( InputH,						SECT_PRE_PROCESS,		int,						1,				false,	false,	true,	ModelInputH ) \
	_( InputW,						SECT_PRE_PROCESS,		int,						1,				false,	false,	true,	ModelInputW ) \
	_( InputC,						SECT_PRE_PROCESS,		int,						1,				false,	false,	true,	ModelInputC ) \
	_( InputShape,					SECT_PRE_PROCESS,		std::vector< int >,			{},				false,	false,	true,	None	) \
	_( InputQuantEn,				SECT_PRE_PROCESS,		bool,						false,			false,	false,	true,	ModelQuantEn ) \
	_( InputType,					SECT_PRE_PROCESS,		std::string,				"Image",		false,	false,	true,	None	) \
	_( InputFmt,					SECT_PRE_PROCESS,		std::string,				"auto",			false,	false,	false,	ModelFmt	) \
	_( InputTensorLayout,			SECT_PRE_PROCESS,		std::string,				"auto",			false,	false,	true,	InputFmt	) \
	_( InputImgFmt,					SECT_PRE_PROCESS,		std::string,				"JPEG",			false,	true,	false,	None	) \
	_( InputImgRawDataType,			SECT_PRE_PROCESS,		std::string,				"DG_UINT8",		false,	true,	false,	None	) \
	_( InputRawDataType,			SECT_PRE_PROCESS,		std::string,				"DG_UINT8",		false,	true,	true,	InputImgRawDataType	) \
	_( InputImgRotation,			SECT_PRE_PROCESS,		int,						0,				false,	true,	false,	None	) \
	_( InputColorSpace,				SECT_PRE_PROCESS,		std::string,				"RGB",			false,	true,	true,	None	) \
	_( InputResizeMethod,			SECT_PRE_PROCESS,		std::string,				"bilinear",		false,	false,	true,	None	) \
	_( InputPadMethod,				SECT_PRE_PROCESS,		std::string,				"letterbox",	false,	false,	true,	None	) \
	_( InputCropPercentage,			SECT_PRE_PROCESS,		double,						1.0,			false,	false,	true,	None	) \
	_( ImageBackend,				SECT_PRE_PROCESS,		std::string,				"auto",			false,	false,	true,	None	) \
	\
	_( OutputPostprocessType,		SECT_POST_PROCESS,		std::string,				"None",			false,	true,	true,	None	) \
	_( OutputConfThreshold,			SECT_POST_PROCESS,		double,						0.1,			false,	true,	true,	None	) \
	_( OutputNMSThreshold,			SECT_POST_PROCESS,		double,						0.6,			false,	true,	true,	None	) \
	_( OutputTopK,					SECT_POST_PROCESS,		size_t,						0,				false,	true,	true,	None	) \
	_( MaxDetections,				SECT_POST_PROCESS,		int,						20,				false,	true,	true,	None	) \
	_( MaxDetectionsPerClass,		SECT_POST_PROCESS,		int,						100,			false,	true,	true,	None	) \
	_( MaxClassesPerDetection,		SECT_POST_PROCESS,		int,						30,				false,	true,	true,	None	) \
	_( UseRegularNMS,				SECT_POST_PROCESS,		bool,						true,			false,	true,	true,	None	)

//////////////////////////////////////////////////////////////////////
/// dg_model_parameters.inc
/// \brief List of all JSON model parameters
///
/// Copyright 2025 DeGirum Corporation
///
/// This file contains declaration of all JSON model parameters.
/// This file is included into dg_model_parameters.h for Framework builds
///

///
/// List of all configuration parameters
/// Format:
///	_( name,					section,				C-type,				default,					req'd,	runtime,visible,fallback )
///
#define DG_MODEL_PARAMS_LIST \
	_( ConfigVersion,			SECT_TOP,				int,				0,							true,	false,	false,	None				) \
	_( Checksum,				SECT_TOP,				std::string,		"",							true,	false,	false,	None				) \
	_( CloudURL,				SECT_TOP,				std::string,		"",							false,	true,	false,	None				) \
	_( CloudModelName,			SECT_TOP,				std::string,		"",							true,	true,	false,	None				) \
	_( CloudToken,				SECT_TOP,				std::string,		"",							false,	true,	false,	None				) \
	_( MeasureTime,				SECT_TOP,				bool,				false,						false,	true,	true,	None				) \
	\
	_( DeviceType,				SECT_DEVICE,			std::string,		"CPU",						false,	true,	true,	None				) \
	_( RuntimeAgent,			SECT_DEVICE,			std::string,		"N2X",						false,	true,	true,	None				) \
	_( SupportedDeviceTypes,	SECT_DEVICE,			std::string,		"",							false,	false,	true,	None				) \
	_( DeviceMask,				SECT_DEVICE,			uint64_t,			-1,							false,	true,	false,	None				) \
	_( DeviceTimeout_ms,		SECT_DEVICE,			double,				-1,							false,	true,	false,	None				) \
	_( EagerBatchSize,			SECT_DEVICE,			size_t,				8,							false,	true,	true,	None				) \
	_( ThreadPackSize,			SECT_DEVICE,			size_t,				2,							false,	true,	false,	None				) \
	_( SimulateAbort,			SECT_DEVICE,			bool,				false,						false,	true,	false,	None				) \
	_( SimulateFail,			SECT_DEVICE,			int,				0,							false,	true,	false,	None				) \
	_( SimulateParams,			SECT_DEVICE,			bool,				false,						false,	true,	false,	None				) \
	_( SimulateTime_ms,			SECT_DEVICE,			double,				0,							false,	true,	false,	None				) \
	_( SimulatePreprocessor,	SECT_DEVICE,			bool,				false,						false,	true,	false,	None				) \
	\
	_( InputN,					SECT_PRE_PROCESS,		int,				0,							false,	true,	true,	ModelInputN			) \
	_( InputH,					SECT_PRE_PROCESS,		int,				0,							false,	true,	true,	ModelInputH			) \
	_( InputW,					SECT_PRE_PROCESS,		int,				0,							false,	true,	true,	ModelInputW			) \
	_( InputC,					SECT_PRE_PROCESS,		int,				0,							false,	true,	true,	ModelInputC			) \
	_( InputShape,				SECT_PRE_PROCESS,		DG::vec_sz,			DG::vec_sz(),				false,	true,	true,	None				) \
	_( InputQuantEn,			SECT_PRE_PROCESS,		bool,				false,						false,	false,	true,	ModelQuantEn		) \
	_( InputType,				SECT_PRE_PROCESS,		std::string,		"Image",					false,	false,	true,	None				) \
	_( InputFmt,				SECT_PRE_PROCESS,		std::string,		"auto",						false,	false,	false,	ModelFmt			) \
	_( InputTensorLayout,		SECT_PRE_PROCESS,		std::string,		"auto",						false,	false,	true,	InputFmt			) \
	_( InputImgFmt,				SECT_PRE_PROCESS,		std::string,		"JPEG",						false,	true,	false,	None				) \
	_( InputImgRawDataType,		SECT_PRE_PROCESS,		std::string,		"DG_UINT8",					false,	true,	false,	None				) \
	_( InputRawDataType,		SECT_PRE_PROCESS,		std::string,		"DG_UINT8",					false,	true,	true,	InputImgRawDataType	) \
	_( InputImgRotation,		SECT_PRE_PROCESS,		int,				0,							false,	true,	false,	None				) \
	_( InputColorSpace,			SECT_PRE_PROCESS,		std::string,		"RGB",						false,	true,	true,	None				) \
	_( InputResizeMethod,		SECT_PRE_PROCESS,		std::string,		"bilinear",					false,	false,	true,	None				) \
	_( InputPadMethod,			SECT_PRE_PROCESS,		std::string,		"letterbox",				false,	false,	true,	None				) \
	_( InputCropPercentage,		SECT_PRE_PROCESS,		double,				1.0,						false,	false,	true,	None				) \
	_( ImageBackend,			SECT_PRE_PROCESS,		std::string,		"auto",						false,	false,	true,	None				) \
	_( InputImgNormEn,			SECT_PRE_PROCESS,		bool,				false,						false,	false,	true,	None				) \
	_( InputImgNormCoeff,		SECT_PRE_PROCESS,		double,				1./255.,					false,	false,	true,	None				) \
	_( InputImgMean,			SECT_PRE_PROCESS,		DG::vec_flt,		DG::vec_flt(),				false,	false,	true,	None				) \
	_( InputImgStd,				SECT_PRE_PROCESS,		DG::vec_flt,		DG::vec_flt(),				false,	false,	true,	None				) \
	_( InputImgQuantOffset,		SECT_PRE_PROCESS,		float,				0,							false,	false,	false,	None				) \
	_( InputImgQuantScale,		SECT_PRE_PROCESS,		float,				1,							false,	false,	false,	None				) \
	_( InputImgSliceType,		SECT_PRE_PROCESS,		std::string,		"None",						false,	false,	false,	None				) \
	_( InputQuantOffset,		SECT_PRE_PROCESS,		float,				0,							false,	false,	true,	InputImgQuantOffset	) \
	_( InputQuantScale,			SECT_PRE_PROCESS,		float,				1,							false,	false,	true,	InputImgQuantScale	) \
	_( InputFrameSize,			SECT_PRE_PROCESS,		int,				400,						false,	false,	false,	None				) \
	_( InputFrameHopStepSize,	SECT_PRE_PROCESS,		int,				160,						false,	false,	false,	None				) \
	_( InputSamplingRate,		SECT_PRE_PROCESS,		double,				16000,						false,	false,	true,	None				) \
	_( InputWaveformSize,		SECT_PRE_PROCESS,		int,				15600,						false,	false,	true,	None				) \
	_( InputMelFrequencyRange,	SECT_PRE_PROCESS,		DG::vec_dbl,		DG::vec_dbl(),				false,	false,	false,	None				) \
	_( InputNormEn,				SECT_PRE_PROCESS,		bool,				false,						false,	false,	true,	InputImgNormEn		) \
	_( InputNormCoeff,			SECT_PRE_PROCESS,		double,				1./255.,					false,	false,	true,	InputImgNormCoeff	) \
	_( InputNormMean,			SECT_PRE_PROCESS,		DG::vec_flt,		DG::vec_flt(),				false,	false,	true,	InputImgMean		) \
	_( InputNormStd,			SECT_PRE_PROCESS,		DG::vec_flt,		DG::vec_flt(),				false,	false,	true,	InputImgStd			) \
	_( InputScaleEn,			SECT_PRE_PROCESS,		bool,				false,						false,	false,	true,	InputNormEn			) \
	_( InputScaleCoeff,			SECT_PRE_PROCESS,		double,				1./255.,					false,	false,	true,	InputNormCoeff		) \
	\
	_( ModelPath,				SECT_MODEL_PARAMETERS,	std::string,		"",							true,	false,	true,	None				) \
	_( ModelInputN,				SECT_MODEL_PARAMETERS,	int,				0,							false,	false,	false,	None				) \
	_( ModelInputH,				SECT_MODEL_PARAMETERS,	int,				0,							false,	false,	false,	None				) \
	_( ModelInputW,				SECT_MODEL_PARAMETERS,	int,				0,							false,	false,	false,	None				) \
	_( ModelInputC,				SECT_MODEL_PARAMETERS,	int,				0,							false,	false,	false,	None				) \
	_( ModelQuantEn,			SECT_MODEL_PARAMETERS,	bool,				false,						false,	false,	false,	None				) \
	_( ModelFmt,				SECT_MODEL_PARAMETERS,	std::string,		"auto",						false,	false,	false,	None				) \
	_( ModelSchedulerThreshold,	SECT_MODEL_PARAMETERS,	double,				0,							false,	false,	false,	None				) \
	_( CalibrationFilePath,		SECT_MODEL_PARAMETERS,	std::string,		"",							false,	false,	true,	None				) \
	_( UseFloat16,				SECT_MODEL_PARAMETERS,	bool,				false,						false,	false,	true,	None				) \
	_( CompilerOptions,			SECT_MODEL_PARAMETERS,	json,				json::object(),				false,	true,	false,	None				) \
	_( LLMMaxNewTokens,			SECT_MODEL_PARAMETERS,	int,				100,						false,	true,	false,	None				) \
	_( LLMTemperature,			SECT_MODEL_PARAMETERS,	float,				-1,							false,	true,	false,	None				) \
	_( LLMTopK,					SECT_MODEL_PARAMETERS,	int,				-1,							false,	true,	false,	None				) \
	_( LLMTopP,					SECT_MODEL_PARAMETERS,	float,				-1,							false,	true,	false,	None				) \
	\
	_( OutputPostprocessType,	SECT_POST_PROCESS,		std::string,		"None",						false,	true,	true,	None				) \
	_( InferenceResultsType,	SECT_POST_PROCESS,		std::string,		"",							false,	true,	true,	None				) \
	_( OutputConfThreshold,		SECT_POST_PROCESS,		double,				0.3,						false,	true,	true,	None				) \
	_( OutputNMSThreshold,		SECT_POST_PROCESS,		double,				0.6,						false,	true,	true,	None				) \
	_( OutputTopK,				SECT_POST_PROCESS,		size_t,				0,							false,	true,	true,	None				) \
	_( MaxDetections,			SECT_POST_PROCESS,		int,				100,						false,	true,	true,	None				) \
	_( MaxDetectionsPerClass,	SECT_POST_PROCESS,		int,				100,						false,	true,	true,	None				) \
	_( MaxClassesPerDetection,	SECT_POST_PROCESS,		int,				1,							false,	true,	true,	None				) \
	_( UseRegularNMS,			SECT_POST_PROCESS,		bool,				true,						false,	true,	true,	None				) \
	_( SigmoidOnCLS,			SECT_POST_PROCESS,		bool,				false,						false,	true,	true,	None				) \
	_( PythonFile,				SECT_POST_PROCESS,		std::string,		"",							false,	false,	false,	None				) \
	_( PostProcessorInputs,		SECT_POST_PROCESS,		DG::vec_int,		DG::vec_int(),				false,	false,	false,	None				) \
	_( OutputNumClasses,		SECT_POST_PROCESS,		int,				20,							false,	false,	true,	None				) \
	_( OutputNumLandmarks,		SECT_POST_PROCESS,		size_t,				17,							false,	false,	true,	None				) \
	_( OutputSoftmaxEn,			SECT_POST_PROCESS,		bool,				false,						false,	false,	true,	None				) \
	_( OutputClassIDAdjustment,	SECT_POST_PROCESS,		int,				0,							false,	false,	true,	None				) \
	_( OutputYoloAnchors,		SECT_POST_PROCESS,		DG::vec3d_int,		DG::vec3d_int(0),			false,	false,	true,	None				) \
	_( OutputYoloStrides,		SECT_POST_PROCESS,		DG::vec_int,		DG::vec_int({8, 16, 32}),	false,	false,	true,	None				) \
	_( OutputYoloRegMax,		SECT_POST_PROCESS,		int,				16,							false,	false,	false,	None				) \
	_( OutputYoloBoxScales,		SECT_POST_PROCESS,		DG::vec_flt,		DG::vec_flt(),				false,	false,	false,	None				) \
	_( OutputShapePerLandmark,	SECT_POST_PROCESS,		int,				3,							false,	false,	false,	None				) \
	_( OutputTensorLayout,		SECT_POST_PROCESS,		std::string,		"NHWC",						false,	false,	true,	None				) \
	_( YScale,					SECT_POST_PROCESS,		double,				1,							false,	false,	false,	None				) \
	_( XScale,					SECT_POST_PROCESS,		double,				1,							false,	false,	false,	None				) \
	_( HScale,					SECT_POST_PROCESS,		double,				1,							false,	false,	false,	None				) \
	_( WScale,					SECT_POST_PROCESS,		double,				1,							false,	false,	false,	None				) \
	_( NMSRadius,				SECT_POST_PROCESS,		double,				10,							false,	false,	false,	None				) \
	_( Stride,					SECT_POST_PROCESS,		int,				16,							false,	false,	false,	None				) \
	_( PoseThreshold,			SECT_POST_PROCESS,		double,				0.8,						false,	true,	true,	None				) \
	_( LabelsPath,				SECT_POST_PROCESS,		std::string,		"",							false,	false,	false,	None				) \
	_( LandmarkLabels,			SECT_POST_PROCESS,		DG::vec_str,		DG::vec_str(),				false,	false,	false,	None				) \
	_( Connections,				SECT_POST_PROCESS,		DG::map_str_vec,	DG::map_str_vec(),			false,	false,	false,	None				)

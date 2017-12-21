#define __MODULE__ "EV.Utils.AVBuilder"

#include <YXC_AVKernel/YXV_AVBuilder.h>
#include <YXC_AVKernel/Utils/_YXV_AVBuilder.hpp>
#include <YXC_AVKernel/Utils/_YXV_AVBuilderElementBase.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>

using namespace _YXV_AVBuilder;

extern "C"
{
	YXC_Status YXV_AVBuilderCreate(YXV_VFilterType vfType, yuint32_t uWidth, yuint32_t uHeight, YXV_PixFmt pixFmt, yuint32_t uNumChannels, yuint32_t uFreq,
		YXV_SampleFmt sampleFmt, YXV_AVBuilder* ppBuilder)
	{
		_YCHK_MAL_R1(pBuilder, _AVBuilder);
		new (pBuilder) _AVBuilder();

		YXCLib::HandleRef<_AVBuilder*> pBuilder_res(pBuilder, YXCLib::TDelete<_AVBuilder>);

		YXC_Status rc = pBuilder->Create(vfType, uWidth, uHeight, pixFmt, uNumChannels, uFreq, sampleFmt);
		_YXC_CHECK_RC_RET(rc);

		*ppBuilder = _AVBHdl_B(pBuilder_res.Detach());
		return YXC_ERC_SUCCESS;
	}

	void YXV_AVBuilderDestroy(YXV_AVBuilder builder)
	{
		_AVBuilder* pBuilder = _AVBPtr_B(builder);

		pBuilder->Destroy();
		pBuilder->~_AVBuilder();
		free(pBuilder);
	}

	/* Internal Data will be continually used, so don't free them. */
	YXC_API(YXC_Status) YXV_AVBuilderOpenSources(YXV_AVBuilder builder, YXV_AVSourceList* pSourceList)
	{
		_AVBuilder* pBuilder = _AVBPtr_B(builder);

		YXC_Status rc = pBuilder->OpenSources(pSourceList);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_API(void) YXV_AVBuilderCloseSources(YXV_AVBuilder builder, YXV_AVSourceList* pSourceList)
	{
		_AVBuilder* pBuilder = _AVBPtr_B(builder);

		pBuilder->CloseSources(pSourceList);
	}

	YXC_API(YXC_Status) YXV_AVBuilderFrame(YXV_AVBuilder builder, yint64_t i64FrameDuration)
	{
		_AVBuilder* pBuilder = _AVBPtr_B(builder);

		YXC_Status rc = pBuilder->NextFrame(i64FrameDuration);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_API(void) YXV_AVBuilderFill(YXV_AVBuilder builder, YXV_Frame* pImgData, YXV_Frame* pAudioData)
	{
		_AVBuilder* pBuilder = _AVBPtr_B(builder);

		pBuilder->FillData(builder, pImgData, pAudioData);
	}

	YXC_API(YXC_Status) YXV_AVBuilderSeek(YXV_AVBuilder builder, yint64_t i64SeekTo)
	{
		_AVBuilder* pBuilder = _AVBPtr_B(builder);

		YXC_Status rc = pBuilder->Seek(i64SeekTo);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_API(YXC_Status) YXV_AVBuilderSkip(YXV_AVBuilder builder, yint64_t i64SkipTime)
	{
		_AVBuilder* pBuilder = _AVBPtr_B(builder);

		YXC_Status rc = pBuilder->Skip(i64SkipTime);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}
};

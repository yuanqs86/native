#define __MODULE__ "EV.Utils.VFilterFAVF"

#include <YXC_AVKernel/Codec/ffwrap/ffcommon.hpp>
#include <YXC_AVKernel/Utils/_YXV_VFilterBase.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_TextEncoding.h>
#include <YXC_AVKernel/Utils/_YXV_VFilterFAVF.hpp>

#if YXC_IS_32BIT
static __forceinline void _sse_alpha_blend_yuv420(const ybyte_t* src, ybyte_t* dst, ybyte_t* alpha, int number, int opacity)
{
	yint16_t opcArr1[4] = { opacity, opacity, opacity, opacity };
	yint16_t opc255[4] = { 255, 255, 255, 255 };
	ybyte_t opc255_8[8] = { 255, 255, 255, 255, 255, 255, 255, 255 };
	yint16_t opc128[4] = { 128, 128, 128, 128 };
	// yint16_t opcArr2[4] = { 255 - opacity, 255 - opacity, 255 - opacity };
	/* mm0 = src, mm1 = dst. mm2 = alpha */
	__asm
	{
		mov edx, number;
		mov ecx, 0;
begin:
		mov eax, src;
		movq mm0, qword ptr [eax+ecx];

		mov eax, dst;
		movq mm1, qword ptr [eax+ecx];

		mov esi, alpha;
		cmp esi, 0;
		je set_alpha_to_255;
		add esi, ecx;
		jmp do_work;

set_alpha_to_255:
		lea esi, opc255_8;

		//mm2 = yint32_t a = (yint32_t)YXC_Round(a_raw * opacity);
		//mm3 = zero
do_work:
		movq mm2, qword ptr [esi];
		pxor mm3, mm3;
		punpcklbw mm2, mm3;
		movq mm4, opcArr1;
		pmullw mm2, mm4;
		movq mm4, opc128;
		paddusw mm2, mm4;
		psrlw mm2, 8;

		/* mm4 = src_y_row[0] * a */
		movq mm4, mm1;
		punpcklbw mm4, mm3;
		movq mm5, opc255;
		psubusw mm5, mm2;
		pmullw mm4, mm5;

		/* mm6 = result. */
		movq mm6, mm0;
		punpcklbw mm6, mm3;
		pmullw mm6, mm2;
		paddusw mm6, mm4;
		movq mm4, opc128;
		paddusw mm6, mm4;
		psrlw mm6, 8;

		movq mm2, qword ptr [esi];

		/* High 4 bits. */
		punpckhbw mm2, mm3;
		movq mm4, opcArr1;
		pmullw mm2, mm4;
		movq mm4, opc128;
		paddusw mm2, mm4;
		psrlw mm2, 8;

		/* mm4 = src_y_row[0] * a */
		movq mm4, mm1;
		punpckhbw mm4, mm3;
		movq mm5, opc255;
		psubusw mm5, mm2;
		pmullw mm4, mm5;

		/* mm6 = result. */
		movq mm7, mm0;
		punpckhbw mm7, mm3;
		pmullw mm7, mm2;
		paddusw mm7, mm4;
		movq mm4, opc128;
		paddusw mm7, mm4;
		psrlw mm7, 8;

		packuswb mm6, mm7;
		mov eax, dst;
		movq qword ptr [eax+ecx], mm6;

		add ecx, 8;
		cmp ecx, edx;
		jl begin;

		EMMS;
	}
}
#else
extern "C" void __cdecl _sse_alpha_blend_yuv420(const ybyte_t* src, ybyte_t* dst, ybyte_t* alpha, int number, int opacity);
#endif

namespace _YXV_AVUtils
{
	typedef void (*filter_buffer_free_func)(struct AVFilterBuffer *buf);

	_VFont_FAVF::_VFont_FAVF() : _fcBuffer(NULL), _fcDrawText(NULL), _fcSink(NULL), _g(NULL), _fontSize(12), _bLinked(FALSE)
	{
		memset(this->_szCfg, 0, sizeof(this->_szCfg));
		memset(this->_fontStyle, 0, sizeof(this->_fontStyle));
		_cx=_cy=0;
	}

	_VFont_FAVF::~_VFont_FAVF()
	{
		if (this->_fcBuffer)
		{
			avfilter_free(this->_fcBuffer);
			this->_fcBuffer = NULL;
		}

		if (this->_fcDrawText)
		{
			avfilter_free(this->_fcDrawText);
			this->_fcDrawText = NULL;
		}

		if (this->_fcSink)
		{
			avfilter_free(this->_fcSink);
			this->_fcSink = NULL;
		}

		if (this->_g)
		{
			avfilter_graph_free(&this->_g);
		}
	}

	YXC_Status _VFont_FAVF::Init(const YXV_TFontDesc* desc)
	{
		YXC_TECharToUTF8(desc->szFace, YXC_STR_NTS, (yuint8_t*)this->_fontStyle, YXC_STRING_ARR_LEN(this->_fontStyle), NULL, NULL);
		this->_fontSize = desc->uFontW;
		return YXC_ERC_SUCCESS;
	}

	static void _CalculateColor(yuint32_t uColor, char* szColor)
	{
		yuint32_t r = (uColor & 0x0000ff);
		yuint32_t g = (uColor & 0x00ff00) >> 8;
		yuint32_t b = (uColor & 0xff0000) >> 16;

		sprintf(szColor, "#%02x%02x%02x", r, g, b);
	}

	static void _CalculateXYParam(double x, double y, double w, double h, yuint32_t uAlign, char* szX, char* szY)
	{
		/*
			7 8 9
			4 5 6
			1 2 3
		*/
		yuint32_t uVAlign = 2 - (uAlign - 1) / 3;
		yuint32_t uHAlign = (uAlign - 1) % 3;

		if (uHAlign == 0)
		{
			sprintf(szX, "%d", YXC_Round(x));
		}
		else if (uHAlign == 2)
		{
			sprintf(szX, "%d-text_w", YXC_Round(x + w));
		}
		else
		{
			sprintf(szX, "%d-text_w/2", YXC_Round(x + w / 2));
		}

		if (uVAlign == 0)
		{
			sprintf(szY, "%d", YXC_Round(y + h));
		}
		else if (uVAlign == 2)
		{
			sprintf(szY, "%d-text_h", YXC_Round(y));
		}
		else
		{
			sprintf(szY, "%d-text_h/2", YXC_Round(y + h / 2));
		}
	}

	YXC_Status _VFont_FAVF::Draw(double x, double y, double w, double h, const char* text,
		yuint32_t uColor, yuint32_t uAlign, YXV_Frame* frame)
	{
		//char szX2[320];
		//sprintf(szX2, "Draw text at %d\n", GetTickCount());
		//::OutputDebugStringA(szX2);

		if (this->_g == NULL)
		{
			this->_g = avfilter_graph_alloc();
			_YXC_CHECK_FF_PTR_RET(this->_g);
		}

		int ffret = -1;

		char buffer_args[400];
		if (this->_fcBuffer == NULL)
		{
			sprintf(buffer_args, "width=%d:height=%d:pix_fmt=%s:time_base=1/25:sar=1", frame->u1.picDesc.w, frame->u1.picDesc.h,
				frame->u1.picDesc.pixFmt == YXV_PIX_FMT_BGR24 ? "bgr24" : "yuv420p");

			AVFilter* filter_buffer = avfilter_get_by_name("buffer");
			ffret = avfilter_graph_create_filter(&this->_fcBuffer, filter_buffer, "buffer", buffer_args, NULL, this->_g);
			_YXC_CHECK_FF_RET(ffret);
		}
		else /* w, h will be never changed. */
		{
			// YXV_FFWrap::av_opt_set_format(this->_fcBuffer->priv, "width", "%d", frame->u1.picDesc.w);
			// YXV_FFWrap::av_opt_set_format(this->_fcBuffer->priv, "height", "%d", frame->u1.picDesc.h);
			// YXV_FFWrap::av_opt_set_format(this->_fcBuffer->priv, "pix_fmt", "%s", frame->u1.picDesc.pixFmt == YXV_PIX_FMT_BGR24 ?
			//	"bgr24" : "yuv420p");
		}

		if (this->_fcSink == NULL)
		{
			AVFilter* filter_sink = avfilter_get_by_name("buffersink");
			ffret = avfilter_graph_create_filter(&this->_fcSink, filter_sink, "result_sink", NULL, NULL, this->_g);
			_YXC_CHECK_FF_RET(ffret);
		}

		char szX[20], szY[20], szColor[20];
		_CalculateXYParam(x, y, w, h, uAlign, szX, szY);
		_CalculateColor(uColor, szColor);

		char utf8Text[256];

		YXC_TECharToUTF8(text, YXC_STR_NTS, (yuint8_t*)utf8Text, 255, NULL, NULL);

		if (this->_fcDrawText == NULL)
		{
			AVFilter* filter_drawtext = avfilter_get_by_name("drawtext");

			sprintf(buffer_args, "fontfile=%s:fontsize=%d:fontcolor=%s:x=%s:y=%s:text='%s'", this->_fontStyle, this->_fontSize,
				szColor, szX, szY, utf8Text);
			ffret = avfilter_graph_create_filter(&this->_fcDrawText, filter_drawtext, "drawtext_filter", buffer_args, NULL, this->_g);
			_YXC_CHECK_FF_RET(ffret);
		}
		else /* Modify x, y, w, h, information. */
		{
			//sprintf(buffer_args, "fontfile=%s:fontsize=%d:fontcolor=%s:x=%s:y=%s:text='%s'", this->_fontStyle, this->_fontSize,
			//	szColor, szX, szY, utf8Text);
			//av_opt_set(this->_fcDrawText, "drawtext_filter", buffer_args, 0);
			//av_opt_set(this->_fcDrawText->priv, "drawtext_filter", buffer_args, 0);
			YXV_FFWrap::av_opt_set_format(this->_fcDrawText->priv, "x", "%s", szX);
			YXV_FFWrap::av_opt_set_format(this->_fcDrawText->priv, "y", "%s", szY);
			YXV_FFWrap::av_opt_set_format(this->_fcDrawText->priv, "text", "%s", utf8Text);
			if ((_cx!=x) || (_cy!=y))
			{
				this->_fcDrawText->input_pads[0].config_props(this->_fcDrawText->inputs[0]);
			}
		}
		_cx=x;
		_cy=y;
		if (!this->_bLinked)//
		{
			ffret = avfilter_link(this->_fcBuffer, 0, this->_fcDrawText, 0);
			_YXC_CHECK_FF_RET(ffret);

			ffret = avfilter_link(this->_fcDrawText, 0, this->_fcSink, 0);
			_YXC_CHECK_FF_RET(ffret);

			ffret = avfilter_graph_config(this->_g, NULL);
			_YXC_CHECK_FF_RET(ffret);

			this->_bLinked = TRUE;
		}
		else
		{
			// i=avfilter_config_links(this->_fcDrawText);
		}

		AVFrame f = {0};
		f.format = YXV_FFWrap::PixFmtFromFF(frame->u1.picDesc.pixFmt);

		f.width = frame->u1.picDesc.w;
		f.height = frame->u1.picDesc.h;

		yuint32_t ix = 0;
		while (ix < AV_NUM_DATA_POINTERS && ix < YXV_MAX_NUM_PLANARS)
		{
			f.linesize[ix] = frame->uSize[ix];
			f.data[ix] = frame->pData[ix];
			++ix;
		}

		ffret = av_buffersrc_add_frame_flags(this->_fcBuffer, &f, AV_BUFFERSRC_FLAG_PUSH | AV_BUFFERSRC_FLAG_KEEP_REF);
		_YXC_CHECK_FF_RET(ffret);

		AVFrame* f3 = av_frame_alloc();
		ffret = av_buffersink_get_frame_flags(this->_fcSink, f3, 0);
		_YXC_CHECK_FF_RET(ffret);

		YXCLib::HandleRef<AVFrame**> f3_res(&f3, av_frame_free);

		YXV_Frame vf = *frame;
		ix = 0;
		while (ix < AV_NUM_DATA_POINTERS && ix < YXV_MAX_NUM_PLANARS)
		{
			vf.uSize[ix] = f3->linesize[ix];
			vf.pData[ix] = f3->data[ix];
			++ix;
		}
		YXV_VFrameCopy(&vf, frame);
		return YXC_ERC_SUCCESS;
	}

	_VPicture_FAVF::_VPicture_FAVF() : _iLastH(0), _iLastW(0), _uBufferW(0), _uBufferH(0)
	{
		memset(&this->_scaled, 0, sizeof(YXV_Frame));
		memset(&this->_img, 0, sizeof(YXV_Frame));
		memset(&this->_lastDrawInfo, 0, sizeof(YXV_PicDrawParam));

		this->_pRef = NULL;
	}

	_VPicture_FAVF::~_VPicture_FAVF()
	{
		YXV_FrameUnref(&this->_scaled);
		YXV_FrameUnref(&this->_img);
	}

	YXC_Status _VPicture_FAVF::_Reset(yint32_t w, yint32_t h, const YXV_PicDrawParam* picInfo, YXV_PixFmt fmt)
	{
		YXC_Status rc = YXC_ERC_UNKNOWN;

		if (w != this->_uBufferW || h != this->_uBufferH)
		{
			yuint32_t bufferW = w;
			yuint32_t bufferH = h;

			YXV_FrameUnref(&this->_scaled);

			if (fmt == YXV_PIX_FMT_YUV420P)
			{
				fmt = YXV_PIX_FMT_YUVA420P;
			}
			else if (fmt == YXV_PIX_FMT_BGR24)
			{
				fmt = YXV_PIX_FMT_RGBA;
			}
			YXV_PicDesc desc = { fmt, w, h, 0 };
			rc = YXV_VFrameAlloc(&desc, &this->_scaled);
			_YXC_CHECK_RC_RET(rc);

			YXV_FrameZero(&this->_scaled);

			this->_uBufferW = bufferW;
			this->_uBufferH = bufferH;
		}

		rc = this->_ScaleAndSplitAlphaChannel(w, h, picInfo, fmt);
		_YXC_CHECK_RC_RET(rc);

		this->_iLastW = w;
		this->_iLastH = h;
		this->_lastDrawInfo = *picInfo;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _VPicture_FAVF::_LoadFrame()
	{
		YXV_FFFormat fmt;
		YXV_VCodecID vCodecId;
		YXV_ACodecID aCodecId;
		YXV_FFormatID fmtId;

		YXC_Status rc = YXV_FFFormatCreateRead((char*)this->_imgPath, &vCodecId, &aCodecId, &fmtId, &fmt);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<YXV_FFObject> fmt_res(fmt, YXV_FFObjClose);

		_YXC_CHECK_REPORT_NEW_RET(vCodecId == YXV_VCODEC_ID_PNG || vCodecId == YXV_VCODEC_ID_BMP || vCodecId == YXV_VCODEC_ID_MJPEG
			|| vCodecId == YXV_VCODEC_ID_JPEGLS || vCodecId == YXV_VCODEC_ID_JPEG2000	|| vCodecId == YXV_VCODEC_ID_GIF,
			YXC_ERC_NOT_SUPPORTED, YC("Only support bmp/png/jpeg/gif codec"));
		_YXC_CHECK_REPORT_NEW_RET(fmtId == YXV_FILE_FORMAT_ID_IMAGE2, YXC_ERC_NOT_SUPPORTED, YC("Only support image2 format."));

		YXV_VParam vParam;
		YXV_FFVFormatParam fmtParam;
		rc = YXV_FFFormatReadParamV(fmt, &vParam, &fmtParam);
		_YXC_CHECK_RC_RET(rc);

		YXV_FFDec dec;
		rc = YXV_FFDecCreate(vCodecId, YXV_ACODEC_ID_NONE, &dec);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<YXV_FFObject> dec_res(dec, YXV_FFObjClose);

		rc = YXV_FFObjConfigV(dec, &vParam, &fmtParam[1]);
		_YXC_CHECK_RC_RET(rc);

		YXV_Packet pkt = {0};
		rc = YXV_FFFormatRead(fmt, &pkt);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<YXV_Packet*> pkt_res(&pkt, YXV_PacketUnref);

		rc = YXV_FFDecProcessV2(dec, &pkt, &this->_img);
		_YXC_CHECK_RC_RET(rc);

		if (!this->_img.bGotFrame)
		{
			YXV_Packet pkt2 = {0};

			rc = YXV_FFDecProcessV2(dec, &pkt2, &this->_img);
			_YXC_CHECK_RC_RET(rc);
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _VPicture_FAVF::_ScaleAndSplitAlphaChannel(yuint32_t w, yuint32_t h, const YXV_PicDrawParam* picInfo, YXV_PixFmt fmt)
	{
		YXV_Rect rect = picInfo->partRect;
		YXV_Frame frame2;
		if (rect.right == 0) /* Full image. */
		{
			rect.left = 0;
			rect.top = 0;
			rect.right = this->_img.u1.picDesc.w;
			rect.bottom = this->_img.u1.picDesc.h;

			frame2 = this->_img;
		}
		else
		{
			YXV_VFrameROI(&this->_img, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, &frame2);
		}

		if (picInfo->bZoom) /* Calculate w, h, ratio. */
		{

		}

		YXV_PicDesc srcDesc = this->_img.u1.picDesc;
		srcDesc.w = rect.right - rect.left;
		srcDesc.h = rect.bottom - rect.top;

		frame2.u1.picDesc = srcDesc;

		YXV_PicDesc dstDesc = { fmt, w, h };

		YXV_VScalerParam vsParam = {0};
		vsParam.interpolation = YXV_IMAGE_INTER_LINEAR;

		YXV_VScaler vs;
		YXC_Status rc = YXV_VScalerCreate(&srcDesc, &dstDesc, &vsParam, &vs);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<YXV_VScaler> vs_res(vs, YXV_VScalerDestroy);

		rc = YXV_VScalerScale(vs, &frame2, &this->_scaled);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	static ybool_t _IsEqualDrawInfo(const YXV_PicDrawParam* i1, const YXV_PicDrawParam* i2)
	{
		return i1->bZoom == i2->bZoom && i1->interpolation == i2->interpolation	&&
			i1->partRect.left == i2->partRect.left && i1->partRect.top == i2->partRect.top &&
			i1->partRect.right == i2->partRect.right && i1->partRect.bottom == i2->partRect.bottom;
	}

	static void _DoYUVBlendSSE(YXV_Frame* dst, YXV_Frame* f, int iX, int iY, int iW, int iH, double opacity)
	{
		ybyte_t opacity8 = YXC_Round(opacity * 255);

		ybyte_t* dst_y = dst->pData[0] + iY * dst->uSize[0] + iX;
		ybyte_t* dst_v = dst->pData[1] + iY / 2 * dst->uSize[1] + iX / 2;
		ybyte_t* dst_u = dst->pData[2] + iY / 2 * dst->uSize[2] + iX / 2;
		ybyte_t* a2 = (ybyte_t*)malloc(iW);
		YXCLib::HandleRef<void*> a2_res(a2, free);

		ybyte_t* src_y = f->pData[0];
		ybyte_t* src_v = f->pData[1];
		ybyte_t* src_u = f->pData[2];
		ybyte_t* alpha_planar = f->pData[3];
		for (yuint32_t i = 0; i < iH; ++i)
		{
			ybyte_t* dst_y_row = dst_y, *src_y_row = src_y;
			ybyte_t* dst_u_row = dst_u, *src_u_row = src_u;
			ybyte_t* dst_v_row = dst_v, *src_v_row = src_v;
			ybyte_t* alpha_row = alpha_planar;

			_sse_alpha_blend_yuv420(src_y_row, dst_y_row, alpha_row, iW, opacity8);
			if (i % 2 == 0)
			{
				if (alpha_row != NULL)
				{
					for (int j = 0; j < iW / 2; ++j)
					{
						a2[j] = alpha_row[j * 2];
					}
					_sse_alpha_blend_yuv420(src_u_row, dst_u_row, a2, iW / 2, opacity8);
					_sse_alpha_blend_yuv420(src_v_row, dst_v_row, a2, iW / 2, opacity8);
				}
				else
				{
					_sse_alpha_blend_yuv420(src_u_row, dst_u_row, alpha_row, iW / 2, opacity8);
					_sse_alpha_blend_yuv420(src_v_row, dst_v_row, alpha_row, iW / 2, opacity8);
				}

				src_u += f->uSize[1];
				src_v += f->uSize[2];
				dst_u += dst->uSize[1];
				dst_v += dst->uSize[2];
			}

			src_y += f->uSize[0];
			alpha_planar += f->uSize[3];
			dst_y += dst->uSize[0];
		}
	}

	static void _DoYUVBlend(YXV_Frame* dst, YXV_Frame* f, int iX, int iY, int iW, int iH, double opacity)
	{
		yuint32_t iW2 = iW / 16 * 16;

#if 1
		_DoYUVBlendSSE(dst, f, iX, iY, iW2, iH, opacity);

		iX += iW2;
		iW -= iW2;
#else
		iW2 = 0;
#endif /* YXC_PLATFORM_X86 */

		if (iW > 0)
		{
			ybyte_t* dst_y = dst->pData[0] + iY * dst->uSize[0] + iX;
			ybyte_t* dst_v = dst->pData[1] + iY / 2 * dst->uSize[1] + iX / 2;
			ybyte_t* dst_u = dst->pData[2] + iY / 2 * dst->uSize[2] + iX / 2;

			ybyte_t* src_y = f->pData[0] + iW2;
			ybyte_t* src_v = f->pData[1] + iW2 / 2;
			ybyte_t* src_u = f->pData[2] + iW2 / 2;
			ybyte_t* alpha_planar = f->pData[3] + iW2;
			for (yuint32_t i = 0; i < iH; ++i)
			{
				ybyte_t* dst_y_row = dst_y, *src_y_row = src_y;
				ybyte_t* dst_u_row = dst_u, *src_u_row = src_u;
				ybyte_t* dst_v_row = dst_v, *src_v_row = src_v;
				ybyte_t* alpha_row = alpha_planar;

				for (yuint32_t j = 0; j < iW / 2; ++j)
				{
					yint32_t a_raw = 255, a2_raw = 255;
					if (f->pData[3] != NULL)
					{
						a_raw = alpha_row[0];
						a2_raw = alpha_row[1];
						alpha_row += 2;
					}
					yint32_t a = (yint32_t)YXC_Round(a_raw * opacity);
					yint32_t a2 = (yint32_t)YXC_Round(a2_raw * opacity);

					dst_y_row[0] = (dst_y_row[0] * (255 - a) + src_y_row[0] * a) / 255;
					dst_y_row[1] = (dst_y_row[1] * (255 - a2) + src_y_row[1] * a2) / 255;
					dst_u_row[0] = (dst_u_row[0] * (255 - a) + src_u_row[0] * a) / 255;
					dst_v_row[0] = (dst_v_row[0] * (255 - a) + src_v_row[0] * a) / 255;

					dst_y_row += 2;
					src_y_row += 2;
					++dst_u_row;
					++src_u_row;
					++dst_v_row;
					++src_v_row;
				}

				src_y += f->uSize[0];
				alpha_planar += f->uSize[3];
				dst_y += dst->uSize[0];

				if (i % 2 != 0)
				{
					src_u += f->uSize[1];
					src_v += f->uSize[2];
					dst_u += dst->uSize[1];
					dst_v += dst->uSize[2];
				}
			}
		}
	}

	YXC_Status _VPicture_FAVF::Draw(double x, double y, double w, double h, const YXV_PicDrawParam* picInfo, YXV_Frame* dst)
	{
		yint32_t iX = YXC_Round(x), iY = YXC_Round(y), iW = YXC_Round(w), iH = YXC_Round(h);

		if (iX < 0)
		{
			iW += iX;
			iX = 0;
		}

		if (iY < 0)
		{
			iH += iY;
			iY = 0;
		}

		if (iX + iW > dst->u1.picDesc.w)
		{
			iW = dst->u1.picDesc.w - iX;
		}

		if (iY + iH > dst->u1.picDesc.h)
		{
			iH = dst->u1.picDesc.h - iY;
		}

		YXV_Frame f = {0};
		if (this->_pRef == NULL)
		{
			if (iW != this->_iLastW || iH != this->_iLastH || !_IsEqualDrawInfo(&_lastDrawInfo, picInfo))
			{
				YXC_Status rc = this->_Reset(iW, iH, picInfo, dst->u1.picDesc.pixFmt);
				_YXC_CHECK_RC_RET(rc);
			}
			f = this->_scaled;
		}
		else
		{
			if (picInfo->partRect.right > 0)
			{
				YXV_VFrameROI(this->_pRef, picInfo->partRect.left, picInfo->partRect.top, iW, iH, &f);
			}
			else
			{
				f = *this->_pRef;
			}
		}

		ybool_t bYUV = dst->u1.picDesc.pixFmt == YXV_PIX_FMT_YUV420P;
		ybool_t bYUV4= dst->u1.picDesc.pixFmt == YXV_PIX_FMT_YUV444P;
		if (bYUV || bYUV4)
		{
			if (bYUV)
			{
				_DoYUVBlend(dst, &f, iX, iY, iW, iH, picInfo->opacity);
			}
			if (bYUV4)
			{
				ybyte_t* dst_y = dst->pData[0] + iY * dst->uSize[0] + iX;
				ybyte_t* dst_v = dst->pData[1] + iY / 2 * dst->uSize[1] + iX / 2;
				ybyte_t* dst_u = dst->pData[2] + iY / 2 * dst->uSize[2] + iX / 2;

				ybyte_t* src_y = f.pData[0];
				ybyte_t* src_v = f.pData[1];
				ybyte_t* src_u = f.pData[2];
				ybyte_t* alpha_planar = f.pData[3];

				for (yuint32_t i = 0; i < iH; ++i)
				{
					ybyte_t* dst_y_row = dst_y, *src_y_row = src_y;
					ybyte_t* dst_u_row = dst_u, *src_u_row = src_u;
					ybyte_t* dst_v_row = dst_v, *src_v_row = src_v;
					ybyte_t* alpha_row = alpha_planar;

					for (yuint32_t j = 0; j < iW; ++j)
					{
						yint32_t a_raw = alpha_row[0];
						if (alpha_row != NULL)
						{
							a_raw = alpha_row[0];
							alpha_row += 1;
						}
						yint32_t a = (yint32_t)YXC_Round(a_raw * picInfo->opacity);
						dst_y_row[0] = (dst_y_row[0] * (255 - a) + src_y_row[0] * a) / 255;
						//	dst_y_row[1] = (dst_y_row[1] * (255 - a2) + src_y_row[1] * a2) / 255;
						dst_u_row[0] = (dst_u_row[0] * (255 - a) + src_u_row[0] * a) / 255;
						dst_v_row[0] = (dst_v_row[0] * (255 - a) + src_v_row[0] * a) / 255;

						dst_y_row += 1;
						src_y_row += 1;
						++dst_u_row;
						++src_u_row;
						++dst_v_row;
						++src_v_row;
					}

					src_y += f.uSize[0];
					alpha_planar += f.uSize[3];
					dst_y += dst->uSize[0];

					//	if (i % 2 != 0)
					{
						src_u += f.uSize[1];
						src_v += f.uSize[2];
						dst_u += dst->uSize[1];
						dst_v += dst->uSize[2];
					}
				}
			}
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _VPicture_FAVF::Init(const char* path)
	{
		YXC_Status rc = YXC_TECharToUTF8(path, YXC_STR_NTS, (yuint8_t*)this->_imgPath, YXC_MAX_CCH_PATH_UTF8, NULL, NULL);
		_YXC_CHECK_RC_RET(rc);

		rc = this->_LoadFrame();
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _VPicture_FAVF::InitFromShallowReference(YXV_Frame* alphaFrame)
	{
		this->_pRef = alphaFrame;
		return YXC_ERC_SUCCESS;
	}

	void _VPicture_FAVF::QueryInfo(yuint32_t* w, yuint32_t* h)
	{
		*w = this->_img.u1.picDesc.w;
		*h = this->_img.u1.picDesc.h;
	}

	_VFilter_FAVF::_VFilter_FAVF() : _VFilter()
	{
	}

	_VFilter_FAVF::~_VFilter_FAVF()
	{

	}

	YXC_Status _VFilter_FAVF::Init(YXV_Frame* vSample)
	{
		this->_sample = vSample;
		this->_desc = vSample->u1.picDesc;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _VFilter_FAVF::DrawText2(double x, double y, double w, double h, const char* text, yuint32_t uColor,
		yuint32_t uAlign, _VFont* font)
	{
		_VFont_FAVF* font_favf = (_VFont_FAVF*)font;

		YXC_Status rc = font_favf->Draw(x, y, w, h, text, uColor, uAlign, this->_sample);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _VFilter_FAVF::DrawPicture(_VPicture* picture, double x, double y, double w, double h, const YXV_PicDrawParam* picInfo)
	{
		_VPicture_FAVF* pic = (_VPicture_FAVF*)picture;

		YXC_Status rc = pic->Draw(x, y, w, h, picInfo, this->_sample);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	//YXC_Status _VFilter_FAVF::FloodFill(double x, double y, double w, double h, yuint32_t uColor, yuint32_t uNewColor,
	//	yuint32_t pixDiff)
	//{
	//	YXV_Frame f2 = {0};
	//	YXV_VFrameROI(this->_sample, x, y, w, h, &f2);

	//	YXV_VFrameFloodReplace(&f2, uColor, uNewColor, pixDiff);
	//	return YXC_ERC_SUCCESS;
	//}
}

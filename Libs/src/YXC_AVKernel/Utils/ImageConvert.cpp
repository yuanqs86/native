#include <YXC_AVKernel/YXV_AVUtils.h>
extern "C"
{
	#include <libswscale/swscale.h>
}
#include "assert.h"

namespace
{
	inline byte ADJUST(double tmp)
	{
		return (byte)((tmp >= 0 && tmp <= 255)?tmp:(tmp < 0 ? 0 : 255));
	}

	int _InitResizeFlags()
	{
		HKEY hKey;
		DWORD dwDisposition;
		LSTATUS ls = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\YXCLib\\Settings\\Image", 0, NULL, 0, KEY_READ, NULL, &hKey, &dwDisposition);

		if (ls == ERROR_SUCCESS)
		{
			DWORD dwMethod = SWS_POINT, dwType;
			DWORD dwCbData = sizeof(DWORD);

			ls = RegQueryValueExW(hKey, L"Interpolation", NULL, &dwType, (LPBYTE)&dwMethod, &dwCbData);
			if (ls == ERROR_SUCCESS && dwType == REG_DWORD)
			{
				return dwMethod;
			}
		}

		SYSTEM_INFO swsInfo;
		GetSystemInfo(&swsInfo);

		DWORD dwStart1 = yxcwrap_getcpu_clock();
		Sleep(3);
		DWORD dwStart2 = yxcwrap_getcpu_clock();

		DWORD dwMHZ = (dwStart2 - dwStart1) / 3000;

		yuint64_t uTotalFreq = (yuint64_t)dwMHZ * YXC_BYTES_IN_MB * swsInfo.dwNumberOfProcessors;
		if (uTotalFreq > 16 * (yuint64_t)YXC_BYTES_IN_GB)
		{
			return SWS_BICUBIC;
		}
		else if (uTotalFreq > 8 * (yuint64_t)YXC_BYTES_IN_GB)
		{
			return SWS_BILINEAR;
		}
		else if (uTotalFreq > 4 * (yuint64_t)YXC_BYTES_IN_GB)
		{
			return SWS_FAST_BILINEAR;
		}
		return SWS_POINT;
	}

	static int gs_yuv_to_rgb_table[256][256][256];
	static ybyte_t gs_yr_y_table[256];
	static ybyte_t gs_yr_ub_table[256];
	static ybyte_t gs_yr_ur_table[256];
	static ybyte_t gs_yr_ug_table[256];
	static ybyte_t gs_yr_uv_table[256];

	static ybyte_t gs_ry_by_table[256];
	static ybyte_t gs_ry_ry_table[256];
	static ybyte_t gs_ry_gy_table[256];
	static ybyte_t gs_ry_byu_table[256];
	static ybyte_t gs_ry_ryv_table[256];

	int _init_yuv_rgb_table()
	{
		int C, D, E;
		int R, G, B;
		int* pRGBV3 = (int*)gs_yuv_to_rgb_table;
		for (int v = 0; v < 256; ++v)
		{
			int* pRGBV2 = pRGBV3;
			for (int u = 0; u < 256; ++u)
			{
				for (int y = 0; y < 256; ++y)
				{
					B=(int)((1.164*(y-16)+2.018*(u-128))*1024);
					B=B>>10;
					B=(B<0)?0:((B>255)?255:B);

					G=(int)((1.164*(y-16)-0.813*(v-128)-0.391*(u-128))*1024);
					G=G>>10;
					G=(G<0)?0:((G>255)?255:G);

					R=(int)((1.164*(y-16)+1.596*(v-128))*1024);
					R=R>>10;
					R=(R<0)?0:((R>255)?255:R);

					//R = ADJUST(y + 1.370705 * (v - 128)); // r分量值
					//G = ADJUST(y - 0.698001 * (u - 128)  - 0.703125 * (v - 128)); // g分量值
					//B = ADJUST(y + 1.732446 * (u - 128)); // b分量值

					//C = y - 16;
					//D = u - 128;
					//E = v - 128;
					//R = ADJUST(( 298 * C          + 409 * E + 128) >> 8);
					//G = ADJUST(( 298 * C - 100 * D - 208 * E + 128) >> 8);
					//B = ADJUST(( 298 * C + 516 * D          + 128) >> 8);
					//R  =   ((R   -   128)   *   .6   +   128  )>255?255:(R   -   128)   *   .6   +   128;
					//G  =   ((G   -   128)   *   .6  +   128  )>255?255:(G   -   128)   *   .6   +   128;
					//B  =   ((B   -   128)   *   .6   +   128  )>255?255:(B   -   128)   *   .6  +   128;

					char* pRGB = (char*)(pRGBV2 + y); // put into the last 3 bytes
					*pRGB++ = B;
					*pRGB++ = G;
					*pRGB++ = R;
				}
				pRGBV2 += 256;
			}
			pRGBV3 += 256 * 256;
		}
		return 0;
	}

	int _init_rgb_yuv_table()
	{
		return 0;
	}

	int gs_init_yuv = _init_yuv_rgb_table();
	int gs_init_rgb = _init_rgb_yuv_table();

	__forceinline static int _get_rgb_pix(int y, int u, int v)
	{
		return gs_yuv_to_rgb_table[v][u][y];
	}

	__forceinline static void _get_yuv_pix_16(char* pRGBUp, char* pRGBDown, int* y_plan, int* u_plan, int* v_plan)
	{

	}

	__forceinline static void _RGB_Read_Color(char*& y_planar_up, char*& y_planar_down, char*& v_planar, char*& u_planar,
		int rgbu1, int rgbu2, int rgbd1, int rgbd2)
	{

	}

	__forceinline static void _YUV_Read_Color(char*& y_planar_up, char*& y_planar_down,
		char*& v_planar, char*& u_planar, int& rgbu1, int& rgbu2, int& rgbd1, int& rgbd2)
	{
		unsigned char y_up_1 = *y_planar_up++;
		unsigned char y_up_2 = *y_planar_up++;

		unsigned char y_down_1 = *y_planar_down++;
		unsigned char y_down_2 = *y_planar_down++;

		unsigned char v = *v_planar++;
		unsigned char u = *u_planar++;

		rgbu1 = _get_rgb_pix(y_up_1, u, v);
		rgbu2 = _get_rgb_pix(y_up_2, u, v);

		rgbd1 = _get_rgb_pix(y_down_1, u, v);
		rgbd2 = _get_rgb_pix(y_down_2, u, v);
	}

	__forceinline static void _YUV_Fill_1(int*& rgb_up, int*& rgb_down, char*& y_planar_up,
										  char*& y_planar_down, char*& v_planar, char*& u_planar)
	{
		int rgbu1, rgbu2, rgbd1, rgbd2;

		_YUV_Read_Color(y_planar_up, y_planar_down, v_planar, u_planar, rgbu1, rgbu2, rgbd1, rgbd2);

		*rgb_up = rgbu1;
		*(int*)((char*)rgb_up + 3) = rgbu2;
		rgb_up = (int*)((char*)rgb_up + 6);
		*rgb_down = rgbd1;
		*(int*)((char*)rgb_down + 3) = rgbd2;
		rgb_down = (int*)((char*)rgb_down + 6);
	}

	__forceinline static void _YUV_Fill_2(char*& c_rgb_up, char*& c_rgb_down, char*& y_planar_up,
										  char*& y_planar_down, char*& v_planar, char*& u_planar)
	{
		int rgbu1, rgbu2, rgbd1, rgbd2;

		_YUV_Read_Color(y_planar_up, y_planar_down, v_planar, u_planar, rgbu1, rgbu2, rgbd1, rgbd2);

		char* c_color1 = (char*)&rgbu1;
		*c_rgb_up++ = *c_color1++;
		*c_rgb_up++ = *c_color1++;
		*c_rgb_up++ = *c_color1++;
		c_color1 = (char*)&rgbu2;
		*c_rgb_up++ = *c_color1++;
		*c_rgb_up++ = *c_color1++;
		*c_rgb_up++ = *c_color1++;
		c_color1 = (char*)&rgbd1;
		*c_rgb_down++ = *c_color1++;
		*c_rgb_down++ = *c_color1++;
		*c_rgb_down++ = *c_color1++;
		c_color1 = (char*)&rgbd2;
		*c_rgb_down++ = *c_color1++;
		*c_rgb_down++ = *c_color1++;
		*c_rgb_down++ = *c_color1++;
	}

	__forceinline static void _YUV420P_To_RGB_Final_Row(char* y_planar, char* v_planar, char* u_planar,
									int* rgb, int nScanH, BOOL has_last_col)
	{
		char* c_rgb = (char*)rgb;
		for (int j = 0; j < nScanH; ++j)
		{
			unsigned char y1 = *y_planar++;
			unsigned char y2 = *y_planar++;

			unsigned char v = *v_planar++;
			unsigned char u = *u_planar++;

			int rgb_color1 = _get_rgb_pix(y1, u, v);
			int rgb_color2 = _get_rgb_pix(y1, u, v);

			char* c_color1 = (char*)&rgb_color1;
			*c_rgb++ = *c_color1++;
			*c_rgb++ = *c_color1++;
			*c_rgb++ = *c_color1++;
			c_color1 = (char*)&rgb_color2;
			*c_rgb++ = *c_color1++;
			*c_rgb++ = *c_color1++;
			*c_rgb++ = *c_color1++;
		}

		if (has_last_col)
		{
			unsigned char y = *y_planar++;
			unsigned char v = *v_planar++;
			unsigned char u = *u_planar++;

			int rgb_color1 = _get_rgb_pix(y, u, v);

			char* c_color1 = (char*)&rgb_color1;
			*c_rgb++ = *c_color1++;
			*c_rgb++ = *c_color1++;
			*c_rgb++ = *c_color1++;
		}
	}

	__forceinline static void _YUV420P_To_RGB_Row(char* y_planar_up, char* y_planar_down, char* v_planar, char* u_planar,
									int* rgb_up, int* rgb_down, int nScanH, bool has_last_col)
	{
		for (int j = 0; j < nScanH - 1; ++j)
		{
			_YUV_Fill_1(rgb_up, rgb_down, y_planar_up, y_planar_down, v_planar, u_planar);
		}

		char* c_rgb_up = (char*)rgb_up;
		char* c_rgb_down = (char*)rgb_down;
		_YUV_Fill_2(c_rgb_up, c_rgb_down, y_planar_up, y_planar_down, v_planar, u_planar);

		if (has_last_col)
		{
			unsigned char y_up_1 = *y_planar_up++;
			unsigned char y_down_1 = *y_planar_down++;

			unsigned char v = *v_planar++;
			unsigned char u = *u_planar++;

			int rgb_color_up1 = _get_rgb_pix(y_up_1, u, v);
			int rgb_color_down1 = _get_rgb_pix(y_down_1, u, v);

			char* c_color1 = (char*)&rgb_color_up1;
			*c_rgb_up++ = *c_color1++;
			*c_rgb_up++ = *c_color1++;
			*c_rgb_up++ = *c_color1++;
			c_color1 = (char*)&rgb_color_down1;
			*c_rgb_down++ = *c_color1++;
			*c_rgb_down++ = *c_color1++;
			*c_rgb_down++ = *c_color1++;
		}
	}

	static void _YUV420P_To_RGB(char* rgb_dst, char* yuv_src, int nWidth, int nHeight, BOOL bInvert)
	{
		int rgb_width , u_width;
		rgb_width = ((nWidth * 3 - 1) / 4 + 1) * 4;
		u_width = (nWidth >> 1);
		int ypSize = nWidth * nHeight;
		int upSize = (ypSize >> 2);
		int offSet = 0;

		char* y_planar_up = yuv_src + nWidth * (nHeight - 2);
		char* y_planar_down = yuv_src + nWidth * (nHeight - 1);
		char* v_planar = yuv_src + (nWidth * nHeight) + (nWidth / 2) * ((nHeight - 1) / 2);
		char* u_planar = yuv_src + (nWidth * nHeight) + (nWidth / 2) * (nHeight / 2 + (nHeight - 1) / 2);

		int nScanH = nWidth / 2;
		int nScan = nHeight / 2;

		BOOL bHasLastCol = nWidth % 2 != 0;

		if (bInvert)
		{
			for (int i = nScan - 1; i >= 0; i--)
			{
				char* y_planar_row_up = y_planar_up;
				char* y_planar_row_down = y_planar_down;
				char* u_planar_row = u_planar;
				char* v_planar_row = v_planar;

				int* rgb_up = (int*)(rgb_dst + (nHeight - i * 2 - 1) * rgb_width);
				int* rgb_down = (int*)((char*)rgb_up - rgb_width);

				_YUV420P_To_RGB_Row(y_planar_row_up, y_planar_row_down, v_planar_row, u_planar_row,
					rgb_up, rgb_down, nScanH, bHasLastCol);

				u_planar -= nWidth / 2;
				v_planar -= nWidth / 2;
				y_planar_up -= nWidth * 2;
				y_planar_down -= nWidth * 2;
			}

			if (nHeight % 2 != 0) // fill last row.
			{
				_YUV420P_To_RGB_Final_Row(y_planar_down, v_planar, u_planar,
					(int*)rgb_dst, nScanH, bHasLastCol);
			}
		}
		else
		{
			for (int i = 0; i < nScan; ++i)
			{
				char* y_planar_row_up = y_planar_up;
				char* y_planar_row_down = y_planar_down;
				char* u_planar_row = u_planar;
				char* v_planar_row = v_planar;

				int* rgb_up = (int*)(rgb_dst + (nHeight - i * 2 - 1) * rgb_width);
				int* rgb_down = (int*)((char*)rgb_up - rgb_width);

				_YUV420P_To_RGB_Row(y_planar_row_up, y_planar_row_down, v_planar_row, u_planar_row,
					rgb_up, rgb_down, nScanH, bHasLastCol);

				u_planar += nWidth / 2;
				v_planar += nWidth / 2;
				y_planar_up += nWidth * 2;
				y_planar_down += nWidth * 2;
			}

			if (nHeight % 2 != 0) // fill last row.
			{
				_YUV420P_To_RGB_Final_Row(y_planar_down, v_planar, u_planar,
					(int*)rgb_dst, nScanH, bHasLastCol);
			}
		}
	}

	static void _RGB_To_YUV420P(char* rgb_src, char* yuv_dst, int nWidth, int nHeight, BOOL bInvert)
	{
		//int rgb_width , u_width;
		//rgb_width = ((nWidth * 3 - 1) / 4 + 1) * 4;
		//u_width = (nWidth >> 1);
		//int ypSize = nWidth * nHeight;
		//int upSize = (ypSize >> 2);
		//int offSet = 0;

		//char* y_planar_up = yuv_dst + nWidth * (nHeight - 2);
		//char* y_planar_down = yuv_dst + nWidth * (nHeight - 1);
		//char* v_planar = yuv_dst + (nWidth * nHeight) + (nWidth / 2) * ((nHeight - 1) / 2);
		//char* u_planar = yuv_dst + (nWidth * nHeight) + (nWidth / 2) * (nHeight / 2 + (nHeight - 1) / 2);

		//int nScanH = nWidth / 2;
		//int nScan = nHeight / 2;

		//BOOL bHasLastCol = nWidth % 2 != 0;

		//if (bInvert)
		//{
		//	for (int i = nScan - 1; i >= 0; i--)
		//	{
		//		char* y_planar_row_up = y_planar_up;
		//		char* y_planar_row_down = y_planar_down;
		//		char* u_planar_row = u_planar;
		//		char* v_planar_row = v_planar;

		//		int* rgb_up = (int*)(rgb_src + (nHeight - i * 2 - 1) * rgb_width);
		//		int* rgb_down = (int*)((char*)rgb_up - rgb_width);

		//		_RGB_To_YV12_Row(y_planar_row_up, y_planar_row_down, v_planar_row, u_planar_row,
		//			rgb_up, rgb_down, nScanH, bHasLastCol);

		//		u_planar -= nWidth / 2;
		//		v_planar -= nWidth / 2;
		//		y_planar_up -= nWidth * 2;
		//		y_planar_down -= nWidth * 2;
		//	}

		//	if (nHeight % 2 != 0) // fill last row.
		//	{
		//		_RGB_To_YV12_Final_Row(y_planar_down, v_planar, u_planar,
		//			(int*)rgv_src, nScanH, bHasLastCol);
		//	}
		//}
		//else
		//{
		//	for (int i = 0; i < nScan; ++i)
		//	{
		//		char* y_planar_row_up = y_planar_up;
		//		char* y_planar_row_down = y_planar_down;
		//		char* u_planar_row = u_planar;
		//		char* v_planar_row = v_planar;

		//		int* rgb_up = (int*)(rgb_src + (nHeight - i * 2 - 1) * rgb_width);
		//		int* rgb_down = (int*)((char*)rgb_up - rgb_width);

		//		_RGB_To_YV12_Row(y_planar_row_up, y_planar_row_down, v_planar_row, u_planar_row,
		//			rgb_up, rgb_down, nScanH, bHasLastCol);

		//		u_planar += nWidth / 2;
		//		v_planar += nWidth / 2;
		//		y_planar_up += nWidth * 2;
		//		y_planar_down += nWidth * 2;
		//	}

		//	if (nHeight % 2 != 0) // fill last row.
		//	{
		//		_RGB_To_YV12_Final_Row(y_planar_down, v_planar, u_planar,
		//			(int*)rgb_src, nScanH, bHasLastCol);
		//	}
		//}
	}

	static void _RGBResizeNN(char* dst, const char* src, int sw, int sh, int dw, int dh, int drs, BOOL bInvert)
	{
		int iXOffBuf[2048]; // max support 2048 width;

		if (dw <= 0 || dh <= 0) return;
		int* x_ofs = iXOffBuf;

		int idw = sw * dw, idh = sh * dh;
		int srs = YXV_ImageAlignRow(sw * 3, 4);
		int iLastFlag = dw - 1;

		for (int x = dw - 1; x >= 0; --x)
		{
			int nX = x * sw / dw;
			if (nX >= sw) nX = sw - 1;
			x_ofs[x] = nX * 3;
			if (x_ofs[x] == sw - 1)
			{
				iLastFlag = x;
			}
		}

		if (bInvert)
		{
			for (int y = 0; y < dh; ++y)
			{
				int* I = (int*)(dst + drs * (dh - y - 1));
				int sy = (y * sh + dh / 2) / dh;
				if (sy >= sh) sy = sh - 1;

				const yuint8_t* srcrow = (const yuint8_t*)(src + sy * srs);

				int x = 0;
				for (; x < iLastFlag; ++x, I = (int*)((char*)I + 3))
				{
					const int* rgb = (int*)(srcrow + x_ofs[x]);
					*I = *rgb;
				}

				char* D = (char*)I;
				for (; x < dw; ++x, D += 3)
				{
					const yuint8_t* rgb = srcrow + x_ofs[x];
					D[0] = rgb[0];
					D[1] = rgb[1];
					D[2] = rgb[2];
				}
			}
		}
		else
		{
			for (int y = 0; y < dh; ++y)
			{
				int* I = (int*)(dst + drs * y);
				int sy = (y * sh + dh / 2) / dh;
				if (sy >= sh) sy = sh - 1;
				const yuint8_t* srcrow = (const yuint8_t*)(src + sy * srs);

				int x = 0;
				for (; x < iLastFlag; ++x, I = (int*)((char*)I + 3))
				{
					const int* rgb = (int*)(srcrow + x_ofs[x]);
					*I = *rgb;
				}

				char* D = (char*)I;
				for (; x < dw; ++x, D += 3)
				{
					const yuint8_t* rgb = srcrow + x_ofs[x];
					D[0] = rgb[0];
					D[1] = rgb[1];
					D[2] = rgb[2];
				}
			}
		}
	}

	static int round(double x)
	{
		return (int)floor(x + 0.5);
	}

	static void _SwapRGB(char* dst, int rs, int num_rows)
	{
		char* dst1 = dst, *dst2 = dst + rs * (num_rows - 1);
		int* p1 = (int*)dst1, *p2 = (int*)dst2;
		for (int i = 0; i < num_rows / 2; ++i)
		{
			for (int j = 0; j < rs / 4; ++j)
			{
				int x = *p1;
				*p1 = *p2;
				*p2 = x;

				++p1;
				++p2;
			}

			p2 -= rs / 2;
		}
	}
}

namespace _YXV_AVUtils
{
	int gs_resize_flag = _InitResizeFlags();
}

using _YXV_AVUtils::gs_resize_flag;

extern "C"
{
	void YXV_YUVImageResize(char* dst, const char* src, int sw, int sh, int dw, int dh, int interpolation,BOOL bInvert)
	{
		int flag = gs_resize_flag;
		if (interpolation == YXV_IMAGE_INTER_NN) flag = SWS_POINT;
		else if (interpolation == YXV_IMAGE_INTER_LINEAR) flag = SWS_BILINEAR;

	//	BOOL bInvert = TRUE;

		if (sw == dw && sh == dh) flag = SWS_POINT;

		SwsContext* pContext = sws_getContext(sw, sh, AV_PIX_FMT_YUV420P, dw, dh, AV_PIX_FMT_YUV420P, flag, NULL, NULL, 0);
		if (pContext != NULL)
		{
			/* V and U is inverted. */
			BYTE* srcPtr[] = { (BYTE*)src, (BYTE*)src + sw * sh * 5 / 4, (BYTE*)src + sw * sh };
			int srcStride[] = { sw, sw / 2, sw / 2 };
			BYTE* dstPtr[] = { (BYTE*)dst, (BYTE*)dst + dw * dh * 5 / 4, (BYTE*)dst + dw * dh };
			int dstStride[] = { dw, dw / 2, dw / 2 };
			if (bInvert)
			{
				for (int i = 0; i < 3; ++i)
				{
					int h1 = i == 0 ? sh - 1 : sh / 2 - 1;
					srcPtr[i] = (BYTE*)srcPtr[i] + srcStride[i] * h1;
					srcStride[i] = -srcStride[i];
				}
			}
			sws_scale(pContext, srcPtr, srcStride, 0, sh, dstPtr, dstStride);
			sws_freeContext(pContext);
		}
	}

	void YXV_RGB24ImageResize(char* dst, const char* src, int sw, int sh, int dw, int dh, int drs, int nAlign, int interpolation, BOOL bInvert)
	{
		int flag = gs_resize_flag;
		if (interpolation == YXV_IMAGE_INTER_LINEAR) flag = SWS_BILINEAR;
		if (interpolation == YXV_IMAGE_INTER_NN) flag = SWS_POINT;

		if (sw == dw && sh == dh) flag = SWS_POINT;
		SwsContext* pContext = sws_getContext(sw, sh, AV_PIX_FMT_BGR24, dw, dh, AV_PIX_FMT_BGR24, flag, NULL, NULL, NULL);
		if (pContext != NULL)
		{
			BYTE* srcPtr[] = { (BYTE*)src };
			int srcStride[] = { YXV_ImageAlignRow(sw * 3) };
			BYTE* dstPtr[] = { (BYTE*)dst };
			int dstStride[] = { drs };

			if (bInvert)
			{
				srcPtr[0] += srcStride[0] * (sh - 1);
				srcStride[0] = -srcStride[0];
			}
			//if (bInvert)
			//{
			//	dstStride[0] = -drs;
			//	dstPtr[0] = (BYTE*)dst + drs * (dh - 1);
			//}
			sws_scale(pContext, srcPtr, srcStride, 0, sh, dstPtr, dstStride);
			if (bInvert)
			{
				//_SwapRGB(dst, drs, dh);
			}

			//BYTE* ptr2 = (BYTE*)_alloca(srcStride[0] * 2); /* More space, work around ffwrap bug. */
			//memcpy(ptr2, src + srcStride[0] * (sh - 1), srcStride[0]);

			//srcPtr[0] = ptr2;
			//if (bInvert)
			//{
			//	dstPtr[0] = (BYTE*)dst;
			//}
			//else
			//{
			//	dstPtr[0] = (BYTE*)dst + drs * (dh - 1);
			//}
			//sws_scale(pContext, srcPtr, srcStride, 0, 1, dstPtr, dstStride);

			sws_freeContext(pContext);
		}


		//IplImage imgSrc = {0};
		//imgSrc.align = nAlign;
		//imgSrc.depth = IPL_DEPTH_8U;
		//imgSrc.height = sh;
		//imgSrc.width = sw;
		//imgSrc.widthStep = YXV_ImageAlignRow(sw * 3, nAlign);
		//imgSrc.imageData = (char*)src;
		//imgSrc.imageSize = imgSrc.widthStep * imgSrc.height;
		//imgSrc.nChannels = 3;
		//imgSrc.origin = 1;
		//imgSrc.nSize = sizeof(IplImage);

		//IplImage imgDst = {0};
		//imgDst.align = nAlign;
		//imgDst.depth = IPL_DEPTH_8U;
	}
	void YXV_RGB24ResizeNN2YUV(char* dst, const char* src, int sw, int sh, int dw, int dh, int drs, int nAlign, int interpolation, BOOL bInvert)
	{
		int flag = gs_resize_flag;
		if (interpolation == YXV_IMAGE_INTER_NN) flag = SWS_POINT;
		else if (interpolation == YXV_IMAGE_INTER_LINEAR) flag = SWS_BILINEAR;

		if (sw == dw && sh == dh) flag = SWS_POINT;
		SwsContext* pContext = sws_getContext(sw, sh, AV_PIX_FMT_BGR24, dw, dh, AV_PIX_FMT_YUV420P, flag, NULL, NULL, 0);

		if (pContext != NULL)
		{
			/* V and U is inverted. */
			BYTE* dstPtr[] = { (BYTE*)dst, (BYTE*)dst + dw * dh, (BYTE*)dst + dw * dh * 5 / 4 };
			int dstStride[] = { dw, dw / 2, dw / 2 };
			BYTE* srcPtr[] = { (BYTE*)src };
			int srcStride[] = { YXV_ImageAlignRow(sw * 3) };
			if (bInvert)
			{
				srcPtr[0] = srcPtr[0] + srcStride[0] * (sh - 1);
				srcStride[0] = -srcStride[0];
			}
			sws_scale(pContext, srcPtr, srcStride, 0, sh, dstPtr, dstStride);
			sws_freeContext(pContext);
		}
		//imgDst.height = dh;
		//imgDst.width = dw;
		//imgDst.widthStep = drs;
		//imgDst.imageData = (char*)dst;
		//imgDst.imageSize = drs * dw;
		//imgDst.nChannels = 3;
		//imgDst.origin = bInvert ? 0 : 1;
		//imgDst.nSize = sizeof(IplImage);

		//cvResize(&imgSrc, &imgDst, CV_INTER_LINEAR);
	}

	void YXV_YUVResizeNN2RGB24_Self(char* dst, const char* src, int sw, int sh, int dw, int dh,
		int drs)
	{
		int iXOffBuf[2048], iXOffBuf2[2048]; // max support 2048 width;

		if (dw <= 0 || dh <= 0) return;
		int* x_ofs = iXOffBuf;
		int* x_ofs2 = iXOffBuf2;

		double ifx = 1.0 * sw / dw, ify = 1.0 * sh / dh;

		for (int x = 0; x < dw; ++x)
		{
			x_ofs[x] = round(x * ifx);
			x_ofs2[x] = round(x * ifx / 2);

			if (x_ofs[x] >= sw) x_ofs[x] = sw - 1;
			if (x_ofs2[x] >= sw / 2) x_ofs2[x] = sw / 2 - 1;
		}

		const yuint8_t* pcy = (const yuint8_t*)src;
		const yuint8_t* pcv = pcy + sw * sh;
		const yuint8_t* pcu = pcv + (sw / 2) * (sh / 2);

		for (int y = 0; y < dh; ++y)
		{
			int* I = (int*)(dst + drs * y);
			int sy = round(y * ify);
			int sy2 = round(y * ify / 2);

			if (sy >= sh) sy = sh - 1;
			if (sy2 >= sh / 2) sy2 = sh / 2 - 1;
			const yuint8_t* psy = pcy + sy * sw;
			const yuint8_t* psu = pcu + sy2 * (sw / 2);
			const yuint8_t* psv = pcv + sy2 * (sw / 2);

			int x = 0;
			for (; x < dw - 1; ++x, I = (int*)((char*)I + 3))
			{
				const yuint8_t* py = psy + x_ofs[x];
				const yuint8_t* pu = psu + x_ofs2[x];
				const yuint8_t* pv = psv + x_ofs2[x];

				int iRGBVal = gs_yuv_to_rgb_table[*pv][*pu][*py];
				*I = iRGBVal;
			}
			// fill last column
			const yuint8_t* py = psy + x_ofs[x];
			const yuint8_t* pu = psu + x_ofs2[x];
			const yuint8_t* pv = psv + x_ofs2[x];
			int iRGBVal = gs_yuv_to_rgb_table[*pv][*pu][*py];
			char* D = (char*)I;
			char* pVal = (char*)&iRGBVal;
			D[0] = pVal[0];
			D[1] = pVal[1];
			D[2] = pVal[2];
		}
	}

	void YXV_YUVResizeNN2RGB24(char* dst, const char* src, int sw, int sh, int dw, int dh, int drs, int nAlign, int interpolation, BOOL bInvert)
	{
		int flag = gs_resize_flag;
		if (interpolation == YXV_IMAGE_INTER_NN) flag = SWS_POINT;
		else if (interpolation == YXV_IMAGE_INTER_LINEAR) flag = SWS_BILINEAR;

		if (sw == dw && sh == dh) flag = SWS_POINT;
		//if (flag == SWS_POINT && !bInvert)
		//{
		//	YXV_YUVResizeNN2RGB24_Self(dst, src, sw, sh, dw, dh, drs);
		//	return;
		//}

		SwsContext* pContext = sws_getContext(sw, sh, AV_PIX_FMT_YUV420P, dw, dh, AV_PIX_FMT_BGR24, flag, NULL, NULL, 0);
		if (pContext != NULL)
		{
			/* V and U is inverted. */
			BYTE* srcPtr[] = { (BYTE*)src, (BYTE*)src + sw * sh, (BYTE*)src + sw * sh * 5 / 4 };
			int srcStride[] = { sw, sw / 2, sw / 2 };
			BYTE* dstPtr[] = { (BYTE*)dst };
			int dstStride[] = { drs };
			if (bInvert)
			{
				srcPtr[0] += srcStride[0] * (sh - 1);
				srcPtr[1] += srcStride[1] * (sh / 2 - 1);
				srcPtr[2] += srcStride[2] * (sh / 2 - 1);

				srcStride[0] = -srcStride[0];
				srcStride[1] = -srcStride[1];
				srcStride[2] = -srcStride[2];
			}
			sws_scale(pContext, srcPtr, srcStride, 0, sh, dstPtr, dstStride);
			if (bInvert)
			{
				//_SwapRGB(dst, drs, dh);
			}
			sws_freeContext(pContext);
		}
	}

	void YXV_YUVResizeNN2RGB24_Step(char* dst, const char* src[3], int stride[3], int sw, int sh, int dw, int dh, int drs,
		int nAlign, int interpolation, BOOL bInvert)
	{
		int flag = gs_resize_flag;
		if (interpolation == YXV_IMAGE_INTER_NN) flag = SWS_POINT;
		else if (interpolation == YXV_IMAGE_INTER_LINEAR) flag = SWS_BILINEAR;

		if (sw == dw && sh == dh) flag = SWS_POINT;
		SwsContext* pContext = sws_getContext(sw, sh, AV_PIX_FMT_YUV420P, dw, dh, AV_PIX_FMT_BGR24, flag, NULL, NULL, 0);

		if (pContext != NULL)
		{
			/* V and U is inverted. */
			BYTE* srcPtr[] = { (BYTE*)src[0], (BYTE*)src[1], (BYTE*)src[2] };
			int srcStride[] = { stride[0], stride[1], stride[2] };
			BYTE* dstPtr[] = { (BYTE*)dst };
			int dstStride[] = { drs };

			if (bInvert)
			{
				srcPtr[0] += srcStride[0] * (sh - 1);
				srcPtr[1] += srcStride[1] * (sh / 2 - 1);
				srcPtr[2] += srcStride[2] * (sh / 2 - 1);

				srcStride[0] = -srcStride[0];
				srcStride[1] = -srcStride[1];
				srcStride[2] = -srcStride[2];
			}
			sws_scale(pContext, srcPtr, srcStride, 0, sh, dstPtr, dstStride);
			if (bInvert)
			{
				//_SwapRGB(dst, drs, dh);
			}
			sws_freeContext(pContext);
		}
	}

	void YXV_NV12ResizeNN2RGB32(char* dst, const char* src, int sw, int sh, int dw, int dh, int drs, int nAlign, int interpolation, BOOL bInvert)
	{
		int flag = gs_resize_flag;
		if (interpolation == YXV_IMAGE_INTER_NN) flag = SWS_POINT;
		else if (interpolation == YXV_IMAGE_INTER_LINEAR) flag = SWS_BILINEAR;

		if (sw == dw && sh == dh) flag = SWS_POINT;
		SwsContext* pContext = sws_getContext(sw, sh, AV_PIX_FMT_NV12, dw, dh, AV_PIX_FMT_RGB32, flag, NULL, NULL, 0);

		if (pContext != NULL)
		{
			/* V and U is inverted. */
			BYTE* srcPtr[] = { (BYTE*)src, (BYTE*)src + sw * sh };
			int srcStride[] = { sw, sw };
			BYTE* dstPtr[] = { (BYTE*)dst };
			int dstStride[] = { drs, 0, 0, 0 };
			if (bInvert)
			{
				srcPtr[0] += sw * (sh - 1);
				srcPtr[1] += sw * (sh / 2 - 1);

				srcStride[0] = -srcStride[0];
				srcStride[1] = -srcStride[1];
			//	dstStride[0] = -drs;
			//	dstPtr[0] = (BYTE*)dst + drs * (dh - 1);
			}
			sws_scale(pContext, srcPtr, srcStride, 0, sh, dstPtr, dstStride);
			sws_freeContext(pContext);

			//if (bInvert)
			//{
			//	_SwapRGB(dst, drs, dh);
			//}
		}
	}

	void YXV_YUV2RGB24(char* dst, const char* src, int sw, int sh, BOOL bInvert)
	{
		_YUV420P_To_RGB(dst, (char*)src, sw, sh, bInvert);
	}

	void YXV_YUYV2RGB24(char* dst, const char* src, int sw, int sh, BOOL bInvert)
	{
		SwsContext* sws = sws_getContext(sw, sh, AV_PIX_FMT_UYVY422, sw, sh, AV_PIX_FMT_BGR24,
			SWS_POINT, NULL, NULL, NULL);

		if (sws != NULL)
		{
			int sliceSrc[4] = { sw * 2 };
			ybyte_t* pSrc[4] = { (ybyte_t*)src };

			if (bInvert)
			{
				sliceSrc[0] = -sliceSrc[0];
				pSrc[0] = (ybyte_t*)src + (sh - 1) * sw * 2;
			}

			ybyte_t* pDst[4] = { (ybyte_t*)dst };
			int sliceDst[4] = { sw * 3 };

			sws_scale(sws, pSrc, sliceSrc, 0, sh, pDst, sliceDst);
			sws_freeContext(sws);
		}
	}

	void YXV_RGB242YUV(char* dst, const char* src, int sw, int sh, BOOL bInvert)
	{
		SwsContext* sws = sws_getContext(sw, sh, AV_PIX_FMT_BGR24, sw, sh, AV_PIX_FMT_YUV420P,
			SWS_POINT, NULL, NULL, NULL);

		if (sws != NULL)
		{
			int sliceSrc[4] = { (sw * 3 + 3) / 4 * 4 };
			ybyte_t* pSrc[4] = { (ybyte_t*)src };

			if (bInvert)
			{
				pSrc[0] = (ybyte_t*)src + (sh - 1) * sliceSrc[0];
				sliceSrc[0] = -sliceSrc[0];
			}

			BYTE* pDst[] = { (BYTE*)dst, (BYTE*)dst + sw * sh, (BYTE*)dst + sw * sh * 5 / 4 };
			int sliceDst[4] = { sw, sw / 2, sw / 2 };

			sws_scale(sws, pSrc, sliceSrc, 0, sh, pDst, sliceDst);
			sws_freeContext(sws);
		}
	}
}

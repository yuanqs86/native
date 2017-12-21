#ifndef H264Decoder_DllHeader_h__
#define H264Decoder_DllHeader_h__

#include <YXC_Sys/YXC_Sys.h>

enum YXC_PixelFormat {
	YXC_PIX_FMT_NONE = -1,
	YXC_PIX_FMT_YUV420P,
	YXC_PIX_FMT_YUYV422,
	YXC_PIX_FMT_RGB24
};

typedef struct __YXC_AVRational__{
	int num; ///< numerator
	int den; ///< denominator
} YXC_AVRational;

#define YXC_QP_MAX_NUM (51 + 6*6)           // The maximum supported qp

enum YXC_AVColorPrimaries{
	YXC_AVCOL_PRI_BT709       = 1, ///< also ITU-R BT1361 / IEC 61966-2-4 / SMPTE RP177 Annex B
	YXC_AVCOL_PRI_UNSPECIFIED = 2,
	YXC_AVCOL_PRI_BT470M      = 4,
	YXC_AVCOL_PRI_BT470BG     = 5, ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM
	YXC_AVCOL_PRI_SMPTE170M   = 6, ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
	YXC_AVCOL_PRI_SMPTE240M   = 7, ///< functionally identical to above
	YXC_AVCOL_PRI_FILM        = 8,
	YXC_AVCOL_PRI_NB             , ///< Not part of ABI
};

enum YXC_AVColorTransferCharacteristic{
	YXC_AVCOL_TRC_BT709       = 1, ///< also ITU-R BT1361
	YXC_AVCOL_TRC_UNSPECIFIED = 2,
	YXC_AVCOL_TRC_GAMMA22     = 4, ///< also ITU-R BT470M / ITU-R BT1700 625 PAL & SECAM
	YXC_AVCOL_TRC_GAMMA28     = 5, ///< also ITU-R BT470BG
	YXC_AVCOL_TRC_SMPTE240M   = 7,
	YXC_AVCOL_TRC_NB             , ///< Not part of ABI
};

enum YXC_AVColorSpace{
	YXC_AVCOL_SPC_RGB         = 0,
	YXC_AVCOL_SPC_BT709       = 1, ///< also ITU-R BT1361 / IEC 61966-2-4 xvYCC709 / SMPTE RP177 Annex B
	YXC_AVCOL_SPC_UNSPECIFIED = 2,
	YXC_AVCOL_SPC_FCC         = 4,
	YXC_AVCOL_SPC_BT470BG     = 5, ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601
	YXC_AVCOL_SPC_SMPTE170M   = 6, ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC / functionally identical to above
	YXC_AVCOL_SPC_SMPTE240M   = 7,
	YXC_AVCOL_SPC_YCOCG       = 8, ///< Used by Dirac / VC-2 and H.264 FRext, see ITU-T SG16
	YXC_AVCOL_SPC_NB             , ///< Not part of ABI
};

typedef struct __YXC_SPS__ {
    int profile_idc;
    int level_idc;
    int chroma_format_idc;
    int transform_bypass;              ///< qpprime_y_zero_transform_bypass_flag
    int log2_max_frame_num;            ///< log2_max_frame_num_minus4 + 4
    int poc_type;                      ///< pic_order_cnt_type
    int log2_max_poc_lsb;              ///< log2_max_pic_order_cnt_lsb_minus4
    int delta_pic_order_always_zero_flag;
    int offset_for_non_ref_pic;
    int offset_for_top_to_bottom_field;
    int poc_cycle_length;              ///< num_ref_frames_in_pic_order_cnt_cycle
    int ref_frame_count;               ///< num_ref_frames
    int gaps_in_frame_num_allowed_flag;
    int mb_width;                      ///< pic_width_in_mbs_minus1 + 1
    int mb_height;                     ///< pic_height_in_map_units_minus1 + 1
	int mb_real_width;
	int mb_real_height;
    int frame_mbs_only_flag;
    int mb_aff;                        ///< mb_adaptive_frame_field_flag
    int direct_8x8_inference_flag;
    int crop;                          ///< frame_cropping_flag
    unsigned int crop_left;            ///< frame_cropping_rect_left_offset
    unsigned int crop_right;           ///< frame_cropping_rect_right_offset
    unsigned int crop_top;             ///< frame_cropping_rect_top_offset
    unsigned int crop_bottom;          ///< frame_cropping_rect_bottom_offset
    int vui_parameters_present_flag;
    YXC_AVRational sar; //Liz modified
    int video_signal_type_present_flag;
    int full_range;
    int colour_description_present_flag;
    enum YXC_AVColorPrimaries color_primaries;
    enum YXC_AVColorTransferCharacteristic color_trc;
    enum YXC_AVColorSpace colorspace;
    int timing_info_present_flag;
    yuint32_t num_units_in_tick;
    yuint32_t time_scale;
    int fixed_frame_rate_flag;
    short offset_for_ref_frame[256]; // FIXME dyn aloc?
    int bitstream_restriction_flag;
    int num_reorder_frames;
    int scaling_matrix_present;
    yuint8_t scaling_matrix4[6][16];
    yuint8_t scaling_matrix8[6][64];
    int nal_hrd_parameters_present_flag;
    int vcl_hrd_parameters_present_flag;
    int pic_struct_present_flag;
    int time_offset_length;
    int cpb_cnt;                          ///< See H.264 E.1.2
    int initial_cpb_removal_delay_length; ///< initial_cpb_removal_delay_length_minus1 + 1
    int cpb_removal_delay_length;         ///< cpb_removal_delay_length_minus1 + 1
    int dpb_output_delay_length;          ///< dpb_output_delay_length_minus1 + 1
    int bit_depth_luma;                   ///< bit_depth_luma_minus8 + 8
    int bit_depth_chroma;                 ///< bit_depth_chroma_minus8 + 8
    int residual_color_transform_flag;    ///< residual_colour_transform_flag
    int constraint_set_flags;             ///< constraint_set[0-3]_flag
} YXC_SPS;

/**
 * Picture parameter set
 */

typedef struct __YXC_PPS__ {
	unsigned int sps_id;
	int cabac;                  ///< entropy_coding_mode_flag
	int pic_order_present;      ///< pic_order_present_flag
	int slice_group_count;      ///< num_slice_groups_minus1 + 1
	int mb_slice_group_map_type;
	unsigned int ref_count[2];  ///< num_ref_idx_l0/1_active_minus1 + 1
	int weighted_pred;          ///< weighted_pred_flag
	int weighted_bipred_idc;
	int init_qp;                ///< pic_init_qp_minus26 + 26
	int init_qs;                ///< pic_init_qs_minus26 + 26
	int chroma_qp_index_offset[2];
	int deblocking_filter_parameters_present; ///< deblocking_filter_parameters_present_flag
	int constrained_intra_pred;     ///< constrained_intra_pred_flag
	int redundant_pic_cnt_present;  ///< redundant_pic_cnt_present_flag
	int transform_8x8_mode;         ///< transform_8x8_mode_flag
	yuint8_t scaling_matrix4[6][16];
	yuint8_t scaling_matrix8[6][64];
	yuint8_t chroma_qp_table[2][64]; ///< pre-scaled (with chroma_qp_index_offset) version of qp_table
	int chroma_qp_diff;
	// ==> Start patch MPC
	int slice_group_change_direction_flag;
	int slice_group_change_rate_minus1;
	// <== End patch MPC
} YXC_PPS;

typedef int (*DecoderCallback)(HANDLE hDecoder, LPBYTE pData, ULONG32 uDataSize, ULONG uImgWidth, ULONG uImgHeight, enum YXC_PixelFormat pix_fmt, LONG lUserParam);

#ifdef __cplusplus
extern "C" {
#endif

	//YXC_API LONG OpenH264Decoder(LONG lUserParam, LPHANDLE pHandle);/*uUserParam - 输入，用户自定义参数；pHandle - 输出，decoder句柄用以作为标示符*/
	//YXC_API LONG SetCallback(HANDLE hDecoder, DecoderCallback pDecoderCallbackFunc);
	//YXC_API LONG SetH264Header(HANDLE hDecoder, LPBYTE pSPS, ULONG32 uSPS, LPBYTE pPPS, ULONG32 uPPS);
	//YXC_API LONG DecodeH264Frame(HANDLE hDecoder, LPBYTE pData, ULONG32 uDataSize);
	//YXC_API LONG CloseH264Decoder(HANDLE hDecoder);

	YXC_API(LONG) DecodeH264SPS(LPBYTE pData/*in*/, ULONG32 uDataSize/*in*/, YXC_SPS* pEJ_SPS/*in, out*/);//h264 sps 解析函数
	//YXC_API LONG DecodeH264PPS(LPBYTE pData/*in*/, ULONG32 uDataSize/*in*/, YXC_PPS* pEJ_PPS/*in, out*/);//h264 pps 解析函数
	YXC_API(LONG) ParseH264Ext(LPBYTE pData, ULONG32 uDataSize, LPBYTE* ppSps, LPBYTE* ppPps, ULONG* pSpsSize, ULONG* pPpsSize);

#ifdef __cplusplus
}

#endif

#endif // DllHeader_h__





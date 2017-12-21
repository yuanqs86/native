#define FAACAPI
extern "C"
{
	#include <faac/faac.h>
	#include <libavcodec/avcodec.h>
}

struct FaacAudioContext
{
	faacEncHandle faac_handle;
	void* ex;
};

namespace YXV_FFWrap
{
	int Faac_encode_frame(AVCodecContext *avctx, AVPacket *avpkt,
		const AVFrame *frame, int *got_packet_ptr)
	{
		FaacAudioContext *s = (FaacAudioContext*)avctx->priv_data;
		int bytes_written, ret;
		int num_samples  = frame ? frame->nb_samples : 0;
		void *samples    = frame ? frame->data[0]    : NULL;
		int buf = avpkt->size;

		if (avpkt->data != NULL)
		{
			if (avpkt->size < (7 + 768) * avctx->channels)
			{
				return -EINVAL;
			}
		}
		else
		{
			avpkt->data = (uint8_t*)av_malloc((7 + 768) * avctx->channels);
			if (avpkt->data == NULL)
			{
				return -ENOMEM;
			}
			buf = (7 + 768) * avctx->channels;
		}

		bytes_written = faacEncEncode(s->faac_handle, (int32_t*)samples,
			num_samples * avctx->channels,
			avpkt->data, buf);
		if (bytes_written < 0) {
			// av_log(avctx, AV_LOG_ERROR, "faacEncEncode() error\n");
			return bytes_written;
		}

		if (!bytes_written)
		{
			*got_packet_ptr = 0;
			return 0;
		}

		avpkt->size = bytes_written;
		*got_packet_ptr = 1;
		return 0;
	}
}

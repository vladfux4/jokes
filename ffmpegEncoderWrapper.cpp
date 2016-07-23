extern "C" {
	#include <libavcodec/avcodec.h>
	#include "libavdevice/avdevice.h"
	#include "libavutil/imgutils.h"
	#include <libswscale/swscale.h>
}

#include <iostream>
#include <exception>

class Encoder
{
private:
	AVCodecID codecId;
	AVCodec *codec;
	AVCodecContext *codecContext;

	AVOutputFormat * fileFormat;
	AVFormatContext* formatContext;
	AVIOContext* fileIOContext;

	AVFrame *frame;
	AVStream* stream;

	struct SwsContext* convertContext;

	uint8_t fps;
	unsigned int ptsCounter, width, height;
	int ret;
	const char * filename;
	const char * formatString;

	bool convertPixel;
	AVPixelFormat converFormat;

	void initEnv()
	{
		av_register_all();
		avcodec_register_all();

		/* find the mpeg1 video encoder */
		if (!(codec = avcodec_find_encoder(codecId))) {
			throw std::invalid_argument("Codec not found");
		}

		if (!(fileFormat = av_guess_format(formatString, filename, nullptr))) {
			throw std::invalid_argument("Could not find format");
		}

		if ((avio_open(&fileIOContext, filename, AVIO_FLAG_WRITE)) < 0) {
			throw std::invalid_argument("Could not open file");
		}
	}

	void initFormatContext()
	{
		if (!(formatContext = avformat_alloc_context())) {
			avformat_free_context(formatContext);
			throw std::runtime_error("Could not allocate file context");
		}

		formatContext->pb = fileIOContext;
		formatContext->oformat = fileFormat;

		strcpy(formatContext->filename, filename);

		/*
		* Some container formats (like MP4) require global headers to be present
		* Mark the encoder so that it behaves accordingly.
		*/
		if (formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
			formatContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
	}

	void initStream()
	{
		stream = avformat_new_stream(formatContext, codec);
		if (!stream) {
			throw std::runtime_error("Could not create new stream");
		}
	}

	void initCodecContext()
	{
		codecContext = stream->codec;
		/* put sample parameters */
		codecContext->bit_rate = width * height;
		/* resolution must be a multiple of two */
		codecContext->width = width;
		codecContext->height = height;
		codecContext->gop_size = 10; /* emit one intra frame every ten frames */
		codecContext->max_b_frames = 1;
		codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
		codecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
		/* frames per second */
		codecContext->time_base.num = 1;
		codecContext->time_base.den = fps;

		/* open it */
		if (avcodec_open2(codecContext, codec, NULL) < 0) {
			throw std::runtime_error("Could not open codec");
		}
	}

	void initConvertContext()
	{
		convertContext = sws_getContext(codecContext->width, codecContext->height, converFormat, codecContext->width, codecContext->height, codecContext->pix_fmt, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}

	void initFrame()
	{
		frame = av_frame_alloc();
		if (!frame) {
			throw std::runtime_error("Could not allocate video frame");
		}
		frame->format = codecContext->pix_fmt;
		frame->width = codecContext->width;
		frame->height = codecContext->height;
		frame->pts = 0;
	}

	void initFrameData()
	{
		/* the image can be allocated by any means and av_image_alloc() is
		* just the most convenient way if av_malloc() is to be used */
		ret = av_image_alloc(frame->data, frame->linesize, codecContext->width, codecContext->height,
			codecContext->pix_fmt, 32);
		if (ret < 0) {
			throw std::runtime_error("Could not allocate raw picture buffer");
		}
	}


public:
	Encoder(const AVCodecID codecId, const char * formatString, const char * filename, const unsigned int width, const unsigned int height, const uint8_t fps,
		const bool convertPixel = false, AVPixelFormat converFormat = AV_PIX_FMT_BGRA)
		: codecId(codecId),
		formatString(formatString),
		filename(filename),
		width(width),
		height(height),
		fps(fps),
		convertPixel(convertPixel),
		converFormat(AV_PIX_FMT_BGRA)
		
	{
		ptsCounter = 0;

		initEnv();
		initFormatContext();
		initStream();
		initCodecContext();
		initFrame();
		initFrameData();

		if(convertPixel)
			initConvertContext();

		if ((avformat_write_header(formatContext, nullptr)) < 0) {
			throw std::runtime_error("Could not write output file header");
		}
	}

	~Encoder()
	{
		av_write_trailer(formatContext);

		avcodec_close(codecContext);
		av_frame_free(&frame);

		avio_close(formatContext->pb);
		avformat_free_context(formatContext);
	}

	void writeFrame(uint8_t * data)
	{
		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = nullptr;    // packet data will be allocated by the encoder
		pkt.size = 0;
		pkt.pts = 0;

		frame->pts = av_rescale_q(ptsCounter, codecContext->time_base, stream->time_base);

		if (convertPixel)
		{
			int inLinesize[1] = { 4 * frame->width }; // RGB stride
			uint8_t * inData[1] = { data }; // RGB24 have one plane
			sws_scale(convertContext, inData, inLinesize, 0, frame->height, frame->data, frame->linesize);
		}
		else
		{
			avpicture_fill((AVPicture *)frame, data, codecContext->pix_fmt, width, height);
		}

		ret = avcodec_send_frame(codecContext, frame);
		if (ret < 0) {
			throw std::runtime_error("Error encoding frame");
		}

		/* read all packets in frame */
		while ((ret = avcodec_receive_packet(codecContext, &pkt)) >= 0) {
			if ((ret = av_write_frame(formatContext, &pkt)) < 0) {
				throw std::runtime_error("Error encoding frame");
			}
		}

		av_packet_unref(&pkt);
		ptsCounter++;
	}
};

int main()
{
	unsigned int width = 512;
	unsigned int height = 512;
	unsigned int fps = 25;


	uint8_t * data = (uint8_t*)malloc(width * height * 4);
	for (size_t i = 0; i < width * height * 4; i++)
	{
		*(data + i) = 100;
	}

	Encoder * encoder = nullptr;
	try
	{
		encoder = new Encoder(AV_CODEC_ID_MPEG2VIDEO, "mp4", "out.mp4", width, height, fps, true);
		for (size_t i = 0; i < fps * 3; i++) //write 3 seconds
		{
			encoder->writeFrame(data);
		}
	}
	catch (std::exception e)
	{
		std::cerr << e.what() << std::endl;
	}
	if (encoder)
		delete encoder;	

	delete data;
	return 0;
}
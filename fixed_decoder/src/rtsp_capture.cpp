#include "ros/ros.h"
#include "fixed_decoder/rtsp_capture.hpp"
#include <csignal>

#define __app_name__ "rtsp_capture_node"
extern "C"
{
    #include "libavdevice/avdevice.h"
    #include "libavutil/pixfmt.h"
    #include "libswscale/swscale.h"
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/time.h"
    #include "libswresample/swresample.h"
}

bool isRunning;
void signalHandler(int signum)
{ 
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    isRunning = false;
    // 清理并关闭  
    exit(signum);  
}


typedef struct {
    time_t lasttime;
} Runner;

int interrupt_callback(void *p) {
    Runner *r = (Runner *)p;
    if (r->lasttime > 0) {
		time_t offset = time(NULL) - r->lasttime;
        if (offset > 10) {
			printf("offset:%i \n",offset);
			isRunning = false;
            // 等待超过10s则中断
            return 1;
        }
    }
    return 0;
}

void *read_h264(void *args)
{
    rtsp_capture *_this = (rtsp_capture*)args;
	//URL
	const char *filepath = _this->rtsp_str.c_str();
	av_register_all();
	avformat_network_init();
    AVFormatContext *pFormatCtx = NULL;
	pFormatCtx = avformat_alloc_context();
	//set callback
	Runner input_runner = {0};
	pFormatCtx->interrupt_callback.opaque = &input_runner;
	pFormatCtx->interrupt_callback.callback = interrupt_callback;
	input_runner.lasttime = time(NULL);
	//set option
	AVDictionary *format_opts = NULL;
	av_dict_set(&format_opts, "stimeout", std::to_string(30 * 1000000).c_str(), 0); //设置链接超时时间（us）
	av_dict_set(&format_opts, "rtsp_transport",  "tcp", 0); //设置推流的方式，默认udp
	if (avformat_open_input(&pFormatCtx, filepath, NULL, &format_opts) != 0)  
	{
		printf("Couldn't open input stream.\n");
		isRunning = false;
        return NULL;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL)<0)
	{
		printf("Couldn't find stream information.\n");
		isRunning = false;
        return NULL;
	}
	int videoindex = -1;
	for (int i = 0; i<pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}
	if (videoindex == -1)
	{
		printf("didn't find a video stream.\n");
		isRunning = false;
        return NULL;
	}
	AVCodecContext *pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	/**/
	AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		printf("codec not found.\n");
		isRunning = false;
        return NULL;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL)<0)
	{
		printf("couldn't open codec.\n");
		isRunning = false;
		return NULL;
	}
	AVFrame *pFrame = av_frame_alloc();
	AVFrame *pFrameYUV = av_frame_alloc();
	uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
 
	//Output Info---输出一些文件（RTSP）信息  
	printf("---------------- File Information ---------------\n");
	av_dump_format(pFormatCtx, 0, filepath, 0);
	printf("-------------------------------------------------\n");
    struct SwsContext *img_convert_ctx = NULL;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
 
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    
	try
	{
		unsigned char* g_buffer = NULL;
    	unsigned int g_len = 0;
		int got_picture = 0;
		for (;;)
		{
			//printf("av_read_frame start \n");
			if (av_read_frame(pFormatCtx, packet) >= 0)
			{
				//printf("av_read_frame end \n");
				if (packet->stream_index == videoindex)
				{
                	//视频解码函数  解码之后的数据存储在 pFrame中
					g_buffer = (unsigned char*)packet->data;
                	g_len = packet->size;
				
					std::vector<unsigned char> vc(g_buffer, g_buffer+g_len);
					sensor_msgs::Image msg;
					msg.header.stamp = ros::Time::now();
                	msg.data = vc;
                	msg.step = g_len;
                	_this->image_pub.publish(msg);
					input_runner.lasttime = time(NULL);
				}
				av_free_packet(packet);
			}
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
}

rtsp_capture::rtsp_capture(){
    ros::NodeHandle nh_("~");

	nh_.param<std::string>("topic_name", topic_str, "/shd/rtsp/visible");
    nh_.param<std::string>("rtsp_url", rtsp_str, "rtsp://192.168.31.8:8000/165506");
	nh_.param<bool>("extradata", extra_data, false);
    image_pub = nh_.advertise<sensor_msgs::Image>(topic_str, 10);
	pthread_create(&tid,NULL,read_h264,(void*)this);
}

rtsp_capture::~rtsp_capture(){}
void rtsp_capture::update(){}


int main(int argc, char **argv)
{
	isRunning = true;
    signal(SIGINT, signalHandler);
    ros::init(argc, argv, __app_name__);
	rtsp_capture rtsp_capture_node;
    ros::Rate rate(25);
	std::cout << "start rtsp_capture_node" << std::endl;
    while (ros::ok() && isRunning)
    {
		rtsp_capture_node.update();
        ros::spinOnce();
        rate.sleep();
    }
	pthread_cancel(rtsp_capture_node.tid);//取消线程 
	void *ret = NULL;
    pthread_join(rtsp_capture_node.tid, &ret);
	std::cout << "rtsp thread exit" << std::endl;
    return 0;
}

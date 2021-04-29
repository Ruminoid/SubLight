#include "SubLight.Classic.h"

#define _r(c) ((c)>>24)
#define _g(c) (((c)>>16)&0xFF)
#define _b(c) (((c)>>8)&0xFF)
#define _a(c) ((c)&0xFF)

void BlendSingle(
	PF_Pixel* dst,
	int dst_stride,
	int w,
	int h,
	int color,
	unsigned char* src,
	int src_stride,
	int dst_x,
	int dst_y,
	int src_w,
	int src_h)
{
	if (!dst || !src) return;

	const auto opacity = 0xFF - static_cast<unsigned char>((_a(color)));
	const auto r = static_cast<unsigned char>((_r(color)));
	const auto g = static_cast<unsigned char>((_g(color)));
	const auto b = static_cast<unsigned char>((_b(color)));

	src_w = MIN(w - dst_x, src_w);
	src_h = MIN(h - dst_y, src_h);

	PF_Pixel* dstp = reinterpret_cast<PF_Pixel*>(
		reinterpret_cast<char*>(dst) +
		dst_y * dst_stride +
		dst_x * sizeof(PF_Pixel));
	if (!dstp) return;

	unsigned char* srcp = src;

	const int src_stride2 = src_stride - src_w;
	const int dst_stride2 = dst_stride / 4 - src_w;

	for (int y = 0; y < src_h; y++, dstp += dst_stride2, srcp += src_stride2)
	{
		for (int x = 0; x < src_w; x++, dstp++, srcp++)
		{
			const unsigned int k = static_cast<unsigned>(*srcp) * opacity / 0xFF;
			const unsigned int ck = 255 - k;

			dstp->red = (k * r + ck * dstp->red) / 255;
			dstp->green = (k * g + ck * dstp->green) / 255;
			dstp->blue = (k * b + ck * dstp->blue) / 255;
			dstp->alpha = 0xFF - ck * (0xFF - dstp->alpha) / 255;
		}
	}
}

/// @deparcated Use AE FillMatte Suite.
void CleanupWorld(

	PF_EffectWorld* output)
{

	for (int y = 0; y < output->height; y++)
	{
		for (int x = 0; x < output->width; x++)
		{
			PF_Pixel* pixel = reinterpret_cast<PF_Pixel*>(
				reinterpret_cast<char*>(output->data) +
				y * output->rowbytes +
				x * sizeof(PF_Pixel));

			pixel->red = 0;
			pixel->green = 0;
			pixel->blue = 0;
			pixel->alpha = 0;
		}
	}
}

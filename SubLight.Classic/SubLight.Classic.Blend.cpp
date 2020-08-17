#include "SubLight.Classic.h"

#define _r(c) ((c)>>24)
#define _g(c) (((c)>>16)&0xFF)
#define _b(c) (((c)>>8)&0xFF)
#define _a(c) ((c)&0xFF)

PF_Pixel* GetPixel(PF_EffectWorld* def, int x, int y)
{
	return reinterpret_cast<PF_Pixel*>(reinterpret_cast<char*>(def->data) + y * def->rowbytes + x * sizeof(PF_Pixel));
}

void BlendSingle(PF_EffectWorld* def, int dst_stride, int w, int h, int color, unsigned char* src, int src_stride, int dst_x, int dst_y, int src_w, int src_h)
{
	auto opacity = 0xFF - (unsigned char)(_a(color));
	auto r = (unsigned char)(_r(color));
	auto g = (unsigned char)(_g(color));
	auto b = (unsigned char)(_b(color));

	src_w = MIN(w - dst_x, src_w);
	src_h = MIN(h - dst_y, src_h);

	//unsigned int* dstp = reinterpret_cast<unsigned*>(dst) + size_t(dst_y) * (dst_stride >> 2) + dst_x;
	
	PF_Pixel* dstp = GetPixel(def, dst_x, dst_y);
	unsigned char* srcp = reinterpret_cast<unsigned char*>(src);

	int src_stride2 = src_stride - src_w;
	int dst_stride2 = (dst_stride >> 2) - src_w;

	int x = 0, y = 0;

	while (y < src_h)
	{
		while (x < src_w)
		{
			unsigned int k = unsigned(*srcp) * opacity / 0xFF;
			unsigned int ck = 255 - k;

			unsigned char finr = (k * r + ck * dstp->red) / 0xFF;
			unsigned char fing = (k * g + ck * dstp->green) / 0xFF;
			unsigned char finb = (k * b + ck * dstp->blue) / 0xFF;
			unsigned char fina = 0xFF - ck * (0xFF - dstp->alpha) / 0xFF;
			
			//*dstp = (((fina & 0xFF) << 24) + ((finb & 0xFF) << 16) + ((fing & 0xFF) << 8) + (finr & 0xFF));

			dstp->red = finr;
			dstp->green = fing;
			dstp->blue = finb;
			dstp->alpha = fina;

			x++;
			dstp++;
			srcp++;
		}

		y++;
		dstp += dst_stride2;
		srcp += src_stride2;
	}
}

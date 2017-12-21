#ifndef _CHROMA_KEY_H_
#define _CHROMA_KEY_H_

#define M_PI  3.1415926535898

typedef struct{
	unsigned char r,g,b;
}color;
typedef struct{
	float y;
	char cb,cr;
}colorY;
typedef struct{
	unsigned char y;
	short cb,cr;
}colorYM;

typedef struct{
	int width;
	int height;
	colorY key_colorY;
	colorYM key_colorYM;
	char kg;
	unsigned short kgM;
	float accept_angle_cos;
	float accept_angle_sin;
	float noise_level;
	unsigned char accept_angle_tg;
	unsigned char accept_angle_ctg;
	unsigned char one_over_kc;
	unsigned char kfgy_scale;
	unsigned short accept_angle_tgM;
	unsigned short accept_angle_ctgM;
	unsigned short one_over_kcM;
	unsigned short kfgy_scaleM;
	unsigned short noise_level_sqM;
}info;

void _utah_ck(unsigned char *,char *,char *,
			  unsigned char *,char *,char *,
			  unsigned char *,char *,char *, info *);

void _utah_ck2(unsigned char *,char *,char *,
			  unsigned char*,unsigned char*,unsigned char*,
			  unsigned char *,char *,char *,unsigned char *,info *);
#endif

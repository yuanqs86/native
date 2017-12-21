/* This contains MMX simulation function descriptors and software register
   definitions. The functions themself are from a CD coming with Intel's
   "The Complete Guide to MMX" book and I do not feel like puting them here.
   They are quite straightforward if one wants to write them.
*/
#ifndef TRUE
 #define TRUE 1
 #define FALSE 0
#endif
#ifndef BOOL
 typedef int BOOL;
#endif

#include <YXC_AVKernel/Utils/chroma_key.h>

#define MapAddressFlat(x,y,z)   TRUE

/* These are software regular registers added by me, but not really used
   except EAX (once).*/

#define EAX 0
#define ECX 1
#define EDX 2
#define EBX 3
#define ESP 4
#define EBP 5
#define ESI 6
#define EDI 7

#define R__EAX regs[EAX]
#define R__ECX regs[ECX]
#define R__EDX regs[EDX]
#define R__EBX regs[EBX]
#define R__ESP regs[ESP]
#define R__EBP regs[EBP]
#define R__ESI regs[ESI]
#define R__EDI regs[EDI]

/* MMX software register definitions start here */

#define MM0 mmregs[0]
#define MM1 mmregs[1]
#define MM2 mmregs[2]
#define MM3 mmregs[3]
#define MM4 mmregs[4]
#define MM5 mmregs[5]
#define MM6 mmregs[6]
#define MM7 mmregs[7]

typedef unsigned char  MMXU8;
typedef unsigned short MMXU16;
typedef unsigned long  MMXU32;
typedef signed   char  MMXS8;
typedef signed   short MMXS16;
typedef signed   long  MMXS32;

typedef struct {
 union {
  MMXU32 d[2];
  MMXU16 w[4];
  MMXU8  b[8];
  MMXS32 ds[2];
  MMXS16 ws[4];
  MMXS8  bs[8];
 } v;
 void *addr;
} MMX64;  /* type of 64 bit items */

 /* Globals - register arrays */
 /* Software MMX registers */

MMX64 *(mmregs[8]);

 /* Software integer registers. We don't bother to distinguish between
    EAX and AX, just use 32 bits */

unsigned regs[8];

 /* Again, not used */

MMX64 *mmdummy;

  /* MMX instruction emulation functions  */

void initregs();
void mov4shorts_ld_d(void *addr, MMX64 *dst);
void mov8bytes_ld(void *addr, MMX64 *dst);
void mov8bytes_st(void *addr, MMX64 *src);
void movd_st ( MMX64 *dst, MMX64 *src , int x86reg ,int dst_index);
void movd_ld ( MMX64 *dst, MMX64 *src , int x86reg ,int src_index);
void movq ( MMX64 *dst, MMX64 *src);

void packsswb(MMX64 *dst, MMX64 *src);
void packssdw(MMX64 *dst, MMX64 *src);
void packuswb(MMX64 *dst, MMX64 *src);
void paddb(MMX64 *dst, MMX64 *src);
void paddw(MMX64 *dst, MMX64 *src);
void paddd(MMX64 *dst, MMX64 *src);
void paddsb(MMX64 *dst, MMX64 *src);
void paddsw(MMX64 *dst, MMX64 *src);
void paddusb(MMX64 *dst, MMX64 *src);
void paddusw(MMX64 *dst, MMX64 *src);
void pand(MMX64 *dst, MMX64 *src);
void pandn(MMX64 *dst, MMX64 *src);
void pcmpeqb(MMX64 *dst, MMX64 *src);
void pcmpeqw(MMX64 *dst, MMX64 *src);
void pcmpeqd(MMX64 *dst, MMX64 *src);
void pcmpgtb(MMX64 *dst, MMX64 *src);
void pcmpgtw(MMX64 *dst, MMX64 *src);
void pcmpgtd(MMX64 *dst, MMX64 *src);
void pmaddwd(MMX64 *dst, MMX64 *src);
void pmulhw(MMX64 *dst, MMX64 *src);
void pmullw(MMX64 *dst, MMX64 *src);
void por(MMX64 *dst, MMX64 *src);
void psllw(MMX64 *dst, MMX64 *src);
void pslld(MMX64 *dst, MMX64 *src);
void psllq(MMX64 *dst, MMX64 *src);
void psllw_imm8(MMX64 *dst, MMXU8 imm8);
void pslld_imm8(MMX64 *dst, MMXU8 imm8);
void psllq_imm8(MMX64 *dst, MMXU8 imm8);
void psraw(MMX64 *dst, MMX64 *src);
void psrad(MMX64 *dst, MMX64 *src);
void psraw_imm8(MMX64 *dst, MMXU8 imm8);
void psrad_imm8(MMX64 *dst, MMXU8 imm8);
void psrlw(MMX64 *dst, MMX64 *src);
void psrld(MMX64 *dst, MMX64 *src);
void psrlq(MMX64 *dst, MMX64 *src);
void psrlw_imm8(MMX64 *dst, MMXU8 imm8);
void psrld_imm8(MMX64 *dst, MMXU8 imm8);
void psrlq_imm8(MMX64 *dst, MMXU8 imm8);
void psubb(MMX64 *dst, MMX64 *src);
void psubw(MMX64 *dst, MMX64 *src);
void psubd(MMX64 *dst, MMX64 *src);
void psubsb(MMX64 *dst, MMX64 *src);
void psubsw(MMX64 *dst, MMX64 *src);
void psubusb(MMX64 *dst, MMX64 *src);
void psubusw(MMX64 *dst, MMX64 *src);
void punpcklbw(MMX64 *dst, MMX64 *src);
void punpcklwd(MMX64 *dst, MMX64 *src);
void punpckldq(MMX64 *dst, MMX64 *src);
void punpckhbw(MMX64 *dst, MMX64 *src);
void punpckhwd(MMX64 *dst, MMX64 *src);
void punpckhdq(MMX64 *dst, MMX64 *src);
void pxor(MMX64 *dst, MMX64 *src);

void get_info(info* info);
/* A bunch of macros, most are never used */

#define CHECK 3
#define R(x,y) ((y*width+x)*3)
#define G(x,y) ((y*width+x)*3+1)
#define B(x,y) ((y*width+x)*3+2)
#define Y(x,y) R(x,y)
#define Cb(x,y) G(x,y)
#define Cr(x,y) B(x,y)
#define AORIG(x,y) (y*width+x)
#define AM(x,y) (8*y)

/* These define the coords for an image point whose color will be used as
   the key color
*/

#define XX 0
#define YY 0
/* This file contains MMX ready implementation of a chroma key function (ck)
   along with the main program, which reads in foreground and background
   frames (in raw ppm format) and outputs the combined images (also in raw
   ppm). Quite a few  variables in main and macros are never used - they are
   left from ealier versions (floating point and/or integer implementations)
   and I'm too lazy to clean up the code. The code is in no way a paragon
   of efficiency - lots of unnecessary register-register moves, for example.
   Optimizing MMX is an infinite process and I don't have neither time nor
   wish to do it. The code provided here because people asked for it and for
   no other reason. If you want to use it, you are on your own. Although the
   code is tested on some sample images and works (otherwise there would not
   be any sense to put it here), no really extensive testing was done, and
   with about 300 lines of MMX, which is essentially assembly level,
   you know ...
   If you want to convert this to real MMX (what else this can be used for ?),
   most of operations have 1-to-1 correspondence to assembler instructions, so
   just replace them with ones, i.e. function call psraw_imm8(MM3,8) corresponds
   to PSRAW mm3,8 in assembler. You will have to change memory loads and
   stores to assembler language analogs as well. mov8bytes_ld and mov8bytes_st
   are directly replacable by movq with memory operand. mov4short_ld_d is
   used for loading parameters only, it can be either simulated with loading a
   parameter and doing some register moves and masks, or, better,
   by storing four copies of parameters inside get_info in a way that they
   can be loaded with one movq. In any case, the result should be an MMX
   register with four identical 16-bit values of the parameter in it.
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* This structure contains all the informtion needed by choma key. Only
   parameters ending with M are really used */
/* Globals - frame size and user-specified parameters */

float RGB2Y(color *,colorY *);

//int main(int argc, char **argv){
//
//FILE *fg_file, *bg_file, *out_file;
//char dummy_char,dummy_char1;
//char fg_fn[100], bg_fn[100], out_fn[100];
//int fi,i,j,bg_width,fg_width,bg_height,fg_height,dummy_int;
//int np,ro,go,bo,rd,gd,bd;
//int lc,num_loops,processed;
//double time_used, snr, lo,ld;
//char *fgcb,*fgcr,*bgcb,*bgcr,*resultcb,*resultcr;
//unsigned char *fgy,*bgy,*resulty;
//unsigned char rf,gf,bf,rb,gb,bb,r,g,b;
//info ck_info;
//float tmp;
//
// if(argc < 5){
//  printf("Usage: ck backgr_fn foregr_fn out_fn number_of_files\n");
//  exit(1);
// }
// /* Reads number_of_files files and does chroma key on them */
// for(fi=0;fi<atoi(argv[4]);fi++){
// sprintf(bg_fn, "%s",argv[1]);           /* should be changes to required
//                                            file name format. I tested it
//                                            on single file at a time, so
//                                            it will read the same files over
//                                            and over */
// bg_file = fopen(bg_fn,"r");
// sprintf(fg_fn, "%s",argv[2]);
// fg_file = fopen(fg_fn, "r");
// sprintf(out_fn,"%s.%d.ppm",argv[3],fi);
// if( bg_file == NULL || fg_file == NULL){
//  printf("Can't open %s or %s input file\n", bg_fn, fg_fn);
//  exit(1);
// }
//
// /* Parses .ppm file */
//
// fscanf(bg_file, "%c%c\n",&dummy_char,&dummy_char);
// dummy_char = (char)getc(bg_file);
// while(dummy_char == '#'){
//  while( ((char)getc(bg_file)) != '\n');
//  dummy_char = (char)getc(bg_file);
// }
// ungetc(dummy_char,bg_file);
// fscanf(bg_file,"%d %d\n%d\n",&bg_width,&bg_height,&dummy_int);
//
// fscanf(fg_file, "%c%c\n",&dummy_char,&dummy_char);
// dummy_char = (char)getc(fg_file);
// while(dummy_char == '#'){
//  while( ((char)getc(fg_file)) != '\n');
//  dummy_char = (char)getc(fg_file);
// }
// ungetc(dummy_char,fg_file);
// fscanf(fg_file,"%d %d\n%d\n",&fg_width,&fg_height,&dummy_int);
// if(fg_width != bg_width || fg_height != bg_height)
//  printf("Warning: frame sizes do not coinside !!!\n");
// printf("File %d. Widths: %d %d, Heights: %d %d\n",fi,fg_width,bg_width,fg_height,bg_height);
//
// /* Allocates pixel arrays */
//
// bgy = (unsigned char *)malloc(bg_width*bg_height*sizeof(char)+8);
// bgcr = (char *)malloc(bg_width*bg_height*sizeof(char)+8);
// bgcb = (char *)malloc(bg_width*bg_height*sizeof(char)+8);
// fgy = (unsigned char *)malloc(fg_width*fg_height*sizeof(char)+8);
// fgcr = (char *)malloc(fg_width*fg_height*sizeof(char)+8);
// fgcb = (char *)malloc(fg_width*fg_height*sizeof(char)+8);
// width = fg_width;
// height = fg_height;
// resulty = (unsigned char *)malloc(width*height*sizeof(char)+8);
// resultcb = (char *)malloc(width*height*sizeof(char)+8);
// resultcr = (char *)malloc(width*height*sizeof(char)+8);
// if(bgy == NULL || fgy == NULL ||
//    bgcb == NULL || fgcb == NULL ||
//    bgcr == NULL || bgcr == NULL){
//  printf("Can't allocate point arrays. Exiting.\n");
//  fclose(bg_file);
//  fclose(fg_file);
//  exit(1);
// }
// /* Read RGB values and convert them into YCbCr planes
//    (result is Y-16, Cb-128, Cr-128)
// */
// for(i=0;i<height;i++){
//  for(j=0;j<width;j++){
//   r = getc(fg_file);
//   g = getc(fg_file);
//   b = getc(fg_file);
//   if(i == YY &&  j == XX){
//     ck_info.key_color.r = r;
//     ck_info.key_color.g = g;
//     ck_info.key_color.b = b;
//   }
//   fgy[AORIG(j,i)] = 0.257*r+0.504*g+0.098*b;
//   fgcb[AORIG(j,i)] = -0.148*r-0.291*g+0.439*b;
//   fgcr[AORIG(j,i)] = 0.439*r-0.368*g-0.071*b;
//   r = getc(bg_file);
//   g = getc(bg_file);
//   b = getc(bg_file);
//   bgy[AORIG(j,i)] = 0.257*r+0.504*g+0.098*b;
//   bgcb[AORIG(j,i)] = -0.148*r-0.291*g+0.439*b;
//   bgcr[AORIG(j,i)] = 0.439*r-0.368*g-0.071*b;
//  }
// }
// initregs(); /* Initializes MMX software registers.  */
//
// /* Asks user for parameters. If using for more than one file,
//    put this outside fi loop
// */
//
// printf("Work angle (10-80 is safe, 0-10 and 80-90 should be avoided):");
// scanf("%f",&angle);
// printf("Noise level (0-63):");
// scanf("%f",&noise_level);
// get_info(&ck_info);
// num_loops = 1; /* Loop structure is an artefact from time measurements */
// for(lc=0;lc<num_loops;lc++)
//   _utah_ck(fgy,fgcb,fgcr,bgy,bgcb,bgcr,resulty,resultcb,resultcr,&ck_info);
//
// /* Convert YCbCr back to RGB and write the output */
// out_file = fopen(out_fn,"w");
// fprintf(out_file,"P6\n%d %d\n255\n",fg_width,fg_height);
// for(i=0;i<fg_height;i++){
//  for(j=0;j<fg_width;j++){
//   tmp = 1.164*resulty[AORIG(j,i)]+1.596*resultcr[AORIG(j,i)];
//   tmp = ( tmp > 0 ) ? tmp : 0;
//   r = ( tmp < 255 ) ? tmp : 255;
//   tmp = 1.164*resulty[AORIG(j,i)]-0.813*resultcr[AORIG(j,i)]-0.392*resultcb[AORIG(j,i)];
//   tmp = ( tmp > 0 ) ? tmp : 0;
//   g = ( tmp < 255 ) ? tmp : 255;
//   tmp = 1.164*resulty[AORIG(j,i)]+2.017*resultcb[AORIG(j,i)];
//   tmp = ( tmp > 0 ) ? tmp : 0;
//   b = ( tmp < 255 ) ? tmp : 255;
//   fprintf(out_file,"%c",r);
//   fprintf(out_file,"%c",g);
//   fprintf(out_file,"%c",b);
//  }
// }
// fclose(bg_file);
// fclose(fg_file);
// fclose(out_file);
// printf("Finished writing output file %d.\n",fi);
// }
// exit(0);
//}

/* This is the chroma key function. It is reasonably autonomous, but
   assumes global variables width and height are set correctly
   Inputs: foreground Y, Cb, Cr arrays (fgy,fgcb,fgcr),
           background Y, Cb, Cr arrays (bgy,bgcb,bgcr),
           place to write the result's Y,Cb,Cr (resy,rescb,rescr),
           information structure (cki).
   IMPORTANT: all Y are of type UNSIGNED char, while all Cb, Cr are
   SIGNED chars  */

//void _utah_ck_sse(unsigned char *fgy, char *fgcb, char *fgcr,
//			  unsigned char *bgy, char *bgcb, char *bgcr,
//			  unsigned char *resy, char *rescb, char *rescr,info *cki)
//{
//	short cb[4] = { cki->key_colorYM.cb, cki->key_colorYM.cb, cki->key_colorYM.cb, cki->key_colorYM.cb };
//	short cr[4] = { cki->key_colorYM.cr, cki->key_colorYM.cr, cki->key_colorYM.cr, cki->key_colorYM.cr };
//	short tgm[4] = { cki->accept_angle_tgM, cki->accept_angle_tgM, cki->accept_angle_tgM, cki->accept_angle_tgM };
//	short ctgm[4] = { cki->accept_angle_ctgM, cki->accept_angle_ctgM, cki->accept_angle_ctgM, cki->accept_angle_ctgM };
//	short kgm[4] = { cki->kgM, cki->kgM, cki->kgM, cki->kgM };
//	short sqm[4] = { cki->noise_level_sqM, cki->noise_level_sqM, cki->noise_level_sqM, cki->noise_level_sqM };
//	short kcm[4] = { cki->one_over_kcM, cki->one_over_kcM, cki->one_over_kcM, cki->one_over_kcM };
//	short scalem[4] = { cki->kfgy_scaleM, cki->kfgy_scaleM, cki->kfgy_scaleM, cki->kfgy_scaleM };
//	unsigned char temp1[8], temp2[8], tempcb[8], tempcr[8];
//
//	int height = cki->height;
//	int width = cki->width;
//	for(int i=0;i<height;i++){
//		for (int j = 0; j < width; ++j) {
//
//			int off = width * i + j;
//
//			void* fgy2 = (void *)(fgy + off);
//			void* fgcb2 = (void *)(fgcb + off);
//			void* fgcr2 = (void *)(fgcr + off);
//
//			void* bgy2 = (void *)(bgy + off);
//			void* bgcb2 = (void *)(bgcb + off);
//			void* bgcr2 = (void *)(bgcr + off);
//
//			void* resy2 = (void *)(resy + off);
//			void* rescb2 = (void *)(rescb + off);
//			void* rescr2 = (void *)(rescr + off);
//
//			__asm
//			{
//				movq mm6, cb;
//				movq mm7, cr;
//
//				mov eax, fgcb2;
//				movq mm0, qword ptr [eax];
//
//				mov eax, fgcr2;
//				movq mm1, qword ptr [eax];
//				pxor mm2, mm2;
//				pxor mm3, mm3;
//				punpcklbw mm2, mm0;
//				psraw mm2, 8;
//				punpcklbw mm3, mm1;
//				psraw mm3, 8;
//				movq mm5, mm3;
//				pmullw mm4, mm6;
//				pmullw mm5, mm7;
//				pmullw mm2, mm7;
//				pmullw mm3, mm6;
//				paddsw mm4, mm5;
//				psubsw mm3, mm2;
//				psraw mm4, 7;
//				psraw mm3, 7;
//
//				movq mm2, tgm;
//				pmullw mm2, mm4;
//				psraw mm2, 4;
//				mov eax, 00010001h;
//				movd mm6, eax;
//				movq mm5, mm6;
//				psllq mm6, 32;
//				por mm6, mm5;
//				movq mm5, mm3;
//				pcmpeqd mm7, mm7;
//				pandn mm5, mm7;
//				paddsw mm5, mm6;
//				movq mm6, mm3;
//				pxor mm7, mm7;
//				pcmpgtw mm7, mm3;
//				pand mm5, mm7;
//				pandn mm7, mm3;
//				por mm5, mm7;
//
//				movq mm7, mm5;
//				movq mm5, mm2;
//				movq mm2, mm7;
//
//				pcmpgtw mm2, mm5;
//				movq mm0, mm2;
//				movq mm5, mm7;
//
//				movq mm6, ctgm;
//				pmullw mm6, mm5;
//				psrlw mm6, 4;
//				movq mm7, mm4;
//				psubusw mm7, mm6;
//				movq mm5, kgm;
//				psubsw mm5, mm4;
//				pmullw mm5, mm5;
//				movq mm4, mm6;
//				movq mm6, mm3;
//				pmullw mm6, mm6;
//				paddusw mm5, mm6;
//				pxor mm2, mm2;
//				pcmpgtw mm2, mm5;
//				movq mm6, sqm;
//				pcmpgtw mm5, mm6;
//				por mm5, mm2;
//				movq mm6, kcm;
//				pmullw mm6, mm7;
//				psrlw mm6, 1;
//				pand mm6, mm5;
//				pcmpeqw mm1, mm1;
//				psrlw mm1, 8;
//				pandn mm5, mm1;
//				por mm6, mm5;
//
//				movq mm5, scalem;
//				pmullw mm5, mm7;
//				psrlw mm5, 4;
//				mov eax, fgy2;
//				movq mm7, qword ptr [eax];
//				pxor mm2, mm2;
//				punpcklbw mm2, mm7;
//				psrlw mm2, 8;
//				psubusw mm2, mm5;
//
//				mov eax, bgy2;
//				movq mm5, qword ptr [eax];
//				punpcklbw mm7, mm5;
//				psrlw mm7, 8;
//				pmullw mm7, mm6;
//				psrlw mm7, 8;
//				paddusw mm2, mm7;
//
//				mov eax, fgy2;
//				movq mm5, qword ptr [eax];
//				punpcklbw mm7, mm5;
//				psrlw mm7, 8;
//				pand mm7, mm0;
//				movq mm5, mm0;
//				pandn mm5, mm2;
//				por mm5, mm7;
//				movq temp1, mm5;
//
//				movq mm2, mm4;
//				movq mm5, mm3;
//				movq mm1, cb;
//				movq mm7, cr;
//				pmullw mm4, mm1;
//				pmullw mm5, mm7;
//				pmullw mm2, mm7;
//				pmullw mm3, mm1;
//				psubsw mm4, mm5;
//				paddsw mm3, mm2;
//				psraw mm4, 7;
//				psraw mm3, 7;
//
//				mov eax, bgcb2;
//				movq mm2, qword ptr [eax];
//				mov eax, bgcr2;
//				movq mm1, qword ptr [eax];
//				punpcklbw mm7, mm2;
//				psraw mm7, 8;
//				punpcklbw mm5, mm1;
//				psraw mm5, 8;
//				pmullw mm7, mm6;
//				psraw mm7, 8;
//				paddsw mm4, mm7;
//				pmullw mm5, mm6;
//				psraw mm5, 8;
//				paddsw mm3, mm5;
//
//				movq mm2, mm0;
//				mov eax, fgcb2;
//				movq mm0, qword ptr [eax];
//				punpcklbw mm6, mm0;
//				psraw mm6, 8;
//				pand mm6, mm2;
//				movq mm7, mm2;
//				pandn mm2, mm4;
//				por mm2, mm6;
//				movq tempcb, mm2;
//				mov eax, fgcr2;
//				movq mm1, qword ptr [eax];
//				punpcklbw mm6, mm1;
//				psraw mm6, 8;
//				pand mm6, mm7;
//				movq mm2, mm7;
//				pandn mm7, mm3;
//				por mm7, mm6;
//				movq tempcr, mm7;
//				/* --------------------  next 4 pixels.  */
//
//				movq mm6, cb;
//				movq mm7, cr;
//
//				mov eax, fgcb2;
//				movq mm0, qword ptr [eax];
//				mov eax, fgcr2;
//				movq mm1, qword ptr [eax];
//				pxor mm2, mm2;
//				pxor mm3, mm3;
//				punpckhbw mm2, mm0;
//				psraw mm2, 8;
//				punpckhbw mm3, mm1;
//				psraw mm3, 8;
//				movq mm5, mm3;
//				pmullw mm4, mm6;
//				pmullw mm5, mm7;
//				pmullw mm2, mm7;
//				pmullw mm3, mm6;
//				paddsw mm4, mm5;
//				psubsw mm3, mm2;
//				psraw mm4, 7;
//				psraw mm3, 7;
//
//				movq mm2, tgm;
//				pmullw mm2, mm4;
//				psraw mm2, 4;
//				mov eax, 00010001h;
//				movd mm6, eax;
//				movq mm5, mm6;
//				psllq mm6, 32;
//				por mm6, mm5;
//				movq mm5, mm3;
//				pcmpeqd mm7, mm7;
//				pandn mm5, mm7;
//				paddsw mm5, mm6;
//				movq mm6, mm3;
//				pxor mm7, mm7;
//				pcmpgtw mm7, mm3;
//				pand mm5, mm7;
//				pandn mm7, mm3;
//				por mm5, mm7;
//
//				movq mm7, mm5;
//				movq mm5, mm2;
//				movq mm2, mm7;
//
//				pcmpgtw mm2, mm5;
//				movq mm0, mm2;
//				movq mm5, mm7;
//
//				movq mm6, ctgm;
//				pmullw mm6, mm5;
//				psrlw mm6, 4;
//				movq mm7, mm4;
//				psubusw mm7, mm6;
//				movq mm5, kgm;
//				psubsw mm5, mm4;
//				pmullw mm5, mm5;
//				movq mm4, mm6;
//				movq mm6, mm3;
//				pmullw mm6, mm6;
//				paddusw mm5, mm6;
//				pxor mm2, mm2;
//				pcmpgtw mm2, mm5;
//				movq mm6, sqm;
//				pcmpgtw mm5, mm6;
//				por mm5, mm2;
//				movq mm6, kcm;
//				pmullw mm6, mm7;
//				psrlw mm6, 1;
//				pand mm6, mm5;
//				pcmpeqw mm1, mm1;
//				psrlw mm1, 8;
//				pandn mm5, mm1;
//				por mm6, mm5;
//
//				movq mm5, scalem;
//				pmullw mm5, mm7;
//				psrlw mm5, 4;
//				mov eax, fgy2;
//				movq mm7, qword ptr [eax];
//				pxor mm2, mm2;
//				punpckhbw mm2, mm7;
//				psrlw mm2, 8;
//				psubusw mm2, mm5;
//
//				mov eax, bgy2;
//				movq mm5, qword ptr [eax];
//				punpckhbw mm7, mm5;
//				psrlw mm7, 8;
//				pmullw mm7, mm6;
//				psrlw mm7, 8;
//				paddusw mm2, mm7;
//
//				mov eax, fgy2;
//				movq mm5, qword ptr [eax];
//				punpckhbw mm7, mm5;
//				psrlw mm7, 8;
//				pand mm7, mm0;
//				movq mm5, mm0;
//				pandn mm5, mm2;
//				por mm5, mm7;
//				movq temp2, mm5;
//
//				movq mm2, mm4;
//				movq mm5, mm3;
//				movq mm1, cb;
//				movq mm7, cr;
//				pmullw mm4, mm1;
//				pmullw mm5, mm7;
//				pmullw mm2, mm7;
//				pmullw mm3, mm1;
//				psubsw mm4, mm5;
//				paddsw mm3, mm2;
//				psraw mm4, 7;
//				psraw mm3, 7;
//
//				mov eax, bgcb2;
//				movq mm2, qword ptr [eax];
//				mov eax, bgcr2;
//				movq mm1, qword ptr [eax];
//				punpckhbw mm7, mm2;
//				psraw mm7, 8;
//				punpckhbw mm5, mm1;
//				psraw mm5, 8;
//				pmullw mm7, mm6;
//				psraw mm7, 8;
//				paddsw mm4, mm7;
//				pmullw mm5, mm6;
//				psraw mm5, 8;
//				paddsw mm3, mm5;
//
//				movq mm2, mm0;
//				mov eax, fgcb2;
//				movq mm0, qword ptr [eax];
//				punpckhbw mm6, mm0;
//				psraw mm6, 8;
//				pand mm6, mm2;
//				movq mm7, mm2;
//				pandn mm2, mm4;
//				por mm2, mm6;
//				mov eax, fgcr2;
//				movq mm1, qword ptr [eax];
//				punpckhbw mm6, mm1;
//				psraw mm6, 8;
//				pand mm6, mm7;
//				pandn mm7, mm3;
//				por mm7, mm6;
//
//				movq mm0, mm2;
//				movq mm4, tempcb;
//				packsswb mm4, mm0;
//				mov eax, rescb2;
//				movq qword ptr [eax], mm4;
//				movq mm1, mm7;
//				movq mm3, tempcr;
//				packsswb mm3, mm1;
//				mov eax, rescr2;
//				movq qword ptr [eax], mm3;
//				movq mm0, temp2;
//				movq mm4, temp1;
//				packuswb mm4, mm0;
//				mov eax, resy2;
//				movq qword ptr [eax], mm4;
//				EMMS;
//			}
//		}
//	}
//}


#define A(x,y) (x + y * width)

void _utah_ck2(unsigned char *fgy, char *fgcb, char *fgcr, char *fga,
        unsigned char* bgy, unsigned char* bgcb, unsigned char* bgcr,
        unsigned char *resy, char *rescb, char *rescr, unsigned char* resa, info *cki){
 unsigned i,j,r,g,b;
 int work_count = 0;
 int /*float*/ sp,kfg,kbg;
 int /*float*/ alfa1,alfa2,alfa3,x1,x2,x3,y1,y2,y3;
 short tmp,tmp1;
 char x,z;

 float noise_level = cki->noise_level;

 int height = cki->height;
 int width = cki->width;
 for(i=0;i<height;i++){
  for(j=0;j<width;j++){

   /* Convert foreground to XZ coords where X direction is defined by
      the key color */

   int a = (unsigned char)fga[A(j,i)];
   if (a == 0)
   {
       resa[A(j,i)] = 0;
	   if (bgy != NULL)
	   {
		   resy[A(j,i)] = *bgy;
		   rescb[A(j,i)] = *bgcb;
		   rescr[A(j,i)] = *bgcr;
	   }
	   continue;
  }

  int cb = (unsigned char)fgcb[A(j,i)] - 128;
  int cr = (unsigned char)fgcr[A(j,i)] - 128;

  if (abs(cb) < 10 && abs(cr) < 10) /* No cb cr, direct use fg. */
  {
	  cb = 0;
	  cr = 0;
  }
   tmp = (cb*cki->key_colorY.cb + cr*cki->key_colorY.cr) >>7;   /*/128; */
   if(tmp > 127) tmp = 127;
   if(tmp < -128) tmp = -128;
   x = tmp;
   tmp = (cr*cki->key_colorY.cb - cb*cki->key_colorY.cr) >>7;   /*/128; */
   if(tmp > 127) tmp = 127;
   if(tmp < -128) tmp = -128;
   z = tmp;

   /* WARNING: accept angle should never be set greater than "somewhat less
   than 90 degrees" to avoid dealing with negative/infinite tg. In reality,
   80 degrees should be enough if foreground is reasonable. If this seems
   to be a problem, go to alternative ways of checking point position
   (scalar product or line equations). This angle should not be too small
   either to avoid infinite ctg (used to suppress foreground without use of
   division)*/

   tmp = ((short)(x)*cki->accept_angle_tg) >>4; /* /0x10; */
   if(tmp > 127) tmp = 127;
   if (abs(z) >= tmp ){
                                 /* keep foreground Kfg = 0*/
    resy[A(j,i)] = fgy[A(j,i)];
    rescb[A(j,i)] = fgcb[A(j,i)];
	rescr[A(j,i)] = fgcr[A(j,i)];
	resa[A(j,i)] = 255;
    /*resy[A(j,i)] = rescb[A(j,i)] = rescr[A(j,i)] = 0; */
   }else {
     work_count++;

     /* Compute Kfg (implicitly) and Kbg, suppress foreground in XZ coord
        according to Kfg */

     tmp = ((short)(z)*cki->accept_angle_ctg)  >> 4; /*/0x10; */
     if(tmp > 127) tmp = 127;
     if(tmp < -128) tmp = -128;
     x1 = abs(tmp);
     y1 = z;

     tmp1 = x-x1;
     if(tmp1 < 0) tmp1 = 0;
     kbg = (((unsigned char)(tmp1)*(unsigned short)(cki->one_over_kc))/2);
     if(kbg < 0) kbg = 0;
     if(kbg > 255) kbg = 255;
     tmp = ((unsigned short)(tmp1)*cki->kfgy_scale) >> 4;/* /0x10; */
     if(tmp > 0xFF) tmp = 0xFF;
     tmp = fgy[A(j,i)] - tmp;
     if(tmp < 0) tmp = 0;
     resy[A(j,i)] = tmp;

     /* Convert suppressed foreground back to CbCr */

     tmp = ((char)(x1)*(short)(cki->key_colorY.cb)-
            (char)(y1)*(short)(cki->key_colorY.cr)) >> 7; /*/128; */
     if(tmp < -128 ) rescb[A(j,i)] = -128;
     else if (tmp > 127) rescb[A(j,i)] = 127;
     else rescb[A(j,i)] = tmp;

     tmp = ((char)(x1)*(short)(cki->key_colorY.cr)+
            (char)(y1)*(short)(cki->key_colorY.cb)) >> 7; /*/128; */
     if(tmp < -128 ) rescr[A(j,i)] = -128;
     else if (tmp > 127) rescr[A(j,i)] = 127;
     else rescr[A(j,i)] = tmp;

     /* Deal with noise. For now, a circle around the key color with
        radius of noise_level treated as exact key color. Introduces
        sharp transitions.
     */

     tmp = z*(short)(z)+(x-cki->kg)*(short)(x-cki->kg);
     if (tmp > 0xFFFF) tmp = 0xFFFF;
     if(tmp < noise_level*noise_level ){
      /* Uncomment this if you want total suppression within the noise circle */
      /*resy[A(j,i)]=rescb[A(j,i)]=rescr[A(j,i)]=0; */
      kbg = 255;
     }

	 resa[A(j, i)] = 255 - kbg;

     /* Add Kbg*background */

	 if (bgy != NULL)
	 {
		 int bgy2 = *bgy - 16;
		 int bgcb2 = *bgcb - 128;
		 int bgcr2 = *bgcr - 128;

		 //tmp = resy[A(j,i)]+ ((unsigned short)(kbg)*bgy2)/255;
		 //resy[A(j,i)] = ( tmp < 255 - 16 ) ? tmp + 16 : 255;
		 //tmp = rescb[A(j,i)]+ ((unsigned short)(kbg)*bgcb2)/255;
		 //if(tmp < -128 ) rescb[A(j,i)] = 0;
		 //else if (tmp > 127) rescb[A(j,i)] = 255;
		 //else rescb[A(j,i)] = tmp + 128;
		 //tmp = rescr[A(j,i)]+ ((unsigned short)(kbg)*bgcr2)/255;
		 //if(tmp < -128 ) rescr[A(j,i)] = 0;
		 //else if (tmp > 127) rescr[A(j,i)] = 255;
		 //else rescr[A(j,i)] = tmp + 128;

		 tmp = ((255 - kbg) * resy[A(j,i)]+ (unsigned short)(kbg)*bgy2)/255;
		 resy[A(j,i)] = ( tmp < 255 - 16 ) ? tmp + 16 : 255;
		 tmp = ((255 - kbg) * rescb[A(j,i)] + (unsigned short)(kbg)*bgcb2)/255;
		 if(tmp < -128 ) rescb[A(j,i)] = 0;
		 else if (tmp > 127) rescb[A(j,i)] = 255;
		 else rescb[A(j,i)] = tmp + 128;
		 tmp = ((255 - kbg) * rescr[A(j,i)]+ (unsigned short)(kbg)*bgcr2)/255;
		 if(tmp < -128 ) rescr[A(j,i)] = 0;
		 else if (tmp > 127) rescr[A(j,i)] = 255;
		 else rescr[A(j,i)] = tmp + 128;
	 }
	 else
	 {
		 resy[A(j,i)] = resy[A(j,i)] + 16;
		 rescb[A(j,i)] = rescb[A(j,i)] + 128;
		 rescr[A(j,i)] = rescr[A(j,i)] + 128;
	 }
   }
  }
 }
}

void _utah_ck(unsigned char *fgy, char *fgcb, char *fgcr, char* fga,
        unsigned char *bgy, char *bgcb, char *bgcr,
        unsigned char *resy, char *rescb, char *rescr,info *cki){
 unsigned i,j,r,g,b;
 int work_count = 0;
 int /*float*/ sp,kfg,kbg;
 int /*float*/ alfa1,alfa2,alfa3,x1,x2,x3,y1,y2,y3;
 short tmp,tmp1;
 char x,z;

 float noise_level = cki->noise_level;

 int height = cki->height;
 int width = cki->width;
 for(i=0;i<height;i++){
  for(j=0;j<width;j++){

   /* Convert foreground to XZ coords where X direction is defined by
      the key color */

  int a = (unsigned char)fga[A(j,i)];
  if (a == 0)
  {
	  resy[A(j,i)] = bgy[A(j, i)];
	  rescb[A(j,i)] = bgcb[A(j, i)];
	  rescr[A(j,i)] = bgcr[A(j, i)];
	  continue;
  }

  int cb = (unsigned char)fgcb[A(j,i)] - 128;
  int cr = (unsigned char)fgcr[A(j,i)] - 128;

  if (abs(cb) < 10 && abs(cr) < 10) /* No cb cr, direct use fg. */
  {
	  cb = 0;
	  cr = 0;
  }

   tmp = (cb*cki->key_colorY.cb + cr*cki->key_colorY.cr) >>7;   /*/128; */
   if(tmp > 127) tmp = 127;
   if(tmp < -128) tmp = -128;
   x = tmp;
   tmp = (cr*cki->key_colorY.cb - cb*cki->key_colorY.cr) >>7;   /*/128; */
   if(tmp > 127) tmp = 127;
   if(tmp < -128) tmp = -128;
   z = tmp;

   /* WARNING: accept angle should never be set greater than "somewhat less
   than 90 degrees" to avoid dealing with negative/infinite tg. In reality,
   80 degrees should be enough if foreground is reasonable. If this seems
   to be a problem, go to alternative ways of checking point position
   (scalar product or line equations). This angle should not be too small
   either to avoid infinite ctg (used to suppress foreground without use of
   division)*/

   tmp = ((short)(x)*cki->accept_angle_tg) >>4; /* /0x10; */
   if(tmp > 127) tmp = 127;
   if (abs(z) >= tmp ){
                                 /* keep foreground Kfg = 0*/
    resy[A(j,i)] = fgy[A(j,i)];
    rescb[A(j,i)] = fgcb[A(j,i)];
    rescr[A(j,i)] = fgcr[A(j,i)];
    /*resy[A(j,i)] = rescb[A(j,i)] = rescr[A(j,i)] = 0; */
   }else {
     work_count++;

     /* Compute Kfg (implicitly) and Kbg, suppress foreground in XZ coord
        according to Kfg */

     tmp = ((short)(z)*cki->accept_angle_ctg)  >> 4; /*/0x10; */
     if(tmp > 127) tmp = 127;
     if(tmp < -128) tmp = -128;
     x1 = abs(tmp);
     y1 = z;

     tmp1 = x-x1;
     if(tmp1 < 0) tmp1 = 0;
     kbg = (((unsigned char)(tmp1)*(unsigned short)(cki->one_over_kc))/2);
     if(kbg < 0) kbg = 0;
     if(kbg > 255) kbg = 255;
     tmp = ((unsigned short)(tmp1)*cki->kfgy_scale) >> 4;/* /0x10; */
     if(tmp > 0xFF) tmp = 0xFF;
     tmp = fgy[A(j,i)] - tmp;
     if(tmp < 0) tmp = 0;
     resy[A(j,i)] = tmp;

     /* Convert suppressed foreground back to CbCr */

     tmp = ((char)(x1)*(short)(cki->key_colorY.cb)-
            (char)(y1)*(short)(cki->key_colorY.cr)) >> 7; /*/128; */
     if(tmp < -128 ) rescb[A(j,i)] = -128;
     else if (tmp > 127) rescb[A(j,i)] = 127;
     else rescb[A(j,i)] = tmp;

     tmp = ((char)(x1)*(short)(cki->key_colorY.cr)+
            (char)(y1)*(short)(cki->key_colorY.cb)) >> 7; /*/128; */
     if(tmp < -128 ) rescr[A(j,i)] = -128;
     else if (tmp > 127) rescr[A(j,i)] = 127;
     else rescr[A(j,i)] = tmp;

     /* Deal with noise. For now, a circle around the key color with
        radius of noise_level treated as exact key color. Introduces
        sharp transitions.
     */

     tmp = z*(short)(z)+(x-cki->kg)*(short)(x-cki->kg);
     if (tmp > 0xFFFF) tmp = 0xFFFF;
     if(tmp < noise_level*noise_level ){
      /* Uncomment this if you want total suppression within the noise circle */
      /*resy[A(j,i)]=rescb[A(j,i)]=rescr[A(j,i)]=0; */
      kbg = 255;
     }

     /* Add Kbg*background */

	 int bgy2 = (unsigned char)bgy[A(j,i)] - 16;
	 int bgcb2 = (unsigned char)bgcb[A(j,i)] - 128;
	 int bgcr2 = (unsigned char)bgcr[A(j,i)] - 128;

	 tmp = ((255 - kbg) * resy[A(j,i)]+ (unsigned short)(kbg)*bgy2)/255;
	 resy[A(j,i)] = ( tmp < 255 - 16 ) ? tmp + 16 : 255;
	 tmp = ((255 - kbg) * rescb[A(j,i)] + (unsigned short)(kbg)*bgcb2)/255;
	 if(tmp < -128 ) rescb[A(j,i)] = 0;
	 else if (tmp > 127) rescb[A(j,i)] = 255;
	 else rescb[A(j,i)] = tmp + 128;
	 tmp = ((255 - kbg) * rescr[A(j,i)]+ (unsigned short)(kbg)*bgcr2)/255;
	 if(tmp < -128 ) rescr[A(j,i)] = 0;
	 else if (tmp > 127) rescr[A(j,i)] = 255;
	 else rescr[A(j,i)] = tmp + 128;

     //tmp = resy[A(j,i)]+ ((unsigned short)(kbg)*bgy2)/255;
     //resy[A(j,i)] = ( tmp < 255 - 16 ) ? tmp + 16 : 255;
     //tmp = rescb[A(j,i)]+ ((unsigned short)(kbg)*bgcb2)/255;
     //if(tmp < -128 ) rescb[A(j,i)] = 0;
     //else if (tmp > 127) rescb[A(j,i)] = 255;
     //else rescb[A(j,i)] = tmp + 128;
     //tmp = rescr[A(j,i)]+ ((unsigned short)(kbg)*bgcr2)/255;
     //if(tmp < -128 ) rescr[A(j,i)] = 0;
     //else if (tmp > 127) rescr[A(j,i)] = 255;
     //else rescr[A(j,i)] = tmp + 128;

   }
  }
 }
}

/* Fills in cki structure using global variables angle and noise_level.
   Main program ask user for these parameters. Mess here is due to lots
   of different parameters tried. Only the ones ending with capital M
   are used inside the ck function */

//void get_info(info *cki){
// float r,g,b,kgl;
// float tmp;
// kgl = RGB2Y(&cki->key_color,&cki->key_colorY);
// cki->accept_angle_cos = cos(M_PI*angle/180);
// cki->accept_angle_sin = sin(M_PI*angle/180);
// tmp = 0xF*tan(M_PI*angle/180);
// if(tmp > 0xFF) tmp = 0xFF;
// cki->accept_angle_tg = tmp;
// cki->accept_angle_tgM = (unsigned short)tmp;
// tmp = 0xF/tan(M_PI*angle/180);
// if(tmp > 0xFF) tmp = 0xFF;
// cki->accept_angle_ctg = tmp;
// cki->accept_angle_ctgM = (unsigned short)tmp;
// tmp  = 1/(kgl);
// cki->one_over_kc = 0xFF*2*tmp-0xFF;
// cki->one_over_kcM = (unsigned short)(cki->one_over_kc);
// tmp = 0xF*(float)(cki->key_colorY.y)/kgl;
// if(tmp > 0xFF) tmp = 0xFF;
// cki->kfgy_scale = tmp;
// cki->kfgy_scaleM = (unsigned short)tmp;
// if (kgl > 127) kgl = 127;
// cki->kg = kgl;
// cki->kgM = kgl;
// printf("UUU %d and %f\n",cki->kgM, kgl);
// cki->noise_level_sqM = (unsigned short)(noise_level*noise_level);
//
// cki->key_colorYM.y = cki->key_colorY.y;
// cki->key_colorYM.cb = (cki->key_colorY.cb);
// cki->key_colorYM.cr = (cki->key_colorY.cr);
//}

/* Converts RGB to YCbCr, returns the length of (Cb,Cr) vector */

float RGB2Y(color *c, colorY *y){
 float tmp,tmp1,tmp2;
 y->y = 0.257*c->r+0.504*c->g+0.098*c->b;
 tmp1 = -0.148*c->r-0.291*c->g+0.439*c->b;
 tmp2 = 0.439*c->r-0.368*c->g-0.071*c->b;
 tmp = sqrt(tmp1*tmp1+tmp2*tmp2);
 y->cb = 127*(tmp1/tmp);
 y->cr = 127*(tmp2/tmp);
 return tmp;
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <tiff-4.0.3/libtiff/tiffio.h>

#define BUFF_MAX 512
#define OPT 1
#define Isize 512
#define Jsize 512
#define EXH 9
#define EXV 9
#define STEP 256
#define MAX 250
#define Bnum 6
#define Fcol 255 | 255 << 8 | 255 << 16
#define Bcol 0
#define Right 100
#define BS 90
#define Xsize Jsize * 2 + 20 + Right
#define Ysize Isize * 2 + 5

unsigned char red[STEP], green[STEP], blue[STEP];
unsigned char dat1[Isize][Jsize];
short int wdat[Isize][Jsize];
short int wdat3[Isize][Jsize];
unsigned char win[Isize][Jsize];
unsigned char win2[Isize][Jsize];
unsigned char win3[Isize][Jsize];
double ar[Isize][Jsize];
double ai[Isize][Jsize];
double ff[Isize][Jsize];
double fff[Isize][Jsize];
double wdat2[Isize][Jsize];
double b_rl[Isize][Jsize];
double b_im[Isize][Jsize];
double sin_tb1[Isize][Jsize];
double cos_tb1[Isize][Jsize];
double f_rl[Isize][Jsize];
double f_im[Isize][Jsize];

unsigned char tiffdatFFT[Isize][Jsize];
unsigned char tiffdatLPF[Isize][Jsize];
unsigned char tiffdatHPF[Isize][Jsize];
unsigned char buffer[Isize * Jsize];

Display *d;
Window Rtw, W, W1, W2, W3, W4, Side, Bt[Bnum];
GC Gc, GcW1, GcW2, GcW3, GcW4;
Colormap Cmap;
Visual *Vis;
XEvent Ev;
XImage *ImageW1, *ImageW2, *ImageW3, *ImageW4;
Font Ft;
unsigned long Dep;

void init_window(), init_image(), event_select(), read_file();
int buff1[Isize * Jsize], buff2[Isize * Jsize], buff3[Isize * Jsize];

//�\���摜��TIFF�`���ŕۑ�����֐�
void tiff_save(unsigned char img[Isize][Jsize])
{
	TIFF *image;

	int i, j, k;
	char save_fname[256];

	printf("Save file name (***.tiff) : ");
	scanf("%s", save_fname);

	k = 0;
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			buffer[k] = img[i][j];
			k++;
		}
	}
	// Open the TIFF file
	if ((image = TIFFOpen(save_fname, "w")) == NULL)
	{
		printf("Could not open output file for writing\n");
		exit(42);
	}

	// We need to set some values for basic tags before we can add any data
	TIFFSetField(image, TIFFTAG_IMAGEWIDTH, Isize);
	TIFFSetField(image, TIFFTAG_IMAGELENGTH, Jsize);
	TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 1);
	//TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, Jsize);

	TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_SEPARATE);

	TIFFSetField(image, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	//TIFFSetField(image, TIFFTAG_XRESOLUTION, 512.0);
	//TIFFSetField(image, TIFFTAG_YRESOLUTION, 512.0);
	//TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, RESUNIT_CENTIMETER);

	// Write the information to the file
	TIFFWriteEncodedStrip(image, 0, buffer, Isize * Jsize);

	// Close the file
	TIFFClose(image);
}

void read_file()
{
	FILE *fi;
	char name[150];
	int i, j, k;

	printf("File Name  : ");
	scanf("%s", name);
	if ((fi = fopen(name, "r")) == NULL)
	{
		printf("Read Error\n");
		exit(1);
	}
	fread(dat1, 1, Isize * Jsize, fi);
	fclose(fi);

	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			ar[i][j] = (double)dat1[i][j];
		}
	}
}

void show_1()
{
	int i, j, k;

	k = 0;
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			buff1[k] = dat1[i][j] | dat1[i][j] << 8 | dat1[i][j] << 16;
			k++;
		}
	}
	XPutImage(d, W1, GcW1, ImageW1, 0, 0, 0, 0, Jsize, Isize);
}

void show_2()
{
	int i, j, k;

	double max_1, min_1, range;
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			if (!i && !j)
			{
				max_1 = ff[i][j];
				min_1 = ff[i][j];
			}
			else
			{
				if (ff[i][j] > max_1)
					max_1 = ff[i][j];
				else if (ff[i][j] < min_1)
					min_1 = ff[i][j];
			}
		}
	}
	range = (max_1 - min_1) / (double)MAX;
	printf("max -> %lf   min -> %lf   range -> %lf\n", max_1, min_1, range);
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			if (ff[i][j] <= min_1)
				wdat2[i][j] = min_1;
			else if (ff[i][j] >= max_1)
				wdat2[i][j] = max_1;
			else
				wdat2[i][j] = ff[i][j];
		}
	}
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			win2[i][j] = (unsigned char)((wdat2[i][j] - min_1) / range);
		}
	}

	k = 0;
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			buff2[k] = win2[i][j] | win2[i][j] << 8 | win2[i][j] << 16;
			tiffdatFFT[i][j] = win2[i][j]; //�ۑ��p
			k++;
		}
	}

	XPutImage(d, W2, GcW2, ImageW2, 0, 0, 0, 0, Jsize, Isize);
}

void rvmtx1(a, b, xsize, ysize) double a[][Jsize];
double b[][Jsize];
int xsize;
int ysize;
{
	int i, j;
	for (i = 0; i < ysize; i++)
	{
		for (j = 0; j < xsize; j++)
		{
			b[j][i] = a[i][j];
		}
	}
}

void rvmtx2(a, b, xsize, ysize) double a[][Jsize];
double b[][Jsize];
int xsize;
int ysize;
{
	int i, j;
	for (i = 0; i < ysize; i++)
	{
		for (j = 0; j < xsize; j++)
		{
			b[i][j] = a[j][i];
		}
	}
}

void cstb(length, inv, sin_tb1, cos_tb1) int length;
int inv;
double sin_tb1[];
double cos_tb1[];
{
	int i;
	double xx, arg, pi = 3.141592653;

	xx = ((-pi) * 2.0) / (double)length;
	if (inv < 0)
		xx = -xx;
	for (i = 0; i < length; i++)
	{
		arg = (float)i * xx;
		sin_tb1[i] = sin(arg);
		cos_tb1[i] = cos(arg);
	}
}

void birv(a, length, ex) double a[];
int length;
int ex;
{
	int i, ii, k, bit;
	static double b[BUFF_MAX];

	for (i = 0; i < length; i++)
	{
		for (k = 0, ii = i, bit = 0;; bit <<= 1, ii >>= 1)
		{
			bit = (ii & 1) | bit;
			if (++k == ex)
				break;
		}
		b[i] = a[bit];
	}
	for (i = 0; i < length; i++)
		a[i] = b[i];
}

void fft1(a_rl, a_im, ex, inv, sin_tb1, cos_tb1) double a_rl[];
double a_im[];
int ex;
int inv;
double sin_tb1[];
double cos_tb1[];
{
	int i, j, k, w, j1, j2;
	int numb, lenb, timb, length = 1;
	double xr, xi, yr, yi, nrml;

	for (i = 0; i < ex; i++)
		length *= 2;
	cstb(length, inv, sin_tb1, cos_tb1);
	if (OPT == 1)
	{
		for (i = 0; i < length; i += 2)
		{
			a_rl[i] = -a_rl[i];
			a_im[i] = -a_im[i];
		}
	}
	numb = 1;
	lenb = length;
	for (i = 0; i < ex; i++)
	{
		lenb /= 2;
		timb = 0;
		for (j = 0; j < numb; j++)
		{
			w = 0;
			for (k = 0; k < lenb; k++)
			{
				j1 = timb + k;
				j2 = j1 + lenb;
				xr = a_rl[j1];
				xi = a_im[j1];
				yr = a_rl[j2];
				yi = a_im[j2];
				a_rl[j1] = xr + yr;
				a_im[j1] = xi + yi;
				xr = xr - yr;
				xi = xi - yi;
				a_rl[j2] = xr * cos_tb1[w] - xi * sin_tb1[w];
				a_im[j2] = xr * sin_tb1[w] + xi * cos_tb1[w];
				w += numb;
			}
			timb += (2 * lenb);
		}
		numb *= 2;
	}
	birv(a_rl, length, ex);
	birv(a_im, length, ex);
	if (OPT == 1)
	{
		for (i = 1; i < length; i += 2)
		{
			a_rl[i] = -a_rl[i];
			a_im[i] = -a_im[i];
		}
	}
	nrml = 1.0 / sqrt((double)length);
	for (i = 0; i < length; i++)
	{
		a_rl[i] *= nrml;
		a_im[i] *= nrml;
	}
}

void fft2(a_rl, a_im, exh, exv, inv, sin_tb1, cos_tb1) double a_rl[][Jsize];
double a_im[][Jsize];
int exh;
int exv;
int inv;
double sin_tb1[];
double cos_tb1[];
{
	int i, j, k, w, j1, j2, numb, lenb, timb, lenh = 1, lenv = 1;
	double xr, xi, yr, yi, nrml;
	for (i = 0; i < exh; i++)
		lenh *= 2;
	for (i = 0; i < exv; i++)
		lenv *= 2;
	for (i = 0; i < lenv; i++)
	{
		fft1(&a_rl[i][0], &a_im[i][0], exh, inv, sin_tb1, cos_tb1);
	}
	printf("Horizontal  done\n");
	rvmtx1(a_rl, b_rl, lenh, lenv);
	rvmtx1(a_im, b_im, lenh, lenv);
	for (i = 0; i < lenh; i++)
	{
		fft1(&b_rl[i][0], &b_im[i][0], exv, inv, sin_tb1, cos_tb1);
	}
	printf("Vertical  done\n");
	rvmtx2(b_rl, a_rl, lenh, lenv);
	rvmtx2(b_im, a_im, lenh, lenv);
}

void fftimag()
{
	double norm, max_a;
	int i, j, a, b, circ;

	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			ai[i][j] = 0.0;
		}
	}
	printf("FFT  started\n");
	fft2(ar, ai, EXH, EXV, 1, sin_tb1, cos_tb1);
	max_a = 0;
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			norm = ar[i][j] * ar[i][j] + ai[i][j] * ai[i][j];
			if (norm != 0.0)
			{
				norm = log(norm) / 2.0;
			}
			else
			{
				norm = 0.0;
			}
			ff[i][j] = norm;
			if (norm > max_a)
			{
				max_a = norm;
			}
		}
	}
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			ff[i][j] = ff[i][j] * MAX / max_a;
		}
	}
	show_2();
}

void show3(A) int A;
{
	int i, j, k;
	double max_1, min_1, range;

	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			if (!i && !j)
			{
				max_1 = f_rl[i][j];
				min_1 = f_im[i][j];
			}
			else
			{
				if (f_rl[i][j] > max_1)
					max_1 = f_rl[i][j];
				else if (f_rl[i][j] < min_1)
					min_1 = f_rl[i][j];
			}
		}
	}

	range = (max_1 - min_1) / (double)MAX;
	printf("max -> %lf   min -> %lf   range -> %lf\n", max_1, min_1, range);
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			if (f_rl[i][j] <= min_1)
				wdat3[i][j] = min_1;
			else if (f_rl[i][j] >= max_1)
				wdat3[i][j] = max_1;
			else
				wdat3[i][j] = f_rl[i][j];
		}
	}
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			win3[i][j] = (unsigned char)((wdat3[i][j] - min_1) / range);
		}
	}
	k = 0;
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			buff3[k] = win3[i][j] | win3[i][j] << 8 | win3[i][j] << 16;
			if (A == 3)
				tiffdatLPF[i][j] = win3[i][j]; //�ۑ��p
			else if (A == 4)
				tiffdatHPF[i][j] = win3[i][j]; //�ۑ��p
			k++;
		}
	}
	if (A == 3)
		XPutImage(d, W3, GcW3, ImageW3, 0, 0, 0, 0, Jsize, Isize);

	else if (A == 4)
		XPutImage(d, W4, GcW4, ImageW4, 0, 0, 0, 0, Jsize, Isize);
}

void filter1(A) int A;
{
	double norm, max_a;
	int i, j, a, b, circ;

	printf("filter parameter  (a,b) ?  \n");
	printf("a = ");
	scanf("%d", &a);
	printf("b = ");
	scanf("%d", &b);
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			norm = (i - Isize / 2) * (i - Isize / 2) + (j - Jsize / 2) * (j - Jsize / 2);
			circ = sqrt(norm);
			if (A == 3)
			{
				if ((circ >= a) && (circ <= b))
					fff[i][j] = ff[i][j];
				else
					fff[i][j] = 0.0;
			}
			else if (A == 4)
			{
				if ((circ >= a) && (circ <= b))
					fff[i][j] = 0.0;
				else
					fff[i][j] = ff[i][j];
			}
		}
	}

	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			f_rl[i][j] = ar[i][j] * fff[i][j];
			f_im[i][j] = ai[i][j] * fff[i][j];
		}
	}
	printf("Inverce FFT Started\n");
	fft2(f_rl, f_im, EXH, EXV, -1, sin_tb1, cos_tb1);
	max_a = 0;
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			norm = f_rl[i][j];
			if (norm > max_a)
				max_a = norm;
		}
	}
	for (i = 0; i < Isize; i++)
	{
		for (j = 0; j < Jsize; j++)
		{
			f_rl[i][j] = f_rl[i][j] * MAX / max_a;
		}
	}
	show3(A);
}

void init_window()
{
	int i;

	d = XOpenDisplay(NULL);
	Rtw = RootWindow(d, 0);
	Vis = XDefaultVisual(d, 0);
	Dep = XDefaultDepth(d, 0);

	W = XCreateSimpleWindow(d, Rtw, 0, 0, Xsize, Ysize, 2, Fcol, Bcol);
	W1 = XCreateSimpleWindow(d, W, 0, 0, Jsize, Isize, 2, Fcol, Bcol);
	W2 = XCreateSimpleWindow(d, W, Jsize + 5, 0, Jsize, Isize, 2, Fcol, Bcol);
	W3 = XCreateSimpleWindow(d, W, 0, Isize + 5, Jsize, Isize, 2, Fcol, Bcol);
	W4 = XCreateSimpleWindow(d, W, Jsize + 5, Isize + 5, Jsize, Isize, 2, Fcol, Bcol);
	Side = XCreateSimpleWindow(d, W, Jsize * 2 + 15, 0, Right, Isize, 2, Fcol, Bcol);
	for (i = 0; i < Bnum; i++)
	{
		Bt[i] = XCreateSimpleWindow(d, Side, 0, 30 * i, BS, 30, 2, Fcol, Bcol);
		XSelectInput(d, Bt[i], ExposureMask | ButtonPressMask);
	}

	XSelectInput(d, W1, ButtonPressMask);
	XSelectInput(d, W2, ButtonPressMask);
	XSelectInput(d, W3, ButtonPressMask);
	XSelectInput(d, W4, ButtonPressMask);

	XMapWindow(d, W);
	XMapSubwindows(d, W1);
	XMapSubwindows(d, W2);
	XMapSubwindows(d, W3);
	XMapSubwindows(d, W4);
	XMapSubwindows(d, Side);
	XMapSubwindows(d, W);
}

void view_proc()
{
	read_file();
	show_1();
}

void init_image()
{
	Gc = XCreateGC(d, W, 0, 0);
	GcW1 = XCreateGC(d, W1, 0, 0);
	GcW2 = XCreateGC(d, W2, 0, 0);
	GcW3 = XCreateGC(d, W3, 0, 0);
	GcW4 = XCreateGC(d, W4, 0, 0);

	ImageW1 = XCreateImage(d, Vis, Dep, ZPixmap, 0, NULL, Jsize, Isize, BitmapPad(d), 0);
	ImageW1->data = (char *)buff1;

	ImageW2 = XCreateImage(d, Vis, Dep, ZPixmap, 0, NULL, Jsize, Isize, BitmapPad(d), 0);
	ImageW2->data = (char *)buff2;

	ImageW3 = XCreateImage(d, Vis, Dep, ZPixmap, 0, NULL, Jsize, Isize, BitmapPad(d), 0);
	ImageW3->data = (char *)buff3;

	ImageW4 = XCreateImage(d, Vis, Dep, ZPixmap, 0, NULL, Jsize, Isize, BitmapPad(d), 0);
	ImageW4->data = (char *)buff3;
}

void event_select()
{
	int x, y, input;
	while (1)
	{
		XNextEvent(d, &Ev);
		switch (Ev.type)
		{
		case Expose:
			XSetForeground(d, Gc, Fcol);
			XSetBackground(d, Gc, 10);
			XDrawImageString(d, Bt[0], Gc, 28, 21, "Load", 4);
			XDrawImageString(d, Bt[1], Gc, 28, 21, "FFT", 3);
			XDrawImageString(d, Bt[2], Gc, 28, 21, "Low", 3);
			XDrawImageString(d, Bt[3], Gc, 28, 21, "High", 4);
			XDrawImageString(d, Bt[4], Gc, 28, 21, "Save", 4);
			XDrawImageString(d, Bt[5], Gc, 28, 21, "quit", 4);
			break;
		case ButtonPress:
			if (Ev.xany.window == Bt[0])
			{
				view_proc();
			}
			if (Ev.xany.window == Bt[1])
			{
				fftimag();
			}
			if (Ev.xany.window == Bt[2])
			{
				filter1(3);
			}
			if (Ev.xany.window == Bt[3])
			{
				filter1(4);
			}
			if (Ev.xany.window == Bt[4])
			{
				printf("Which result ? (1:FFT  2:LPF 3:HPF)  =�@");
				scanf("%d", &input);
				if (input == 1)
					tiff_save(tiffdatFFT);
				else if (input == 2)
					tiff_save(tiffdatLPF);
				else if (input == 3)
					tiff_save(tiffdatHPF);
				else
					printf("input error !");
			}
			if (Ev.xany.window == Bt[5])
			{
				exit(1);
			}
			break;
		}
	}
}

int main(void)
{
	init_window();
	init_image();
	event_select();
	return 0;
}

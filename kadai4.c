#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<tiff-4.0.3/libtiff/tiffio.h>

#define Isize  512
#define Jsize  Isize
#define TI		128
#define TJ		TI
#define Bnum   8
#define Xsize  Jsize*2+Right+5
#define Ysize  Isize+5
#define Right  100
#define BS     100  	
#define Fcol   255|255<<8|255<<16	
#define Bcol   1

Display    *d;
Window     Rtw,W,W1,W2,Side,Bt[Bnum];
GC         Gc,GcW1,GcW2;
Visual     *Vis;
XEvent     Ev;
XImage     *ImageW1,*ImageW2;
unsigned long Dep;

void init_window(),init_color(),init_image(),
    event_select(),set_quad();

unsigned char   dat[Isize][Jsize];
unsigned char   template[TI][TJ];

short int fdat[Isize][Jsize];

unsigned char tiffdat[Isize][Jsize];
int buff[Isize*Jsize];	
unsigned char buffer[Isize*Jsize];

int   buff_temp[TI*TJ];

void e_distance(),relation();

void tiff_save(unsigned char img[Isize][Jsize]){
	TIFF *image;
	
	int i,j,k;
	char save_fname[256];

	printf("Save file name (***.tiff) : ");
	scanf("%s",save_fname);
	
	k=0;
	for(i=0;i<Isize;i++){
		for(j=0;j<Jsize;j++){
			buffer[k]=img[i][j];
				k++;
		}
	}
	// Open the TIFF file
	if((image = TIFFOpen(save_fname, "w")) == NULL){
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
	FILE  *fi; 
	char name[150];
	int i,j,k;
	printf("File Name  : ");
	scanf("%s",name);
	if((fi=fopen(name,"r"))==NULL){
		printf("Read Error\n");
		exit(1);
	}
	fread(dat,1,Isize*Jsize,fi);
}

void read_template()
{
	FILE  *fi; 
	char name[150];
	int i,j,k;

	printf("Template  : ");
	scanf("%s",name);
	if((fi=fopen(name,"r"))==NULL){
		printf("Read Error\n");
		exit(1);
	}
	fread(template,1,TI*TJ,fi);
}

void view_imgW1(unsigned char ttt[Isize][Jsize])
{
	int i,j,k;

	k=0;
	for(i=0;i<Isize;i++){
		for(j=0;j<Jsize;j++){
			buff[k]=ttt[i][j]|ttt[i][j]<<8|ttt[i][j]<<16;
			tiffdat[i][j] = ttt[i][j];
				k++;
		}
	}
	XPutImage(d,W1,GcW1,ImageW1,0,0,0,0,Jsize,Isize);
}

void view_imgW2(unsigned char ttt[TI][TJ])
{
	int i,j,k;

	k=0;
	for(i=0;i<TI;i++){
		for(j=0;j<TJ;j++){
			buff_temp[k]=ttt[i][j]|ttt[i][j]<<8|ttt[i][j]<<16;
				k++;
		}
	}
	XPutImage(d,W2,GcW2,ImageW2,0,0,0,0,Jsize,Isize);
}

void e_distance()
{
int si,sj,syori,n;
float d,dmin;

    dmin=1000.0;
    n=0;
    syori=(Isize-TI-1)*(Jsize-TJ-1)/100;
    for(int i=0; i<Isize-TI; i++){      //基本画像内でサイズTI，TJとなる小領域を設定
        for(int j=0; j<Jsize-TJ; j++){
            d=0;
            for(int ii=0; ii<TI; ii++){ //テンプレート画像との距離値の計算
                for(int jj=0; jj<TJ; jj++){
					d += (dat[i+ii][j+jj]-template[ii][jj]) * (dat[i+ii][j+jj]-template[ii][jj]) / 10000.;
					// printf("%f\n", d);
                }
            }
			d = sqrt(d);
            if(d < dmin){
                dmin = d;
                si = i; sj = j;
				// printf("dmin = %f\n", dmin);
            }
            if(n%syori==0)printf("%d％終了\n",n/syori);
            n++;
        }
    }
    printf("座標（%d, %d）でのユークリッド距離は%f\n", si, sj, dmin*100.);  //計算結果(領域の左上座標と距離値)の表示
    set_quad(si,sj,TI,TJ);  //si,sjを始点とする．
}

void relation()
{
	int i, j, ii, jj, x, y, syori, n=0;
	int F_ave=0, G_ave=0;
	double s, s1, s2, s3, smax = 0;


	syori=(Isize-TI-1)*(Jsize-TJ-1)/100;
	for (i=0; i<Isize-TI; i++) {
		for (j=0; j<Jsize-TJ; j++) {
			for (ii=0; ii<TI; ii++) {
				for (jj=0; jj<TJ; jj++) {
					F_ave += dat[ii+i][jj+j];
					G_ave += template[ii][jj];
				}
			}
			F_ave = F_ave / (TI*TJ);
			G_ave = G_ave / (TI*TJ);

			s1 = s2 = s3 = 0;
			for (ii=0; ii<TI; ii++) {
				for (jj=0; jj<TJ; jj++) {
					s1 += (dat[i+ii][j+jj] - F_ave) * (template[ii][jj] - G_ave);
					s2 += (dat[i+ii][j+jj] - F_ave) * (dat[i+ii][j+jj] - F_ave);
					s3 += (template[ii][jj] - G_ave) * (template[ii][jj] - G_ave);
				}
			}

			s = s1 / (sqrt(s2) * sqrt(s3));
			if (s > smax) {
				smax = s;
				x = i; y = j;
			}
			if(n%syori==0) printf("%d％終了\n",n/syori);
            n++;
		}
	}
	printf("（%d, %d）: %f\n", x, y, smax);
	set_quad(x, y, TI, TJ);
}

void init_window()
{
	int i;

	if( (d = XOpenDisplay( NULL )) == NULL ) {
		fprintf( stderr, "Xサーバーがなんちゃらかんちゃら\n" );
		exit(1);
	}

	Rtw=RootWindow(d,0);
	Vis=XDefaultVisual(d,0);
	Dep=XDefaultDepth(d,0);

	W=XCreateSimpleWindow(d,Rtw,0,0,Xsize,Ysize,2,Fcol,Bcol);
	W1=XCreateSimpleWindow(d,W,0,0,Jsize,Isize,2,Fcol,Bcol);
	W2=XCreateSimpleWindow(d,W,Jsize+5,0,TJ,TI,2,Fcol,Bcol);
	Side=XCreateSimpleWindow(d,W,Jsize+TJ+10,0,Right,Isize,2,Fcol,Bcol);
	for(i=0;i<Bnum;i++){
		Bt[i]=XCreateSimpleWindow(d,Side,0,30*i,BS,30,2,Fcol,Bcol);
		XSelectInput(d,Bt[i],ExposureMask | ButtonPressMask);
	}
	XSelectInput(d,W1,ButtonPressMask);
	XSelectInput(d,W2,ButtonPressMask);	

	XMapWindow(d,W);
	XMapSubwindows(d,W1);
	XMapSubwindows(d,W2);
	XMapSubwindows(d,Side);
	XMapSubwindows(d,W);
}

void init_image()
{
	Gc  = XCreateGC(d,W,0,0);
	GcW1= XCreateGC(d,W1,0,0);
	GcW2= XCreateGC(d,W1,0,0);
	
	ImageW1=XCreateImage(d,Vis,Dep,ZPixmap,0,NULL,Jsize,Isize,
			BitmapPad(d),0);
	ImageW1->data = (char *)buff;

	ImageW2=XCreateImage(d,Vis,Dep,ZPixmap,0,NULL,TJ,TI,
			BitmapPad(d),0);
	ImageW2->data = (char *)buff_temp;
}

void event_select()
{
	int x,y;
	while(1){
		XNextEvent(d,&Ev);
		switch(Ev.type){
			case Expose :
				XSetForeground(d,Gc,Fcol);
				XSetBackground(d,Gc,10);
				XDrawImageString(d,Bt[0],Gc,28,21,"Image",5);
				XDrawImageString(d,Bt[1],Gc,28,21,"View",4);
				XDrawImageString(d,Bt[2],Gc,28,21,"Template",8);
				XDrawImageString(d,Bt[3],Gc,28,21,"View T",6);
				XDrawImageString(d,Bt[4],Gc,28,21,"E_DIST",6);
				XDrawImageString(d,Bt[5],Gc,28,21,"Relation",8);
				XDrawImageString(d,Bt[6],Gc,28,21,"Save",4);
				XDrawImageString(d,Bt[Bnum-1],Gc,28,21,"Quit",4);
			break;
			case ButtonPress :
				if(Ev.xany.window == Bt[0]){
					read_file();
				}
				if(Ev.xany.window == Bt[1]){
					view_imgW1(dat);
				}
				if(Ev.xany.window == Bt[2]){
					read_template();
				}
				if(Ev.xany.window == Bt[3]){
					view_imgW2(template);
				}
				if(Ev.xany.window == Bt[4]){
					e_distance();
				}
				if(Ev.xany.window == Bt[5]){
					relation();
				}
				if(Ev.xany.window == Bt[6]){
					tiff_save(tiffdat);
				}
				if(Ev.xany.window == Bt[Bnum-1]){
					exit(1);
				}
				if(Ev.xany.window == W1){
					x=Ev.xbutton.x; y=Ev.xbutton.y;
					printf("(%d %d) %d\n",y,x,dat[y][x]);
				}
			break;
		}
	}
} 

void set_quad(int y,int x,int sizeY,int sizeX)
{
	int sx,sy,ex,ey;
	sx=x; sy=y; ex=sx+sizeX-1; ey=sy+sizeY-1;
	XSetForeground(d,GcW1,255);
	XSetLineAttributes(d,GcW1,1,LineSolid,CapButt,JoinMiter);
	XDrawLine(d,W1,GcW1,sx,sy,ex,sy);
	XDrawLine(d,W1,GcW1,sx,sy,sx,ey);
	XDrawLine(d,W1,GcW1,ex,ey,ex,sy);
	XDrawLine(d,W1,GcW1,ex,ey,sx,ey);
}


int main()
{
	init_window();
	init_image();
	event_select();
	return 0;
}

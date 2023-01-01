
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <stdint.h>
#define SW 2560
#define SH 1920
#define DW (SW/2)
#define DH (SH/2)


// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////

// https://github.com/DEQ2000-cyber/
// PROGRAMA 3D DE PUNTOFIJO RAPIDO Y FACIL.
// SU COMPILCION ES LA NORMAL:
// gcc.exe -Wall -pedantic -o DEQ3D.exe DEQ3D.c

// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREAR ARCHIVOS *.BMP.

void __fastcall SaveBMP(
    const char* filename,
    register unsigned char *data,
    const unsigned int ancho,
    const unsigned int alto,
    unsigned char bpp
    )
{
    register int Y;
    register unsigned int X;
    unsigned int STEP;
    unsigned char BPP;
    unsigned char BYTES_BAZURAS[3];
    if ( bpp<24 )bpp*=8;
    if ( !data ) return;
    const unsigned char padding = ( ( 4 - ( ancho * (bpp/8) ) % 4) % 4 );
    unsigned char header[14], informacionheader[ 40 ];
    memset( header, 0, ( sizeof(unsigned char) * 14 ) );
    memset( informacionheader, 0, ( sizeof(unsigned char) * 40 ) );
    const unsigned int filesize = 14 + 40 + ancho * alto * (bpp/8) + padding * ancho;
    header[0] = 'B',header[1] = 'M';
    header[2] = filesize, header[3] = filesize >> 8, header[4] = filesize >> 16, header[5] = filesize >> 24;
    header[10] = 14 + 40;
    informacionheader[0] = 40;
    informacionheader[4] = ancho, informacionheader[5] = ancho >> 8, informacionheader[6] = ancho >> 16, informacionheader[7]  = ancho >> 24;
    informacionheader[8] = alto,  informacionheader[9] = alto >> 8, informacionheader[10] = alto >> 16,  informacionheader[11] = alto >> 24;
    informacionheader[12] = 1, informacionheader[14] = bpp;
    FILE *f = fopen( filename, "wb" );
    fwrite( header, 14, 1, f );
    fwrite( informacionheader, 40, 1, f );
    BPP = (bpp/8);
    for ( Y = alto-1; Y >=0 ; --Y){
        for ( X = 0; X < ancho; ++X) STEP = ( Y * ancho + X ) * BPP, fwrite( &data[ STEP ], BPP, 1, f );
        fwrite( BYTES_BAZURAS, padding, 1, f );
    }
    fclose(f),f=NULL;
    return;
}

void __fastcall SaveBMPConverter( const char *filename, register void *data, const unsigned int w, const unsigned int h, const unsigned int bpp ){ SaveBMP( filename, (unsigned char*)data, w, h, bpp ); }

// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////



// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREAR Y LEER ARCHIVOS PROPIOS DE 3D *.FVM.

typedef struct __FVM__
{
	unsigned int NV; // Numero de Vertices.
	unsigned int NF; // Numero de Faces.
	int* V; // Buffer a Vertices.
	unsigned int* F; // Buffer a Faces.
}__FVM__;


void GenerateFvm( __FVM__ *Fvm, const char* Name ){
	FILE *f = fopen(Name,"wb");
	fwrite( (unsigned int*)&Fvm->NV, sizeof(unsigned int), 1, f );
	fwrite( (unsigned int*)&Fvm->NF, sizeof(unsigned int), 1, f );
	fwrite( Fvm->V, (sizeof(int)*Fvm->NV * 3), 1, f );
	fwrite( Fvm->F, (sizeof(unsigned int)*Fvm->NF * 3), 1, f );
	fclose(f);
	f=NULL;
	return;
}


void LoadFvm( __FVM__ *Fvm, const char* Name ){
	FILE *f = fopen(Name,"rb");
	fread( &Fvm->NV, sizeof(unsigned int), 1, f );
	fread( &Fvm->NF, sizeof(unsigned int), 1, f );
	Fvm->V=(int*)malloc(sizeof(int)*Fvm->NV*3);
	Fvm->F=(unsigned int*)malloc(sizeof(unsigned int)*Fvm->NF*3);
	fread( Fvm->V, (sizeof(int)*Fvm->NV*3), 1, f );
	fread( Fvm->F, (sizeof(unsigned int)*Fvm->NF*3), 1, f );
	printf("V: %i\nF: %i\n", Fvm->NV, Fvm->NF );
	fclose(f);
	f=NULL;
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////



// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// LEER ARCHIVO *.OBJ Y CONVERTIRLO A *.FVM 3D.

void __fastcall GenerateFvmToObj( __FVM__ *Fvm, const char* Name, int Tam ){
	unsigned int LenVertice;
	unsigned int LenFace;
	FILE *f = fopen(Name,"r");
	char *BUFFER = NULL;
	BUFFER = (char*)malloc(sizeof(char)*512);
	LenFace = LenVertice = 0;
	Fvm->F  = NULL;
	Fvm->V  = NULL;
	Fvm->NF = 0;
	Fvm->NV = 0;
	do
	{
		switch(BUFFER[0]){
			case 'v':
			if(BUFFER[1]==' ')LenVertice++;
			BUFFER[0]=32;
			break;
			case 'f':
			{
				int indices[4] = {0,0,0,0};
				int cf = sscanf(BUFFER,"%*c %i/%*i/%*i %i/%*i/%*i %i/%*i/%*i %i/%*i/%*i",indices,&indices[1],&indices[2],&indices[3]);
				if(cf == 4) LenFace++;
				LenFace++;
				BUFFER[0]=32;
			}
			break;
		}
	} while (fgets(BUFFER,512,f));
	Fvm->V = (int*)malloc( sizeof(int) * (LenVertice*3) );
	Fvm->F = (unsigned int*)malloc( sizeof(unsigned int) * (LenFace*3) );
	rewind(f);
	Fvm->NV = LenVertice;
	Fvm->NF = LenFace;
	LenVertice = LenFace = 0;
	float x,y,z;
	int a,b,c,d;
	do
	{
		switch(BUFFER[0]){
			case 'v':
			if(BUFFER[1]==' '){
				sscanf(BUFFER,"%*c %f %f %f", &x, &y, &z );
				z=(z==0?1:z);
				Fvm->V[ LenVertice++ ] = (x * Tam);
				Fvm->V[ LenVertice++ ] = -(y * Tam);
				Fvm->V[ LenVertice++ ] = (z * Tam);
			}
			break;
			case 'f':
			{
				int cF = sscanf(BUFFER,"%*c %i/%*i/%*i %i/%*i/%*i %i/%*i/%*i %i/%*i/%*i",&a, &b, &c, &d);
				Fvm->F[ LenFace++ ] = (a-1);
				Fvm->F[ LenFace++ ] = (b-1);
				Fvm->F[ LenFace++ ] = (c-1);
				if(cF == 4)
				{
					Fvm->F[ LenFace++ ] = (c-1);
					Fvm->F[ LenFace++ ] = (d-1);
					Fvm->F[ LenFace++ ] = (a-1);
				}
			}
			break;
		}
	} while (fgets(BUFFER,512,f));
	LenVertice = LenFace = 0;
	free(BUFFER);
	BUFFER=NULL;
	fclose(f);
	f=NULL;
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////


// ########################################################################
// ########################################################################
// ########################################################################
// SIMULACION DE CUBO.

#define POINT_X 300
#define POINT_Y 300
#define POINT_Z 300
int VERTICES[8*3] = {
	-POINT_X, -POINT_Y,  POINT_Z,
	 POINT_X, -POINT_Y,  POINT_Z,
	-POINT_X,  POINT_Y,  POINT_Z,
	 POINT_X,  POINT_Y,  POINT_Z,
	-POINT_X,  POINT_Y, -POINT_Z,
	 POINT_X,  POINT_Y, -POINT_Z,
	-POINT_X, -POINT_Y, -POINT_Z,
	 POINT_X, -POINT_Y, -POINT_Z
};

unsigned int FACES[12*3] = {
	0, 1, 2,
	2, 1, 3,
	2, 3, 4,
	4, 3, 5,
	4, 5, 6,
	6, 5, 7,
	6, 7, 0,
	0, 7, 1,
	1, 7, 3,
	3, 7, 5,
	6, 0, 4,
	4, 0, 2
};

// ########################################################################
// ########################################################################
// ########################################################################


// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROYECION EN PERSPECTIVA DE PUNTO FIJO.

void __fastcall Proyected( register int *src, register int *dst, unsigned int n, int fov ){
	// Code taken by 3D engine LMP88.
	int shift = 0;
	int z;
	int ffac;
	ffac = (1 << (fov + 12));
	shift = fov - 8;
	while(n--){
		z = src[2];
		// z+=(z==0);
		fov = ffac / z;
		*dst++ = ( ( src[0] * fov + (1<<11) ) >> 12 ) + DW;
		*dst++ = DH - ( ( src[1] * fov + (1<<11) ) >> 12 );
		*dst++ = fov >> shift;
		src += 3;
	}
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// STRUCTURA DE EDGES/BORDES Y DEPTHBUFFER-Z/BUFER DE PROFUNDIDAD Z.

typedef struct {
	int x;
	int z;
} EDGE_FLAT;

// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// SCANLINE/SCANEO DE LINEA.

static EDGE_FLAT DR[ SH ]; // -->
static EDGE_FLAT DL[ SH ]; // <--
static int MinY;
static int MaxY;
static void ScanLine( int x0, int y0, int z0, int x1, int y1, int z1 ){
	int X;
	register int Len;
	Len = y1 - y0;
	if (y0>y1)
		y0^=y1, y1^=y0, y0^=y1,
		x0^=x1, x1^=x0, x0^=x1,
		z0^=z1, z1^=z0, z0^=z1,
		Len = y1 - y0;
	MinY = ( y0 < MinY? y0 : MinY );
	MaxY = ( y1 > MaxY? y1 : MaxY );
	Len = y1 - y0;
	Len += (Len==0);
	x1 = ((x1-x0)<<16) / Len;
	z1 = ((z1-z0)<<16) / Len;
	x0<<=16;
	z0<<=16;
	while( Len-- ){
		X = x0>>16;
		if ( X < DL[y0].x ) DL[y0].x = X, DL[y0].z = z0>>16;
		if ( X > DR[y0].x ) DR[y0].x = X, DR[y0].z = z0>>16;
		x0+=x1;
		z0+=z1;
		y0++;
	}
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// TRIANGLEFILL/TRIANGULO RELLENO.

void __fastcall TriangleFill( register unsigned int *P, register int *B, int *buftri_, const unsigned int c ){
	register unsigned int *VID;
	register int *BUF;
	register int z0;
	int z1;
	MinY = 0;
	MaxY = SH;
	EDGE_FLAT *PL=DL;
	EDGE_FLAT *PR=DR;
	ScanLine( buftri_[0], buftri_[1], buftri_[2], buftri_[3], buftri_[4], buftri_[5] );
	ScanLine( buftri_[3], buftri_[4], buftri_[5], buftri_[6], buftri_[7], buftri_[8] );
	ScanLine( buftri_[6], buftri_[7], buftri_[8], buftri_[0], buftri_[1], buftri_[2] );
	P+=MinY;
	B+=MinY;
	PL+=MinY;
	PR+=MinY;
	MaxY-=MinY;
	while( MaxY-- )
	{
		if ( PL->x <= PR->x )
		{
			VID = P + PL->x;
			BUF = B + PL->x;
			PR->x -= PL->x;
			PR->z -= PL->z;
			PR->z<<=16;
			PR->x += (PR->x==0);
			z1 = PR->z / PR->x;
			z0 = PL->z<<16;
			while( PR->x-- ){
				*VID = (*BUF>=z0>>16?c:*VID);
				*BUF = (*BUF>=z0>>16?z0>>16:*BUF);
				VID++;
				BUF++;
				z0+=z1;
			}
		}
		PL->x = SW+1;
		PR->x = -1;
		P+=SW;
		B+=SW;
		PL++;
		PR++;
	}
}


// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// BUFFER DE COS Y SIN JUNTOS DE PUNTO FIJO.

const static unsigned short _lookupSineCosine[ 256 ] = {
	0x7FFE, 0x82FD, 0x85FD, 0x88FD, 0x8BFD, 0x8EFD, 0x91FC, 0x94FC, 0x97FB, 0x9AFA,
	0x9DFA, 0xA0F9, 0xA3F8, 0xA6F7, 0xA9F6, 0xACF5, 0xAFF4, 0xB2F3, 0xB5F1, 0xB8F0,
	0xBAEF, 0xBDED, 0xC0EB, 0xC2EA, 0xC5E8, 0xC8E6, 0xCAE5, 0xCDE3, 0xCFE1, 0xD1DF,
	0xD4DD, 0xD6DA, 0xD8D8, 0xDAD6, 0xDDD4, 0xDFD1, 0xE1CF, 0xE3CD, 0xE5CA, 0xE6C8,
	0xE8C5, 0xEAC2, 0xEBC0, 0xEDBD, 0xEFBA, 0xF0B8, 0xF1B5, 0xF3B2, 0xF4AF, 0xF5AC,
	0xF6A9, 0xF7A6, 0xF8A3, 0xF9A0, 0xFA9D, 0xFA9A, 0xFB97, 0xFC94, 0xFC91, 0xFD8E,
	0xFD8B, 0xFD88, 0xFD85, 0xFD82, 0xFD7F, 0xFD7B, 0xFD78, 0xFD75, 0xFD72, 0xFD6F,
	0xFC6C, 0xFC69, 0xFB66, 0xFA63, 0xFA60, 0xF95D, 0xF85A, 0xF757, 0xF654, 0xF551,
	0xF44E, 0xF34B, 0xF148, 0xF045, 0xEF43, 0xED40, 0xEB3D, 0xEA3B, 0xE838, 0xE635,
	0xE533, 0xE330, 0xE12E, 0xDF2C, 0xDD29, 0xDA27, 0xD825, 0xD623, 0xD420, 0xD11E,
	0xCF1C, 0xCD1A, 0xCA18, 0xC817, 0xC515, 0xC213, 0xC012, 0xBD10, 0xBA0E, 0xB80D,
	0xB50C, 0xB20A, 0xAF09, 0xAC08, 0xA907, 0xA606, 0xA305, 0xA004, 0x9D03, 0x9A03,
	0x9702, 0x9401, 0x9101, 0x8E00, 0x8B00, 0x8800, 0x8500, 0x8200, 0x7F00, 0x7B00,
	0x7800, 0x7500, 0x7200, 0x6F00, 0x6C01, 0x6901, 0x6602, 0x6303, 0x6003, 0x5D04,
	0x5A05, 0x5706, 0x5407, 0x5108, 0x4E09, 0x4B0A, 0x480C, 0x450D, 0x430E, 0x4010,
	0x3D12, 0x3B13, 0x3815, 0x3517, 0x3318, 0x301A, 0x2E1C, 0x2C1E, 0x2920, 0x2723,
	0x2525, 0x2327, 0x2029, 0x1E2C, 0x1C2E, 0x1A30, 0x1833, 0x1735, 0x1538, 0x133B,
	0x123D, 0x1040, 0x0E43, 0x0D45, 0x0C48, 0x0A4B, 0x094E, 0x0851, 0x0754, 0x0657,
	0x055A, 0x045D, 0x0360, 0x0363, 0x0266, 0x0169, 0x016C, 0x006F, 0x0072, 0x0075,
	0x0078, 0x007B, 0x007E, 0x0082, 0x0085, 0x0088, 0x008B, 0x008E, 0x0191, 0x0194,
	0x0297, 0x039A, 0x039D, 0x04A0, 0x05A3, 0x06A6, 0x07A9, 0x08AC, 0x09AF, 0x0AB2,
	0x0CB5, 0x0DB8, 0x0EBA, 0x10BD, 0x12C0, 0x13C2, 0x15C5, 0x17C8, 0x18CA, 0x1ACD,
	0x1CCF, 0x1ED1, 0x20D4, 0x23D6, 0x25D8, 0x27DA, 0x29DD, 0x2CDF, 0x2EE1, 0x30E3,
	0x33E5, 0x35E6, 0x38E8, 0x3BEA, 0x3DEB, 0x40ED, 0x43EF, 0x45F0, 0x48F1, 0x4BF3,
	0x4EF4, 0x51F5, 0x54F6, 0x57F7, 0x5AF8, 0x5DF9, 0x60FA, 0x63FA, 0x66FB, 0x69FC,
	0x6CFC, 0x6FFD, 0x72FD, 0x75FD, 0x78FD, 0x7BFD
};

const short __fastcall tdSIN( int iv ){ if( iv > 254 ) return -_lookupSineCosine[iv-255]; else return _lookupSineCosine[iv]; }
const short __fastcall tdCOS( int iv ){ return tdSIN( iv + 128 ); }

// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// ROTACIONES 3D DE PUNTO FIJO.

void __fastcall MallaRotarX( register int *_3d, const int theta, register unsigned int len )
{
	//Rotacion X
    int SIN,COS;
    SIN = tdSIN( theta );
    COS = tdCOS( theta );
    while(len--){
		_3d[0] = (_3d[0]);
		_3d[1] = (_3d[1] * COS - _3d[2] * SIN)>>15;
		_3d[2] = (_3d[2] * COS + _3d[1] * SIN)>>15;
    	_3d+=3;
    }
}

void __fastcall MallaRotarY( register int *_3d, const int theta, register unsigned int len )
{
	// Rotacion Y
    int SIN,COS;
    SIN = tdSIN( theta );
    COS = tdCOS( theta );
    while(len--){
        _3d[0] = (_3d[0] * COS + _3d[2] * SIN)>>15;
        _3d[1] = (_3d[1]);
        _3d[2] = (_3d[2] * COS - _3d[0] * SIN)>>15;
    	_3d+=3;
    }
}

void __fastcall MallaRotarZ( register int *_3d, const int theta, register unsigned int len )
{
	// Rotacion Z
    int SIN,COS;
    SIN = tdSIN( theta );
    COS = tdCOS( theta );
    while(len--){
		_3d[0] = (_3d[0] * COS + _3d[1] * SIN)>>15;
		_3d[1] = (_3d[1] * COS - _3d[0] * SIN)>>15;
		_3d[2] = (_3d[2]);
		_3d+=3;
	}
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////



// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROGRAMA 3D.

int main(int argc, char const *argv[])
{
	for ( unsigned int i = 0; i < SH; ++i ) DL[i].x = SW+1, DR[i].x = -1;
	MinY = 0;
	MaxY = SH;
	__FVM__ MALLA3D = {0,0,NULL,NULL};
	// Si Extra.Fvm no existe puede activar esta linea y generara el cubo.
	/*{
		MALLA3D.V = VERTICES;
		MALLA3D.F = FACES;
		MALLA3D.NF = 12;
		MALLA3D.NV = 8;
		GenerateFvm( &MALLA3D, "Extra.Fvm" );
	}*/
	// Si Extra.Fvm no existe puede activar esta linea y generara el cubo.
	{
		if (argc<2)
		{
			LoadFvm( &MALLA3D, "Extra.Fvm" );
		} else {
			GenerateFvmToObj( &MALLA3D, argv[1], 101 );
			GenerateFvm( &MALLA3D, "Extra.Fvm" );
		}
	}
	unsigned int *P = (unsigned int*)malloc( (SW * SH) * sizeof(unsigned int) );
	int *B = (int*)malloc( (SW * SH) * sizeof(int) );
	memset( P, 0, (SW * SH) * sizeof(unsigned int) );
	memset( B, 0, (SW * SH) * sizeof(int) );
	unsigned int *FACE = NULL;
	int          *VERT = NULL;
	int *V = (int*)malloc( sizeof(int) * (MALLA3D.NV*3) );
	MallaRotarX( MALLA3D.V, 10, MALLA3D.NV );
	for ( unsigned int i = 0; i < (MALLA3D.NV*3); i+=3) {
		// MALLA3D.V[i+0] -= 180;
		MALLA3D.V[i+1] += 350;
		MALLA3D.V[i+2] -= 600;
	}
	Proyected( MALLA3D.V, V, MALLA3D.NV, 9 );
	FACE = MALLA3D.F;
	VERT = V;
	unsigned int Z = MALLA3D.NF;
	unsigned int TL;
	unsigned int Step;
	int buftri_[ 9 ];
	unsigned int COLOR[4] = {
		0xff00ff00,
		0xff00ffff,
		0xffff00ff,
		0xffffff00
	};
	printf("3D PREPARED...\n");
	while ( Z-- )
	{
		TL = 9;
		while( TL )
			Step = ( *FACE * 3 ),
			buftri_[ 9 - (TL--) ] = VERT[ Step + 0 ],
			buftri_[ 9 - (TL--) ] = VERT[ Step + 1 ],
			buftri_[ 9 - (TL--) ] = VERT[ Step + 2 ],
			FACE++;
		TriangleFill( P, B, buftri_, COLOR[ Z % 4 ] );
	}
	printf("3D FIN...\n");
	free(V),V=NULL;
	free(B),B=NULL;
	SaveBMPConverter( "Screen.bmp", P, SW, SH, 32 );
	free(P),P=NULL;
	return 0;
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////
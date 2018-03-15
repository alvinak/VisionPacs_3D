#include "stdafx.h"

#include "HsBaseImg.h"
#include "HsFile.h"

//EBMSopInstUID:	1.2.392.200046.100.2.1.17175936433.120807152841.1.1.1.1
extern void** ArrayNew(unsigned long nRow, unsigned long nCol, unsigned long nSize,unsigned long *nNewRow=NULL, unsigned long* nNewCol=NULL);
extern void ArrayFree(void **pArr,int iflag=0);
extern void** ArrayCopy(void** pSrcArr,unsigned long nSrcRow, unsigned long nSrcCol, unsigned long nSrcSize);
extern int GetFormatType(const QString &sFormat);
extern RECT GetShowRcByImgSize(RECT rc, double ImgWidth, double ImgHeight);
extern void DrawRuler(HDC dc,int nWidth,RECT WndRc,double fPixSpacing,double fMag,char cFx);
//bool Convert(long &x,long &y,long w,long h,bool HorFlip,int nAngle);

float SinPiX(const float &x);
float Matrix_Bicubic(const float (&A)[4], const float (&B)[4][4], const float (&C)[4]);

float SinPiX(const float &x)
{
	float absx = (x>=0.00) ? x:-x;

	if(absx<1)
		return (1 + absx*absx*(-2 + absx));//(1 - 2*absx*absx + absx*absx*absx);
	else if(absx<2)
		return (4+absx*(-8 + 5*absx - absx*absx));//(4 - 8*absx + 5*absx*absx - absx*absx*absx);
	else
		return 0;

	//const float a = -1; //a������ȡ a=-2,-1,-0.75,-0.5�ȵȣ��𵽵����񻯻�ģ���̶ȵ�����

	//float absx = (x>=0.00) ? x:-x;
	//double x2=absx*absx;
	//double x3=x2*absx;
	//if (absx<=1)
	//	return (a+2)*x3 - (a+3)*x2 + 1;
	//else if (absx<=2) 
	//	return a*x3 - (5*a)*x2 + (8*a)*absx - (4*a);
	//else
	//	return 0;

}
float Matrix_Bicubic(const float (&A)[4], const float (&B)[4][4], const float (&C)[4])
{
	float m[4]={0,0,0,0};
	float re = 0.00;

	for (BYTE col=0; col<4; col++)
	{
		for ( int row=0; row<4; row++)
			m[col] = m[col]+A[row]*B[row][col]; 
	}

	for (BYTE j=0;j<4;j++)
		re = re + m[j]*C[j];

	return re;
}

CHsBaseImg::CHsBaseImg(void)
:m_pOriData(NULL)
,m_pDisplayData(NULL)
,m_pWcLut(0)
,m_pOriOverlayData(NULL)
,m_pDisplayOverlayData(NULL)
,m_pBits(NULL)
,m_hWnd(NULL)
,m_pPreSizeRc(NULL)
,m_bSharpOri(false)
,m_bMpr(false)
,m_sImgMprMode("MIP")
,m_bOriPixelRepresentation(false)
{
	m_ImgState.nDispalyRow = 0;
	m_ImgState.nDispalyCol = 0;
}

CHsBaseImg::~CHsBaseImg(void)
{
	Hs_FreeMem();

    if (m_pPreSizeRc)
		delete m_pPreSizeRc;
    m_pPreSizeRc = NULL;
}

int CHsBaseImg::Hs_FreeMem()
{
	if(m_pOriData)
		ArrayFree((void **)m_pOriData);
	m_pOriData = NULL;

	if(m_pWcLut)
		delete []m_pWcLut;
	m_pWcLut = NULL;

	if (m_pOriOverlayData)
		ArrayFree((void **)m_pOriOverlayData);
	m_pOriOverlayData = NULL;

	
	if (m_pDisplayOverlayData)
		ArrayFree((void **)m_pDisplayOverlayData);
	m_pDisplayOverlayData = NULL;

	if(m_pBits)
		delete []m_pBits;
	m_pBits = NULL;

	if(m_pDisplayData)
		ArrayFree((void **)m_pDisplayData,0);
	m_pDisplayData = NULL;

	int n = m_pLutV.size();
	for (int i = 0; i<n; i++)
		delete m_pLutV[i];
	m_pLutV.clear();


	if(m_pPreSizeRc)
		delete m_pPreSizeRc;

	m_PreSize.cx = m_PreSize.cy = -1;
	m_pPreSizeRc = NULL;
	m_nPreSizeType = 0;


	return Ret_Success;
}
int CHsBaseImg::Hs_WinLevel(long w,long c,bool bChangeValue,long *pRetW,long *pRetC)
{
	if (bChangeValue)
	{
        w = m_ImgState.CurWc.x + w*m_ImgInfo.fWinLevelStep;
        c = m_ImgState.CurWc.y + c*m_ImgInfo.fWinLevelStep;
	}

	int nRet = Ret_Success;

	bool bPaletteColor = int(m_ImgInfo.sPhotometric.indexOf("PALETTE COLOR"))>=0 ? true:false;
	if(bPaletteColor)
	{
		nRet = UpdateBits(m_ImgState.nDispalyRow,m_ImgState.nDispalyCol,m_pDisplayData,m_pDisplayOverlayData,m_pBits);
	}
	else
	{
		nRet = WinLevelNormal( w, c);
	}

	if (pRetW)
        *pRetW = m_ImgState.CurWc.x;

	if (pRetC)
        *pRetC = m_ImgState.CurWc.y;

	//m_bDrawInfoFilled = false;
	//m_bPrintInfoFilled = false;

	return nRet;
}

int CHsBaseImg::WinLevelNormal(long w,long c,bool bUpdateBits)
{//�����͸ı�WcLut

	//m_bDrawInfoFilled = false;//��ʾ��Ϣ�б仯
	//m_bPrintInfoFilled = false;

	if (w==0 && c==0)
	{
		w = m_ImgInfo.fWinWidth;
		c = m_ImgInfo.fWinCenter;
	}

	if(w<1) w = 1;
	if(w>m_ImgInfo.nWcLutLen)	w = m_ImgInfo.nWcLutLen;
	//if(c>m_ImgInfo.iLutEnd)	c = m_ImgInfo.iLutEnd ;
	//if(c<m_ImgInfo.iLutStart)	c = m_ImgInfo.iLutStart;


    m_ImgState.CurWc.x = w;
    m_ImgState.CurWc.y = c;


	//if(m_ImgInfo.fRescaleSlope!=0.00 && m_ImgInfo.fRescaleIntercept!=0.00 && m_ImgState.bUseSlope)
	//{//�����б�ʽؾ�,��ô:
	//	//1.��׼��Ϊ��ʱ���ǵ�����ֵ�Ѿ���б�ʼӹ�����.
	//	//2.�˴�������wcҲ��б�ʼӹ�����.
	//	//3.���ǵ�WcLut(0��nWcLutLen)���±�Ҳ��Ӧ����б�ʼӹ�,���(0*fRescaleSlope+fRescaleIntercept �� (nWcLutLen*fRescaleSlope+fRescaleIntercept)
	//	//����,�Ҳ�û�мӹ�������,Ҳ������ӹ�Lut���±�.����,�˴�������wcҪ�ָ��ӹ�ǰ��ԭ��.

	//	c = long((c-m_ImgInfo.fRescaleIntercept)/m_ImgInfo.fRescaleSlope);
	//	w = long(w/m_ImgInfo.fRescaleSlope);
	//}


	if(m_pWcLut == NULL)
		m_pWcLut = new long[m_ImgInfo.nWcLutLen];


	long *pLut = m_pWcLut;
	long iStart = m_ImgInfo.iLutStart;
	long iEnd = m_ImgInfo.iLutEnd;

	if (m_ImgInfo.nPixelRepresentation == 1)//��λ����(����ֵ���и���)
	{
		if (m_bOriPixelRepresentation == true)
		{
			pLut = &(m_pWcLut[-long(m_ImgInfo.fRescaleIntercept)]);//���ø��±�
			iStart = long(m_ImgInfo.fRescaleIntercept);
			iEnd = long(m_ImgInfo.nWcLutLen*m_ImgInfo.fRescaleSlope + m_ImgInfo.fRescaleIntercept);
		}
		else
			pLut = &(m_pWcLut[m_ImgInfo.nWcLutLen / 2]);//���ø��±�
	}
		

	int nMin = c - w/2;
	int nMax = c + w/2;

	//AtlTrace("\r\n%d,%d",nMin,nMax);//-1399  , -386

	//nMin = max(nMin,iStart);
	//nMax = min(nMax,iEnd);

	bool b1 = LONG(&(m_pWcLut[0])) == LONG(&(pLut[iStart])) ? true:false;
	bool b2 = LONG(&(m_pWcLut[m_ImgInfo.nWcLutLen-1])) == LONG(&(pLut[iEnd-1])) ? true:false;
	if(b1==false)
	{
		qDebug("\r\n���鸺�±����!");
		delete []m_pWcLut;
		m_pWcLut = NULL;
		return 0;
	}


	double nPerGray = 255.00/w;

	for (long i=iStart;i<iEnd;i++)
	{
		if (i<nMin)
		{
			pLut[i] = 0;
		}
		else if (i>nMax)
		{
			pLut[i] = 255;
		}
		else
		{
			pLut[i] = int((i-nMin)*nPerGray);//pLut[i]= (int)(255*(i-nMin)/w+0.5);
		}

		//pLut[i] = int((i-nMin)*nPerGray);
		//if(pLut[i]<0) 
		//	pLut[i]=0;
		//if(pLut[i]>255) 
		//	pLut[i]=255;
		//AtlTrace("\r\npLut[%d] = %d",i,pLut[i]);
	}


	//TRACE("\r\nLut: %d   %d",iStart,iEnd);

	if(bUpdateBits)
		UpdateBits(m_ImgState.nDispalyRow,m_ImgState.nDispalyCol,m_pDisplayData,m_pDisplayOverlayData,m_pBits);

	return Ret_Success;
}

int CHsBaseImg::UpdateBits(unsigned long nRow,unsigned long nCol,BYTE** pDisplayData,BYTE** pDisplayOverlayData,BYTE* &pBits,RECT* pValidRc)
{//����·,һ·����m_Displayϵ������,һ·�����������������.����m_pDisplayϵ�е������漰��"����ÿ�ζ����·����ڴ�"����--��ϣ������ж�������
	if (m_pOriData==NULL)
		return 0;

	bool bDisplayData = false;//����m_pDisplayData������������

	if (pDisplayData==m_pDisplayData  && pDisplayOverlayData==m_pDisplayOverlayData && pBits==m_pBits)
		bDisplayData = true;

	nCol = ((nCol*8)+31)/32*4;
	if(bDisplayData==true)
	{
		if(m_pDisplayData==NULL)
		{
			m_pDisplayData = (BYTE**)ArrayCopy((void**)m_pOriData,m_ImgInfo.nRows,m_ImgInfo.nCols,m_ImgInfo.nBitsAllocated*m_ImgInfo.nSamplePerPixel/8);
			pDisplayData = m_pDisplayData;
		}

		//����ÿ�ζ����·����ڴ�.�������������ֻ���޸��ڴ��е�ֵ����
		if (m_ImgState.nCurBitsCol !=nCol || m_ImgState.nCurBitsRow!=nRow)
		{
			if (m_pBits)
				delete []m_pBits;
			m_pBits = NULL;

			m_ImgState.nCurBitsCol = nCol;
			m_ImgState.nCurBitsRow = nRow;
		}

		if(m_pBits==NULL)
		{
			if (m_ImgInfo.bGrayImg)
				m_pBits = new BYTE[nRow*nCol];
			else
				m_pBits = new BYTE[nRow*nCol*sizeof(COLORREF)];
		}
	}
	else
	{
		if (m_ImgInfo.bGrayImg)
			pBits = new BYTE[nRow*nCol];
		else
			pBits = new BYTE[nRow*nCol*sizeof(COLORREF)];
	}


	bool bPaletteColor = int(m_ImgInfo.sPhotometric.indexOf("PALETTE COLOR"))>=0 ? true:false;
	if(bPaletteColor)
		return UpdateBitsPaletteColor( nRow, nCol,pDisplayData,pDisplayOverlayData,pBits);
	else
		return UpdateBitsNormal( nRow, nCol,pDisplayData,pDisplayOverlayData,pBits,pValidRc);
}

int CHsBaseImg::UpdateBitsPaletteColor(unsigned long nRow,unsigned long nCol,BYTE** pDisplayData,BYTE** pDisplayOverlayData,BYTE* pBits)
{

	//ͨ����ʱָ�����pLutָ���λ�ò�ͬ,��ʵ�ָ��±�
	long *pLut = m_pWcLut;
	if (m_ImgInfo.nPixelRepresentation==1)//��λ����(����ֵ���и���)
		pLut = &(m_pWcLut[m_ImgInfo.nWcLutLen/2]);//��˿��ø��±�

	bool bUseSlope = false;
	if(m_ImgInfo.fRescaleIntercept!=0.00 && m_ImgInfo.fRescaleSlope!=0.00)
		bUseSlope = true;

	//׼��Ӧ��n��Lut.Ϊ�˼���ѭ��.�˴�����ҪӦ�õ�Lut�����ó�,�浽��ʱ����vector��
	int nLut = m_pLutV.size();
	QVector<LutData*> pLutUsed;

	for(int k=0;k<nLut;k++)
	{
		if(m_pLutV[k]->bUse && m_pLutV[k]->bWc==false)//��Ҫ���ò�����Lut
			pLutUsed.push_back(m_pLutV[k]);
	}


	if (m_ImgInfo.nBitsPerPixel==8)//m_pDisplayDataһ���ֽڱ�ʾһ������
	{
		COLORREF *tBits = (COLORREF*)pBits;
		BYTE **tDisplayData = (BYTE **)pDisplayData;

        BYTE *pR = GetLutByName("R")->pLutData;
		BYTE *pG = GetLutByName("G")->pLutData;
		BYTE *pB = GetLutByName("B")->pLutData;

		COLORREF WhitePixValue = RGB(255,255,255);

		int nLut = pLutUsed.size();
		unsigned long i = 0;
		for (long r=nRow-1;r>=0;r--)
		{
			for (long c=0;c<nCol;c++)
			{
				if (pDisplayOverlayData && m_ImgState.bShowOverLay)
				{
					if(pDisplayOverlayData[r][c])
					{
						tBits[i] = WhitePixValue; 

						i++;
						continue;
					}
				}


				BYTE v = tDisplayData[r][c];

				if(m_ImgState.bInversed)
					tBits[i] = RGB(255 - pB[v],	255 - pG[v],	255 - pG[v]);
				else
					tBits[i] = RGB(pB[v],	pG[v],  pR[v]);

				i++;
			}

		}

	}
	else if(m_ImgInfo.nBitsPerPixel==16)//Ҫ��m_pDisplayData�������ֽڱ�ʾһ������
	{
		COLORREF *tBits = (COLORREF*)pBits;
		unsigned short **tDisplayData = (unsigned short **)pDisplayData;

		BYTE *pR = GetLutByName("R")->pLutData;
		BYTE *pG = GetLutByName("G")->pLutData;
		BYTE *pB = GetLutByName("B")->pLutData;

		COLORREF WhitePixValue = RGB(255,255,255);

		int nLut = pLutUsed.size();
		unsigned long i = 0;
		for (long r=nRow-1;r>=0;r--)
		{
			for (long c=0;c<nCol;c++)
			{
				if (pDisplayOverlayData && m_ImgState.bShowOverLay)
				{
					if(pDisplayOverlayData[r][c])
					{
						tBits[i] = WhitePixValue; 

						i++;
						continue;
					}
				}


				unsigned short v = tDisplayData[r][c];

				if(m_ImgState.bInversed)
					tBits[i] = RGB(255 - pB[v],	255 - pG[v],	255 - pG[v]);
				else
					tBits[i] = RGB(pB[v],	pG[v],  pR[v]);

				i++;
			}

		}
	}
	else 
	{
//		::MessageBox(0,"�����","UpdateBitsPaletteColor",0);
	}

	return Ret_Success;
}

int CHsBaseImg::UpdateBitsNormal(unsigned long nRow,unsigned long nCol,BYTE** pDisplayData,BYTE** pDisplayOverlayData,BYTE* pBits,RECT* pValidRc)
{

	//ͨ����ʱָ�����pLutָ���λ�ò�ͬ,��ʵ�ָ��±�
	long *pLut = m_pWcLut;
	if (m_ImgInfo.nPixelRepresentation == 1)//��λ����(����ֵ���и���)
	{
		if (m_bOriPixelRepresentation == true)
		{
			pLut = &(m_pWcLut[-long(m_ImgInfo.fRescaleIntercept)]);//���ø��±�
		}
		else
			pLut = &(m_pWcLut[m_ImgInfo.nWcLutLen / 2]);//���ø��±�
	}

	bool bUseSlope = false;
	if(m_ImgInfo.fRescaleIntercept!=0.00 && m_ImgInfo.fRescaleSlope!=0.00)
		bUseSlope = true;

	//׼��Ӧ��n��Lut.Ϊ�˼���ѭ��.�˴�����ҪӦ�õ�Lut�����ó�,�浽��ʱ����vector��
	int nLut = m_pLutV.size();
	QVector<LutData*> pLutUsed;

	for(int k=0;k<nLut;k++)
	{
		if(m_pLutV[k]->bUse && m_pLutV[k]->bWc==false)//��Ҫ���ò�����Lut
			pLutUsed.push_back(m_pLutV[k]);
	}


	if (m_ImgInfo.nSamplePerPixel==1)
	{
		if (m_ImgInfo.nBitsPerPixel==8)//m_pDisplayDataһ���ֽڱ�ʾһ������
		{
			if (m_ImgInfo.nPixelRepresentation==1)
				FillBits((char**)pDisplayData,pBits,pDisplayOverlayData,nRow,nCol,m_ImgState.bShowOverLay,m_ImgState.bInversed,pLutUsed,pLut,pValidRc);
			else
				FillBits(( unsigned char**)pDisplayData,pBits,pDisplayOverlayData,nRow,nCol,m_ImgState.bShowOverLay,m_ImgState.bInversed,pLutUsed,pLut,pValidRc);
		}
		else if(m_ImgInfo.nBitsPerPixel==16)//Ҫ��m_pDisplayData�������ֽڱ�ʾһ������
		{
			if (m_ImgInfo.nPixelRepresentation==1)
				FillBits((short**)pDisplayData,pBits,pDisplayOverlayData,nRow,nCol,m_ImgState.bShowOverLay,m_ImgState.bInversed,pLutUsed,pLut,pValidRc);
			else
				FillBits((unsigned short**)pDisplayData,pBits,pDisplayOverlayData,nRow,nCol,m_ImgState.bShowOverLay,m_ImgState.bInversed,pLutUsed,pLut,pValidRc);
		}
		else if (m_ImgInfo.nBitsPerPixel==32)
		{
			if (m_ImgInfo.nPixelRepresentation==1)
				FillBits((long**)pDisplayData,pBits,pDisplayOverlayData,nRow,nCol,m_ImgState.bShowOverLay,m_ImgState.bInversed,pLutUsed,pLut,pValidRc);
			else
				FillBits((unsigned long**)pDisplayData,pBits,pDisplayOverlayData,nRow,nCol,m_ImgState.bShowOverLay,m_ImgState.bInversed,pLutUsed,pLut,pValidRc);
		}
	}
	else if (m_ImgInfo.nSamplePerPixel==3)
	{
		if (m_ImgInfo.nBitsPerPixel==24)
		{
			COLORREF *tBits = (COLORREF*)pBits;
			MYDATA24 **tDisplayData = (MYDATA24 **)pDisplayData;

			COLORREF WhitePixValue = RGB(255,255,255);


			unsigned long i = 0;
			for (long r=nRow-1;r>=0;r--)
			{
				for (long c=0;c<nCol;c++)
				{
					if (pDisplayOverlayData && m_ImgState.bShowOverLay)
					{
						if(pDisplayOverlayData[r][c])
						{
							tBits[i] = WhitePixValue; 

							i++;
							continue;
						}
					}

					MYDATA24 v = tDisplayData[r][c];

					if(m_ImgState.bInversed)
						tBits[i] = RGB(255 - pLut[v.pData[2]],	255 - pLut[v.pData[1]],	255 - pLut[v.pData[0]]);
					else
						tBits[i] = RGB(pLut[v.pData[2]],	pLut[v.pData[1]],	pLut[v.pData[0]]);

					i++;
				}

			}

		}
	}

	return Ret_Success;
}

int CHsBaseImg::Hs_DrawImg(HDC dc,const RECT& rc,BYTE *pBits,unsigned long nRow,unsigned long nCol)
{
	if (pBits==NULL)//��ʾĬ����ʾm_pBits
	{
		if(m_pDisplayData==NULL)
		{
			Hs_Size(NULL,rc.right-rc.left,  rc.bottom-rc.top,HSIZE_RESAMPLE,true);
		}
		
		pBits = m_pBits;
		nRow = m_ImgState.nDispalyRow;
		nCol = m_ImgState.nDispalyCol;
	}

	if(pBits==NULL)
		return 0;


	//if (hdd)
	//{
	//	long tCol = ((nRow*8)+31)/32*4;
	//	long nPix = nRow*tCol;
	//
	//
	//	BITMAPINFOHEADER bmi;
	//	ZeroMemory(&bmi, sizeof(bmi)/sizeof(char));
	//	bmi.biSize = sizeof(bmi)/sizeof(char);
	//	bmi.biBitCount = 8;//32;
	//	bmi.biClrImportant = 0;
	//	bmi.biHeight = nRow;
	//	bmi.biPlanes = 1;
	//	bmi.biCompression = BI_RGB;
	//	bmi.biWidth = tCol;
	//	bmi.biSizeImage = bmi.biWidth * bmi.biHeight;//*4;
	//
	//	BOOL r = DrawDibBegin(hdd, dc, 0, 0, &bmi, bmi.biWidth, bmi.biHeight, DDF_SAME_HDC);
	//	UINT u = DrawDibRealize(hdd, dc, TRUE);
	//	//r = DrawDibDraw(hdd, dc, 0, 0,bmi.biWidth, bmi.biHeight, &bmi, m_pBits, 0, 0, bmi.biWidth, bmi.biHeight, NULL);
	//	r = DrawDibDraw(hdd, dc,rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top , &bmi, pBits, 0, 0,tCol, m_ImgInfo.nRows, NULL);
	//
	//	return 0;
	//}


	//if (0)
	//{
	//	long tCol = ((nCol*8)+31)/32*4;
	//	long nPix = nRow*tCol;

	//	BITMAPINFOHEADER bmi;
	//	ZeroMemory(&bmi, sizeof(bmi)/sizeof(char));
	//	bmi.biSize = sizeof(bmi)/sizeof(char);
	//	bmi.biBitCount = 8;
	//	bmi.biClrImportant = 0;
	//	bmi.biHeight = nRow;
	//	bmi.biPlanes = 1;
	//	bmi.biCompression = BI_RGB;
	//	bmi.biWidth = tCol;
	//	bmi.biSizeImage = bmi.biWidth * bmi.biHeight;

	//	RGBQUAD pColor[256];
	//	for(int i=0;i<256;i++)
	//	{
	//		pColor[i].rgbBlue = pColor[i].rgbRed = pColor[i].rgbGreen = i;
	//		pColor[i].rgbReserved = 0;
	//	}

	//	int nSize = sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD);
	//	BYTE *pInfo = new BYTE[nSize];
	//	memcpy(pInfo,&bmi,sizeof(BITMAPINFOHEADER));
	//	memcpy(pInfo+sizeof(BITMAPINFOHEADER),pColor,256*sizeof(RGBQUAD));

	//	::SetStretchBltMode(dc,COLORONCOLOR);
	//	int re = StretchDIBits(dc,rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 0, 0, bmi.biWidth, bmi.biHeight, pBits, (BITMAPINFO*)pInfo, DIB_RGB_COLORS,SRCCOPY);

	//	delete []pInfo;
	//}

	if (m_ImgInfo.bGrayImg)
	{
		long tCol = ((nCol*8)+31)/32*4;
		long nPix = nRow*tCol;

		BYTE bt[1064];
		memset(bt,0,1064);

		BITMAPINFO* pInfo = (BITMAPINFO*)bt;

		pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pInfo->bmiHeader.biBitCount = 8;
		pInfo->bmiHeader.biClrImportant = 0;
		pInfo->bmiHeader.biHeight = nRow;
		pInfo->bmiHeader.biPlanes = 1;
		pInfo->bmiHeader.biCompression = BI_RGB;
		pInfo->bmiHeader.biWidth = tCol;
		pInfo->bmiHeader.biSizeImage = pInfo->bmiHeader.biWidth * pInfo->bmiHeader.biHeight;

		for(int i=0;i<256;i++)
		{
			pInfo->bmiColors[i].rgbBlue = pInfo->bmiColors[i].rgbRed = pInfo->bmiColors[i].rgbGreen = i;
			pInfo->bmiColors[i].rgbReserved = 0;
		}

		//SetDIBitsToDevice(dc,rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,0, 0, 0 , pInfo->bmiHeader.biHeight,pBits,(BITMAPINFO*)pInfo,DIB_RGB_COLORS);
		::SetStretchBltMode(dc,COLORONCOLOR);
		//if(tCol == rc.right-rc.left && nRow == rc.bottom-rc.top)
			//��ô�����StretchDIBits�����Ļ����ٶȽ��Ǿ��˵�.������û��Ƶ����ˣ�������취���������
		//AtlTrace("\r\nIm = %d,%d\r\nRc = %d %d\r\n",tCol,nRow,rc.right-rc.left, rc.bottom-rc.top);		

		int re = StretchDIBits(dc,rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 0, 0, pInfo->bmiHeader.biWidth , pInfo->bmiHeader.biHeight, pBits, (BITMAPINFO*)pInfo, DIB_RGB_COLORS,SRCCOPY);

	}
	else
	{
		long tCol = ((nCol*8)+31)/32*4;
		long nPix = nRow*tCol;

		BYTE bt[40];
		memset(bt,0,40);

		BITMAPINFO* pInfo = (BITMAPINFO*)bt;

		pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pInfo->bmiHeader.biBitCount = 32;
		pInfo->bmiHeader.biClrImportant = 0;
		pInfo->bmiHeader.biHeight = nRow;
		pInfo->bmiHeader.biPlanes = 1;
		pInfo->bmiHeader.biCompression = BI_RGB;
		pInfo->bmiHeader.biWidth = tCol;
		pInfo->bmiHeader.biSizeImage = pInfo->bmiHeader.biWidth * pInfo->bmiHeader.biHeight*sizeof(COLORREF);


		::SetStretchBltMode(dc,COLORONCOLOR);
		int re = StretchDIBits(dc,rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 0, 0, pInfo->bmiHeader.biWidth , pInfo->bmiHeader.biHeight, (COLORREF*)pBits, (BITMAPINFO*)pInfo, DIB_RGB_COLORS,SRCCOPY);
	}

	////���㵱ǰ��ʾ��Ϣ
	//FillInfo(InfoType_Draw);

	return 0;
}
int CHsBaseImg::Hs_ApplyOverlay(bool bShow)
{
	if (m_pOriOverlayData==NULL)
		return 0;

	m_ImgState.bShowOverLay = bShow;
	UpdateBits(m_ImgState.nDispalyRow,m_ImgState.nDispalyCol,m_pDisplayData,m_pDisplayOverlayData,m_pBits);

	return 0;
}

int CHsBaseImg::Hs_ApplySlope(bool bUse)
{
	m_ImgState.bUseSlope = bUse;
    WinLevelNormal(m_ImgState.CurWc.x,m_ImgState.CurWc.y);
	return 0;
}

//void CHsImage::GetPixValueFromLut(short &nPixValue,LutData *pLut)
//{
//	if(pLut==NULL)
//		return ;
//
//	if (pLut->nBytePerData==1)
//	{
//		if(nPixValue<=pLut->nMinValue)
//		{
//			nPixValue = pLut->pLutData[pLut->nMinValue];
//			return ;
//		}
//		if(nPixValue>=pLut->nMaxValue)
//		{
//			nPixValue = pLut->pLutData[pLut->nMaxValue];
//			return ;
//		}
//
//		nPixValue = pLut->pLutData[nPixValue];
//
//		return ;
//	}
//
//	if (pLut->nBytePerData==2)
//	{
//		short *pData = (short*)(pLut->pLutData);
//		if(nPixValue<=pLut->nMinValue)
//		{
//			nPixValue = pData[pLut->nMinValue];
//			return ;
//		}
//		if(nPixValue>=pLut->nMaxValue)
//		{
//			nPixValue = pData[pLut->nMaxValue];
//			return ;
//		}
//
//		nPixValue = pData[nPixValue];
//
//		return ;
//	}
//
//}

LutData* CHsBaseImg::Hs_ApplyLut(QString sName)
{
    size_t n = m_pLutV.size();

	m_ImgState.sCurLutName = "";

	LutData *pSelect = NULL;
	for (size_t i=0;i<n;i++)
	{
        if((int)(m_pLutV[i]->sName.indexOf(sName) )>=0)
		{
			m_pLutV[i]->bUse = true;
			pSelect = m_pLutV[i];	
			m_ImgState.sCurLutName = pSelect->sName;
		}
		else
		{
			m_pLutV[i]->bUse = false;
		}
	}

	if(pSelect==NULL)
	{
		WinLevelNormal(m_ImgInfo.fWinWidth,m_ImgInfo.fWinCenter);
		return NULL;
	}

	int nRet = 0;
	if(pSelect->bWc==false)
	{
		int tMax = 1 << m_ImgInfo.nBitStored;
		int tMin = 0;
		if (m_ImgInfo.fRescaleSlope!=0.00 && m_ImgInfo.fRescaleIntercept!=0.00)
		{
			tMax = tMax*m_ImgInfo.fRescaleSlope + m_ImgInfo.fRescaleIntercept;
			tMin = tMin*m_ImgInfo.fRescaleSlope + m_ImgInfo.fRescaleIntercept;
		}

		double fWinCenter = (tMin+tMax)*1.00/2;
		double fWinWidth = tMax - tMin;

		WinLevelNormal(fWinWidth,fWinCenter);
	}
	else
	{
		WinLevelNormal(pSelect->nW,pSelect->nC);
	}

	return pSelect;
}



int CHsBaseImg::ImgSize(BYTE ** pRetPixData,BYTE ** pRetOverlayData,RECT *pSrcRc, unsigned long nWidth, unsigned long nHeight, short nType)
{
	unsigned long nBytePerPix = (m_ImgInfo.nBitsAllocated*m_ImgInfo.nSamplePerPixel)/8;

	//��m_pOriData���ĸ���
	long x0 = pSrcRc->left;
	long x1 = pSrcRc->right;

	long y0 = pSrcRc->top;
	long y1 = pSrcRc->bottom;

	unsigned long w = pSrcRc->right - pSrcRc->left ;
	unsigned long h = pSrcRc->bottom - pSrcRc->top; 

	float fZoomX = w*1.00/nWidth;
	float fZoomY = h*1.00/nHeight;

	//AtlTrace("\r\nSize:	%f,  %f",nWidth*1.00/w,nHeight*1.00/h);

	//��ʼ
	if (m_ImgInfo.nSamplePerPixel==1)
	{
		if (nType==HSIZE_NORMAL)//�������.�ô�����.windowsֱ����ʾ��ʱ��͸�����,Ч��һ��
		{
			if (nBytePerPix==1)
				ImgSizeOneSampleNormal((BYTE **)pRetPixData,nHeight,nWidth,(BYTE **)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0);
			else if (nBytePerPix==2)
			{
				if(m_ImgInfo.nPixelRepresentation==1)
					ImgSizeOneSampleNormal((short **)pRetPixData,nHeight,nWidth,(short **)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0);
				else
					ImgSizeOneSampleNormal((unsigned short **)pRetPixData,nHeight,nWidth,(unsigned short **)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0);
			}
			else if (nBytePerPix==4)
			{
				if(m_ImgInfo.nPixelRepresentation==1)
					ImgSizeOneSampleNormal((long **)pRetPixData,nHeight,nWidth,(long **)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0);
				else
					ImgSizeOneSampleNormal((unsigned long **)pRetPixData,nHeight,nWidth,(unsigned long **)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0);
			}

			//OverLay
			if (m_pOriOverlayData)
				ImgSizeOneSampleNormal((BYTE **)pRetOverlayData,nHeight,nWidth,(BYTE **)m_pOriOverlayData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0);
		}

		if (nType==HSIZE_RESAMPLE)//˫���Բ�ֵ
		{//��ֵ*(�˺�-ǰ��)+ǰֵ*(���-�˺�)
			if (nBytePerPix==1)
			{
				ImgSizeOneSampleResample((BYTE**)pRetPixData,nHeight,nWidth,(BYTE**)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,x1,y1);
			}
			else if (nBytePerPix==2)
			{
				if(m_ImgInfo.nPixelRepresentation==1)
					ImgSizeOneSampleResample((short **)pRetPixData,nHeight,nWidth,(short**)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,x1,y1);
				else
					ImgSizeOneSampleResample((unsigned short **)pRetPixData,nHeight,nWidth,(unsigned short**)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,x1,y1);
			}
			else if (nBytePerPix==4)
			{
				if(m_ImgInfo.nPixelRepresentation==1)
					ImgSizeOneSampleResample((long **)pRetPixData,nHeight,nWidth,(long**)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,x1,y1);
				else
					ImgSizeOneSampleResample((unsigned long **)pRetPixData,nHeight,nWidth,(unsigned long**)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,x1,y1);
			}


			//Overlay
			if (m_pOriOverlayData)
				ImgSizeOneSampleResample((BYTE**)pRetOverlayData,nHeight,nWidth,(BYTE**)m_pOriOverlayData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,x1,y1);			
		}

		if (nType==HSIZE_BICUBIC)
		{
			if (nBytePerPix==1)
				ImgSizeOneSampleBicubic((BYTE **)pRetPixData,nHeight,nWidth,(BYTE **)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,m_ImgInfo.iLutStart,m_ImgInfo.iLutEnd);
			else if(nBytePerPix==2)
			{
				if(m_ImgInfo.nPixelRepresentation==1)
					ImgSizeOneSampleBicubic((short **)pRetPixData,nHeight,nWidth,(short **)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,m_ImgInfo.iLutStart,m_ImgInfo.iLutEnd);
				else
					ImgSizeOneSampleBicubic((unsigned short **)pRetPixData,nHeight,nWidth,(unsigned short **)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,m_ImgInfo.iLutStart,m_ImgInfo.iLutEnd);
			}
			else if (nBytePerPix==4)
			{
				if(m_ImgInfo.nPixelRepresentation==1)
					ImgSizeOneSampleBicubic((long **)pRetPixData,nHeight,nWidth,(long **)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,m_ImgInfo.iLutStart,m_ImgInfo.iLutEnd);
				else
					ImgSizeOneSampleBicubic((unsigned long **)pRetPixData,nHeight,nWidth,(unsigned long **)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,m_ImgInfo.iLutStart,m_ImgInfo.iLutEnd);
			}

			//OverLay
			if (m_pOriOverlayData)
				ImgSizeOneSampleBicubic((BYTE **)pRetOverlayData,nHeight,nWidth,(BYTE **)m_pOriOverlayData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,0,255);

		}
	}
	else
	{
		ImgSizeThreeSample((MYDATA24**)pRetPixData,nHeight,nWidth,(MYDATA24**)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,x1,y1,nType);
		//Overlay
		if (m_pOriOverlayData)
		{
			if(nType==HSIZE_NORMAL)
				ImgSizeOneSampleNormal(m_pDisplayOverlayData,nHeight,nWidth,m_pOriOverlayData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0);
			if(nType==HSIZE_RESAMPLE)
				ImgSizeOneSampleResample(m_pDisplayOverlayData,nHeight,nWidth,m_pOriOverlayData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,x1,y1);
			if(nType==HSIZE_BICUBIC)
				ImgSizeOneSampleBicubic((BYTE **)m_pDisplayOverlayData,nHeight,nWidth,(BYTE **)m_pOriOverlayData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,x0,y0,0,255);
		}
	}

	return Ret_Success;
}
////////////��ֵ///////////////////////////////////////////////////////////////////////////
//Ŀ��:��m_pOriData��pSrcRc���������ͨ���������ػ�������صķ�ʽ��������.
//����:ԭʼ���ؾ���m_pOriData��m_pOverlayData��m_ImageInfo
//��Ʒ:m_pDisplayData,m_pDisplayOverlayData
//����:	pSrcRc	= m_pOriData�ľ�������--���ӹ�����. ��ΪNULL��Ϊ�ӹ�ȫ������
//		nWidth	= ��Ʒm_pDisplayData�Ŀ�;��Ϊ0�������nHeight��RcSrc�����Զ�����
//		nHeight	= ��Ʒm_pDisplayData�ĸ�;��Ϊ0�������nWidth��RcSrc�����Զ�����
//		nType	= ��ֵ��ʽ:HSIZE_NORMAL,HSIZE_RESAMPLE,HSIZE_BICUBIC
////////////////////////////////////////////////////////////////////////////////////////////
int CHsBaseImg::Hs_Size(RECT *pSrcRc, unsigned long nWidth, unsigned long nHeight, short nType,bool bForce)
{

	if(m_pOriData==NULL)
		return Ret_InvalidPara;


	if (pSrcRc==NULL && nWidth==0 && nHeight==0)
	{
		pSrcRc = m_pPreSizeRc;
		nWidth = m_PreSize.cx;
		nHeight= m_PreSize.cy;
		nType = m_nPreSizeType;
	}

	unsigned long nBytePerPix = (m_ImgInfo.nBitsAllocated*m_ImgInfo.nSamplePerPixel)/8;
	if (nType==HSIZE_NONE)
	{
		if(m_pDisplayData)
			ArrayFree((void **)m_pDisplayData,0);
		m_pDisplayData = NULL;

		if(m_pDisplayOverlayData)
			ArrayFree((void **)m_pDisplayOverlayData);
		m_pDisplayOverlayData = NULL;

		m_ImgState.nDispalyRow = m_ImgState.nCurOriPixRow;
		m_ImgState.nDispalyCol = m_ImgState.nCurOriPixCol;

		//��û�ڴ������
		m_pDisplayData 	=	(BYTE**)ArrayCopy((void**)m_pOriData,			m_ImgState.nDispalyRow,	m_ImgState.nDispalyCol,	nBytePerPix);
		m_pDisplayOverlayData = (BYTE**)ArrayCopy((void**)m_pOriOverlayData,	m_ImgState.nDispalyRow,	m_ImgState.nDispalyCol,	1);

		//int nSharp = m_ImgState.nSharpValueV.size();
		//for(int s=0;s<nSharp;s++)
		//	Hs_Sharp(m_ImgState.nSharpValueV[s]);

        Hs_WinLevel(m_ImgState.CurWc.x,m_ImgState.CurWc.y);

		return Ret_Success;
	}


	//��m_pOriData���ĸ���
	long x0 = 0;
	long x1 = 0;
	long y0 = 0;
	long y1 = 0;

	//ȷ����ͼ�ĳ������
	unsigned long w = 0;
	unsigned long h = 0;
	//if (m_ImgInfo.nSamplePerPixel == 3)//���ڲ�ɫͼ�񣬱��������ṩ�ֲ���ֵ--�����ṩ��
	//	pSrcRc = NULL;

	if (pSrcRc==NULL)
	{
		w = m_ImgState.nCurOriPixCol;
		h = m_ImgState.nCurOriPixRow;

		x0 = 0; x1 = w;
		y0 = 0;	y1 = h;

		m_ImgState.bWholeImgSized = true;
	}
	else
	{
		RECT ComRc;
		RECT ImgRc = {0,0,m_ImgState.nCurOriPixCol,m_ImgState.nCurOriPixRow};
		if(::IntersectRect(&ComRc,pSrcRc,&ImgRc) == FALSE)//������pSrcRcӦ�������ؾ����н���
			return Ret_InvalidPara;

		w = ComRc.right - ComRc.left;
		h = ComRc.bottom - ComRc.top;

		x0 = ComRc.left;
		x1 = ComRc.right;

		y0 = ComRc.top;
		y1 = ComRc.bottom;

		m_ImgState.bWholeImgSized = false;
	}

	if(w==0 || h==0)//������Ϊ0
		return Ret_InvalidPara;
	
	//������ͼ�ĳ���
	if (nWidth==0 && nHeight!=0)
	{
		nWidth = w*nHeight/h;
	}
	else if (nWidth!=0 && nHeight==0)
	{
		nHeight = nWidth*h/w;
	}
	else if(nWidth!=0 && nHeight!=0)
	{
		//RECT rcAll = {0,0,nWidth,nHeight};
		//RECT rcImg = GetShowRcByImgSize(rcAll,pSrcRc->right-pSrcRc->left,pSrcRc->bottom-pSrcRc->top);//m_ImgState.nCurOriPixCol,m_ImgState.nCurOriPixRow

		//nWidth = rcImg.right - rcImg.left;
		//nHeight = rcImg.bottom - rcImg.top;
	}
	else if(nWidth==0 && nHeight==0)
	{
		return Ret_InvalidPara;
	}

	nWidth = ((nWidth*8)+31)/32*4;

	//������ͬЧ���Ĳ�ֵ,�Ͳ�Ҫ������---bForceǿ�Ʋ���û���õĻ�
	if (m_PreSize.cx == nWidth && m_PreSize.cy == nHeight && m_nPreSizeType==nType && bForce==false)
	{
		if (pSrcRc==NULL)
		{
			if(m_pPreSizeRc==NULL)
				return Ret_Success;
		}
		else
		{
			if (m_pPreSizeRc!=NULL)
			{
				if (m_pPreSizeRc->left == pSrcRc->left && m_pPreSizeRc->right == pSrcRc->right && m_pPreSizeRc->top == pSrcRc->top && m_pPreSizeRc->bottom == pSrcRc->bottom)
					return Ret_Success;
			}
		}
		
	}
	else
	{
		m_PreSize.cx = nWidth;
		m_PreSize.cy = nHeight;
		m_nPreSizeType = nType;

		if (pSrcRc!=NULL)
		{
			if(m_pPreSizeRc==NULL )
				m_pPreSizeRc = new RECT;

			if(m_pPreSizeRc!=pSrcRc)//�����Լ�=�Լ�
				*m_pPreSizeRc = *pSrcRc;
		}

	}

	//����ÿ�ζ�Ҫ�������ڴ�.�����ͼ������.�ͼ���ʹ���ϴε�m_pDisplayData
	if(m_ImgState.nDispalyRow != nHeight || m_ImgState.nDispalyCol != nWidth)
	{
		if(m_pDisplayData)
			ArrayFree((void **)m_pDisplayData,0);
		m_pDisplayData = NULL;

		if(m_pDisplayOverlayData)
			ArrayFree((void **)m_pDisplayOverlayData);
		m_pDisplayOverlayData = NULL;

		m_ImgState.nDispalyRow = nHeight;
		m_ImgState.nDispalyCol = nWidth;
	}

	//��û�ڴ������
	if(m_pDisplayData==NULL)
		m_pDisplayData = (BYTE**)ArrayNew(m_ImgState.nDispalyRow,m_ImgState.nDispalyCol,nBytePerPix);

	if(m_pDisplayOverlayData==NULL && m_pOriOverlayData)
		m_pDisplayOverlayData = (BYTE**)ArrayNew(m_ImgState.nDispalyRow,m_ImgState.nDispalyCol,1);

	//AtlTrace("HS_Size New:	Row:%d,	Col:%d",m_ImgState.nDispalyRow,m_ImgState.nDispalyCol);

	RECT sRc = {x0,y0,x1,y1};
	ImgSize(m_pDisplayData,m_pDisplayOverlayData,&sRc,nWidth,nHeight,nType);

	int nSharp = m_ImgState.nSharpValueV.size();
	for(int s=0;s<nSharp;s++)
		Hs_Sharp(m_ImgState.nSharpValueV[s],s==nSharp-1,m_hWnd);

	//UpdateBits(m_nDispalyRow,m_nDispalyCol,m_pDisplayData,m_pDisplayOverlayData,m_pBits);
    Hs_WinLevel(m_ImgState.CurWc.x,m_ImgState.CurWc.y);
	return Ret_Success;
}
void CHsBaseImg::ImgSizeThreeSample(MYDATA24** pNewData,unsigned long nNewRow,unsigned long nNewCol,MYDATA24 **pOriData,unsigned long nOriRow,unsigned long nOriCol, long x0, long y0,long x1,long y1,short nSizeType)
{
	//float fZoomX = (nOriCol-0)*1.00/nNewCol;
	//float fZoomY = (nOriRow-0)*1.00/nNewRow;

	float fZoomX = (x1-x0)*1.00/nNewCol;
	float fZoomY = (y1-y0)*1.00/nNewRow;

	if (nSizeType==HSIZE_NORMAL)
	{
		unsigned long tr = 0;
		unsigned long tc = 0;

		for(unsigned long r=0;r<nNewRow;r++)
		{
			tr = y0+(unsigned long)(r*fZoomY);
			for(unsigned long c=0;c<nNewCol;c++)
			{
				tc = x0+(unsigned long)(c*fZoomX);
				pNewData[r][c] = pOriData[tr][tc];
			}
		}

		return;
	}

	if (nSizeType==HSIZE_RESAMPLE)
	{
		if (0)
		{
			unsigned long X0 = 0;
			unsigned long Y0 = 0;
			unsigned long X1 = 0;
			unsigned long Y1 = 0;

			double fr = 0;
			double fc = 0;

			for(unsigned long r=0;r<nNewRow;r++)
			{
				fr = y0+r*fZoomY ;

				Y0 = (unsigned long)fr;
				Y1 = (Y0+1);
				if(Y1>=m_ImgState.nCurOriPixRow)
					Y1 = m_ImgState.nCurOriPixRow-1;

				for(unsigned long c=0;c<nNewCol;c++)
				{
					fc = x0 + c*fZoomX ;

					X0 = (unsigned long)fc;
					X1 = X0+1;
					if(X1>=m_ImgState.nCurOriPixCol)
						X1 = m_ImgState.nCurOriPixCol-1;

					//������,�ٺ���
					BYTE B0 =  pOriData[Y0][X1].pData[0]*(fc-X0) + pOriData[Y0][X0].pData[0]*(X1-fc);
					BYTE G0 =  pOriData[Y0][X1].pData[1]*(fc-X0) + pOriData[Y0][X0].pData[1]*(X1-fc);
					BYTE R0 =  pOriData[Y0][X1].pData[2]*(fc-X0) + pOriData[Y0][X0].pData[2]*(X1-fc);

					BYTE B1 =  pOriData[Y1][X1].pData[0]*(fc-X0) + pOriData[Y1][X0].pData[0]*(X1-fc);
					BYTE G1 =  pOriData[Y1][X1].pData[1]*(fc-X0) + pOriData[Y1][X0].pData[1]*(X1-fc);
					BYTE R1 =  pOriData[Y1][X1].pData[2]*(fc-X0) + pOriData[Y1][X0].pData[2]*(X1-fc);

					pNewData[r][c].pData[0] = B1*(fr-Y0) + B0*(Y1-fr);
					pNewData[r][c].pData[1] = G1*(fr-Y0) + G0*(Y1-fr);
					pNewData[r][c].pData[2] = R1*(fr-Y0) + R0*(Y1-fr);
				}
			}

			return;
		}

		//if (0)
		//{
		//	//��ЩRow����,��������nNewRow��
		//	float fr = 0.00;
		//	int Y = 0;
		//	float fy = 0.00;

		//	//��ЩCol�������������ѭ���ڽ������� nNewRow*nNewCol��.�ظ���nNewRow*(nNewCol-1)��,����Ҫ�����
		//	float fc =0.00;
		//	int *X = new int[nNewCol];		memset(X, 0,nNewCol*sizeof(int));
		//	float *fx = new float[nNewCol];	memset(fx,0,nNewCol*sizeof(float));


		//	int preC = 0;
		//	for (int c=0;c<nNewCol;c++)
		//	{
		//		fc = c*fZoomX;
		//		X[c] = int(fc) + x0;
		//		fx[c] = float(fc- X[c] + x0);


		//		if(X[c]+2>=nOriCol)//����ҪԽ��
		//		{
		//			fc = preC*fZoomX;//.�������һ����ȷ��cҲ����preC,������
		//			X[c] = int(fc) + x0;
		//			fx[c] = float(fc- X[c] + x0);
		//		}
		//		else
		//		{
		//			preC = c;
		//		}

		//	}

		//	int PreR = 0;
		//	for(int r=0;r<nNewRow;r++)
		//	{
		//		fr = r*fZoomY;
		//		Y = int(fr) + y0;
		//		fy = float(fr - Y + y0);


		//		if ( Y>=nOriRow-2 )//����ҪԽ��.
		//		{
		//			fr = PreR*fZoomY;//�������һ����ȷ��rҲ����preR,������
		//			Y = int(fr) + y0;
		//			fy = float(fr - Y + y0);
		//		}
		//		else
		//		{
		//			PreR = r;
		//		}
		//		for(int c=0;c<nNewCol;c++)
		//		{
		//			pNewData[r][c].pData[0] = (BYTE)( (1-fy)*(1-fx[c])*pOriData[Y+1][X[c]+1].pData[0] + (1-fy)* fx[c]	*pOriData[Y+1][X[c]+2].pData[0] + fy*(1-fx[c])	*pOriData[Y+2][X[c]+1].pData[0] + fy*fx[c] * pOriData[Y+2][X[c]+2].pData[0] );//��ȡ��
		//			pNewData[r][c].pData[1] = (BYTE)( (1-fy)*(1-fx[c])*pOriData[Y+1][X[c]+1].pData[1] + (1-fy)* fx[c]	*pOriData[Y+1][X[c]+2].pData[1] + fy*(1-fx[c])	*pOriData[Y+2][X[c]+1].pData[1] + fy*fx[c] * pOriData[Y+2][X[c]+2].pData[1] );//��ȡ��
		//			pNewData[r][c].pData[2] = (BYTE)( (1-fy)*(1-fx[c])*pOriData[Y+1][X[c]+1].pData[2] + (1-fy)* fx[c]	*pOriData[Y+1][X[c]+2].pData[2] + fy*(1-fx[c])	*pOriData[Y+2][X[c]+1].pData[2] + fy*fx[c] * pOriData[Y+2][X[c]+2].pData[2] );//��ȡ��
		//		}

		//		//for(int c=0;c<nNewCol;c++)//�Ż�ǰ:�б�����ȡǰ
		//		//{
		//		//	float fc = c*fZoomX;
		//		//	int X = int(fc) + x0;
		//		//	float fx = float(fc- X + x0);

		//		//	if ( X>=nOriCol-2 )
		//		//		X = nOriCol-3;

		//		//	pNewData[r][c] = (T4)(   (1-fy)*(1-fx)*pOriData[Y+1][X+1]+(1-fy)*fx*pOriData[Y+1][X+2]+fy*(1-fx)*pOriData[Y+2][X+1]+fy*fx*pOriData[Y+2][X+2]   );
		//		//}
		//	}

		//	delete []X;
		//	delete []fx;
		//}
		

		if (1)
		{
			RECT trc = {min(x0,x1),min(y0,y1),max(x0,x1),max(y0,y1)};

			//double resize_time = fZoomX;

			int ii1=0,jj1=0;
			float u1=0.0, v1=0.0;
			float temp1=0.0, temp2=0.0,temp3=0.0, temp4=0.0;
			for(int r=0;r<nNewRow;r++)
			{
				ii1 = int(floor(r*fZoomY)) + trc.top;
				u1 = float(r*fZoomY - ii1 + trc.top);
				if ( ii1>=nOriRow - 1 )
				{
					ii1 = nOriRow - 2;
				}
				for(int c=0;c<nNewCol;c++)
				{
					jj1 = int(floor(c*fZoomX)) + trc.left;
					v1 = float(c*fZoomX - jj1 + trc.left);
					if ( jj1 >= nOriCol - 1 )
					{
						jj1 = nOriCol - 2;
					}
					temp1 = (1-u1)*(1-v1);
					temp2 = (1-u1)*v1;
					temp3 = u1*(1-v1);
					temp4 = u1*v1;

					pNewData[r][c].pData[0] = (unsigned short int)(temp1*pOriData[ii1][jj1].pData[0] + temp2*pOriData[ii1][jj1+1].pData[0] + temp3*pOriData[ii1+1][jj1].pData[0] + temp4*pOriData[ii1+1][jj1+1].pData[0]);
					pNewData[r][c].pData[1] = (unsigned short int)(temp1*pOriData[ii1][jj1].pData[1] + temp2*pOriData[ii1][jj1+1].pData[1] + temp3*pOriData[ii1+1][jj1].pData[1] + temp4*pOriData[ii1+1][jj1+1].pData[1]);
					pNewData[r][c].pData[2] = (unsigned short int)(temp1*pOriData[ii1][jj1].pData[2] + temp2*pOriData[ii1][jj1+1].pData[2] + temp3*pOriData[ii1+1][jj1].pData[2] + temp4*pOriData[ii1+1][jj1+1].pData[2]);

				}
			}	
		}
	}

	if (nSizeType==HSIZE_BICUBIC)
	{
		float A_Matrix[4] = {1,1,1,0};

		float B_Matrix0[4][4]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
		float B_Matrix1[4][4]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
		float B_Matrix2[4][4]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

		float C_Matrix[4]	={0,0,0,0};

		float i = 0.00;
		float j = 0.00;
		float u = 0.00;
		float v = 0.00;

		float tmp0 = 0.00;
		float tmp1 = 0.00;
		float tmp2 = 0.00;

		float fr = 0;
		float fc = 0;
		int tr=0, tc=0;

		for(unsigned long r=0;r<nNewRow;r++)
		{
			fr = y0+r*fZoomY;

			i = ((unsigned long)fr);
			u = fr - i;

			for(unsigned long c=0;c<nNewCol;c++)
			{
				fc = x0 + c*fZoomX;

				j = ((unsigned long)fc);
				v = fc - j;

				for (BYTE t=0; t<4; t++)
				{
					A_Matrix[t] = SinPiX(u-t+1);
					C_Matrix[t] = SinPiX(v-t+1);
					for (BYTE f=0; f<4; f++)
					{
						tr = i+t-1;
						tc = j+f-1;
						if(tc>=m_ImgState.nCurOriPixCol) tc = m_ImgState.nCurOriPixCol-1;
						if(tr>=m_ImgState.nCurOriPixRow) tr = m_ImgState.nCurOriPixRow-1;
						if(tc<=x0) 
							tc = x0;
						if(tr<=y0) 
							tr = y0;

						B_Matrix0[t][f] = pOriData[tr][tc].pData[0];
						B_Matrix1[t][f] = pOriData[tr][tc].pData[1];
						B_Matrix2[t][f] = pOriData[tr][tc].pData[2];
					}
				}

				tmp0 = Matrix_Bicubic(A_Matrix,B_Matrix0,C_Matrix);
				if ( tmp0 < m_ImgInfo.iLutStart)
					pNewData[r][c].pData[0] = m_ImgInfo.iLutStart;
				else if ( tmp0 >= m_ImgInfo.iLutEnd)
					pNewData[r][c].pData[0] = m_ImgInfo.iLutEnd - 1; //m_imgInfo.HS_LUT_length-1;
				else
					pNewData[r][c].pData[0] = (BYTE)tmp0;

				tmp1 = Matrix_Bicubic(A_Matrix,B_Matrix1,C_Matrix);
				if ( tmp1 < m_ImgInfo.iLutStart)
					pNewData[r][c].pData[1] = m_ImgInfo.iLutStart;
				else if ( tmp1 >= m_ImgInfo.iLutEnd)
					pNewData[r][c].pData[1] = m_ImgInfo.iLutEnd - 1; //m_imgInfo.HS_LUT_length-1;
				else
					pNewData[r][c].pData[1] = (BYTE)tmp1;

				tmp2 = Matrix_Bicubic(A_Matrix,B_Matrix2,C_Matrix);
				if ( tmp2 < m_ImgInfo.iLutStart)
					pNewData[r][c].pData[2] = m_ImgInfo.iLutStart;
				else if ( tmp2 >= m_ImgInfo.iLutEnd)
					pNewData[r][c].pData[2] = m_ImgInfo.iLutEnd - 1; //m_imgInfo.HS_LUT_length-1;
				else
					pNewData[r][c].pData[2] = (BYTE)tmp2;
			}
		}

		return;
	}

}

LutData *CHsBaseImg::GetLutByName(QString sLutName)
{
	int n = int(m_pLutV.size());
	for (int i=0;i<n;i++)
	{
		if(int(m_pLutV[i]->sName.compare(sLutName))==0)
			return m_pLutV[i];
	}

	return NULL;
}
double CHsBaseImg::Hs_GetPixelValue(unsigned long r,unsigned long c)
{
	if (m_pOriData==NULL)
		return 0.00;

	if(m_ImgInfo.nSamplePerPixel==1)
	{
		if (m_ImgInfo.nBitsPerPixel==8)
		{
			if (m_ImgInfo.nPixelRepresentation==0)
			{
				unsigned char ** pPixData = (unsigned char**)m_pOriData;
				double fValue = pPixData[r][c]*1.00;

				if(m_ImgInfo.fRescaleSlope!=0.00 && m_ImgInfo.fRescaleIntercept!=0.00)
					fValue = fValue*m_ImgInfo.fRescaleSlope + m_ImgInfo.fRescaleIntercept;

				return fValue;
			}
			else
			{
				char ** pPixData = (char**)m_pOriData;
				double fValue = pPixData[r][c]*1.00;

				if(m_ImgInfo.fRescaleSlope!=0.00 && m_ImgInfo.fRescaleIntercept!=0.00)
					fValue = fValue*m_ImgInfo.fRescaleSlope + m_ImgInfo.fRescaleIntercept;

				return fValue;
			}

		}
		else if (m_ImgInfo.nBitsPerPixel==16)
		{
			if (m_ImgInfo.nPixelRepresentation==0)
			{
				unsigned short ** pPixData = (unsigned short**)m_pOriData;
				double fValue = pPixData[r][c]*1.00;

				if(m_ImgInfo.fRescaleSlope!=0.00 && m_ImgInfo.fRescaleIntercept!=0.00)
					fValue = fValue*m_ImgInfo.fRescaleSlope + m_ImgInfo.fRescaleIntercept;
//pPixData[r][c] = 0;
				return fValue;
			}
			else
			{
				short ** pPixData = (short**)m_pOriData;
				double fValue = pPixData[r][c]*1.00;

				if(m_ImgInfo.fRescaleSlope!=0.00 && m_ImgInfo.fRescaleIntercept!=0.00)
					fValue = fValue*m_ImgInfo.fRescaleSlope + m_ImgInfo.fRescaleIntercept;
//pPixData[r][c] = 0;
				return fValue;
			}

		}
		else if (m_ImgInfo.nBitsPerPixel==32)
		{				
			long ** pPixData = (long**)m_pOriData;
			double fValue = pPixData[r][c]*1.00;

			if(m_ImgInfo.fRescaleSlope!=0.00 && m_ImgInfo.fRescaleIntercept!=0.00)
				fValue = fValue*m_ImgInfo.fRescaleSlope + m_ImgInfo.fRescaleIntercept;

			return fValue;
		}
	}
	else if (m_ImgInfo.nSamplePerPixel==3)
	{			
		MYDATA24 **pPixData = (MYDATA24**)m_pOriData;
		return double(RGB(pPixData[r][c].pData[2] , pPixData[r][c].pData[1] ,  pPixData[r][c].pData[0]));
	}


	return 0;
}
//
//int CHsBaseImg::Hs_CreateBitmapHandle(unsigned long nWidth,unsigned long nHeight,short nSizeType,HBITMAP &reBmp,bool bRuler,bool bPtInfo,bool bAnno)
//{
//	bool bFree = false;
//	if(Hs_IsEmpty()==true)
//	{
//		Hs_Reload();
//		bFree = true;
//	}
//
//
//	int w = m_ImgState.nCurOriPixCol;
//	int h = m_ImgState.nCurOriPixRow;
//
//
//	if (nSizeType!=HSIZE_NONE)
//	{
//		nWidth = ((nWidth*8)+31)/32*4;
//
//		if (nWidth==0 && nHeight!=0)
//			nWidth = w*nHeight/h;
//		else if (nWidth!=0 && nHeight==0)
//			nHeight = nWidth*h/w;
//		else if(nWidth==0 && nHeight==0)
//			return Ret_InvalidPara;
//	}
//
//
//	if (nWidth<1) nWidth = 1;
//	if (nHeight<1) nHeight = 1;
//
//	//��ʱ�����Ҵ�����ͼ̫���ˣ�CreateCompatibleBitmap�ܲ��˵�,��Сһ��
//	int nMaxSize = 63000000;
//	if (nWidth*nHeight>nMaxSize)
//	{
//		double f = sqrt(nMaxSize*1.00/(nWidth*nHeight) );
//		nWidth = f*nWidth;
//		nHeight = f*nHeight;
//
//		nSizeType = HSIZE_RESAMPLE;
//	}
//
//	if (1)
//	{
//		unsigned long nBytePerPix = (m_ImgInfo.nBitsAllocated*m_ImgInfo.nSamplePerPixel)/8;
//
//		BYTE ** pDisData = NULL;
//		BYTE ** pOverlay = NULL;
//
//		RECT RcOfOriData = {0,0,w,h};
//		if (nSizeType == HSIZE_NONE)
//		{
//			pDisData = (BYTE**)ArrayCopy((void**)m_pOriData,			m_ImgState.nCurOriPixRow,	m_ImgState.nCurOriPixCol,	nBytePerPix);
//			pOverlay = (BYTE**)ArrayCopy((void**)m_pOriOverlayData,		m_ImgState.nCurOriPixRow,	m_ImgState.nCurOriPixCol,		1);
//
//			if (m_bSharpOri == false)//����ʾ����
//			{
//				int nSharp = m_ImgState.nSharpValueV.size();
//				for (int i=0;i<nSharp;i++)
//				{
//					BYTE** pNewDsp = ImgSharp(pDisData,m_ImgState.nCurOriPixRow,	m_ImgState.nCurOriPixCol,m_ImgState.nSharpValueV[i],m_hWnd);
//					if(pNewDsp)
//					{
//						ArrayFree((void**)pDisData);
//						pDisData = pNewDsp;
//					}
//				}
//			}
//		}
//		else
//		{
//			pDisData = (BYTE **)ArrayNew(nHeight,nWidth,nBytePerPix);
//			pOverlay = (BYTE **)ArrayNew(nHeight,nWidth,1);
//			ImgSize(pDisData,pOverlay,&RcOfOriData,nWidth,nHeight,nSizeType);
//		}
//	
//		//Hs_WinLevel(m_ImgState.CurWc.x,m_ImgState.CurWc.y);//m_ImgInfo.fWinWidth,m_ImgInfo.fWinCenter
//		WinLevelNormal( m_ImgState.CurWc.x,m_ImgState.CurWc.y);
//
//		BYTE *pBits = NULL;
//
//		UpdateBits(nHeight,nWidth,pDisData,pOverlay,pBits);
//
//		//////////////////////////////////////////////////////////////////////////
//
//		HDC hdc = GetDC(NULL);
//		HDC hMemDC = CreateCompatibleDC(hdc);
//		RECT rc = {0, 0, nWidth, nHeight};
//		reBmp = CreateCompatibleBitmap(hdc, nWidth, nHeight);
//		if(reBmp==NULL)
//		{//���Բ�ͬ���޲�ͬ����ʱ����ܻ��Ǵ���������
//			
//			char str[256];
//			sprintf(str,"����ʧ��\r\n���δ���ͼ��ߴ�Ϊ\r\n%d X %d = %d\r\n������������",nWidth,nHeight,nWidth*nHeight);
//			::MessageBox(CDcmStationCtrl::m_pThis->m_hWnd,str,"",0);
//
//			reBmp = CreateCompatibleBitmap(hdc, 256, 256);
//			HGDIOBJ tOld = SelectObject(hMemDC, (HGDIOBJ)reBmp);
//
//			CWxsDraw dr;
//			RECT xrc = {0,0,256,256};
//			dr.FillRect(hMemDC,xrc,RGB(255,255,255));
//
//			xrc.top = 127;
//			dr.DrawTxt(hMemDC,xrc,str,DT_CENTER,RGB(255,0,0));
//
//			::SelectObject(hMemDC, tOld);
//			::DeleteObject(hMemDC);
//			::ReleaseDC(NULL,hdc);
//
//			ArrayFree((void**)pDisData,5);
//			ArrayFree((void**)pOverlay);
//			delete []pBits;
//
//			if (bFree)
//				Hs_FreeMem();
//
//			return 0;//Ret_CreateBitmapErr;ֻ�ܴ��ϰ�
//		}
//		HGDIOBJ hOld = SelectObject(hMemDC, (HGDIOBJ)reBmp);
//		SetBkColor(hMemDC, RGB(0, 0, 0));
//		//ExtTextOut(hMemDC, 0, 0, ETO_OPAQUE, &rc, 0, 0, 0);
//
//		Hs_DrawImg(hMemDC,rc,pBits,nHeight,nWidth);
//
//		
//		if (bRuler)
//		{
//			if (m_ImgInfo.fPixelSpaceY>0.000001)
//			{
//				//double fMag = (rc.bottom - rc.top)*1.00/m_ImgState.nCurOriPixRow;
//				int h = rc.bottom - rc.top;
//				int nWidth = 1;
//				if(h<=600)
//					nWidth = 1;
//				else if(h>600 && h<1500)
//					nWidth = 2;
//				else
//					nWidth = 3;
//
//
//				DrawRuler(hMemDC,nWidth,rc,m_ImgInfo.fPixelSpaceY,1.00,'R');
//			}
//
//		}
//
//		if (bPtInfo)
//		{
//			RECT trc = rc;
//			trc.left+=2;trc.top+=2;trc.right+=2;trc.bottom+=2;
//
//			if(rc.right - rc.left > 512)
//				Hs_DrawInfo(hMemDC,trc,InfoType_Draw,0,false);
//
//			trc = rc;
//			Hs_DrawInfo(hMemDC,trc,InfoType_Draw,m_DrawInfo.clor,false);
//		}
//
//		if (bAnno)
//		{
//			//ע��ͼ��
//			m_AnnoManager.BackUpPara();
//			m_AnnoManager.SetBasePara(NULL,&(m_ImgState.nCurOriPixCol),&(m_ImgState.nCurOriPixRow),&rc);
//			m_AnnoManager.DrawAnnObjs(hMemDC,rc);
//			m_AnnoManager.ResumePara();
//
//			//��λ��
//			m_LocLineManager3.BackUpPara();
//			m_LocLineManager3.SetBasePara(NULL,&(m_ImgState.nCurOriPixCol),&(m_ImgState.nCurOriPixRow),&rc);
//			m_LocLineManager3.DrawAnnObjs(hMemDC,rc);
//			m_LocLineManager3.ResumePara();
//
//			m_LocLineManager2.BackUpPara();
//			m_LocLineManager2.SetBasePara(NULL,&(m_ImgState.nCurOriPixCol),&(m_ImgState.nCurOriPixRow),&rc);
//			m_LocLineManager2.DrawAnnObjs(hMemDC,rc);
//			m_LocLineManager2.ResumePara();
//
//			m_LocLineManager1.BackUpPara();
//			m_LocLineManager1.SetBasePara(NULL,&(m_ImgState.nCurOriPixCol),&(m_ImgState.nCurOriPixRow),&rc);
//			m_LocLineManager1.DrawAnnObjs(hMemDC,rc);
//			m_LocLineManager1.ResumePara();
//		}
//
//		SelectObject(hMemDC, hOld);
//		DeleteDC(hMemDC);
//		ReleaseDC(NULL, hdc);
//
//		//////////////////////////////////////////////////////////////////////////
//
//		ArrayFree((void**)pDisData,6);
//		ArrayFree((void**)pOverlay);
//		delete []pBits;
//	}
//
//	if (0)
//	{
//		HDC hdc = GetDC(NULL);
//		HDC hMemDC = CreateCompatibleDC(hdc);
//		RECT rc = {0, 0, nWidth, nHeight};
//		reBmp = CreateCompatibleBitmap(hdc, nWidth, nHeight);
//		HGDIOBJ hOld = SelectObject(hMemDC, (HGDIOBJ)reBmp);
//		SetBkColor(hMemDC, RGB(0, 0, 0));
//		//ExtTextOut(hMemDC, 0, 0, ETO_OPAQUE, &rc, 0, 0, 0);
//
//		Hs_DrawImg(hMemDC,rc);
//
//		SelectObject(hMemDC, hOld);
//		DeleteDC(hMemDC);
//		ReleaseDC(NULL, hdc);
//
//	}
//
//	if (bFree)
//		Hs_FreeMem();
//
//	return Ret_Success;
//}
int CHsBaseImg::Hs_CopyTo(CHsBaseImg &NewHsImg)
{
	NewHsImg.Hs_FreeMem();

	//ԭʼ���ؿ���
	if (m_pOriData)
		NewHsImg.m_pOriData = (BYTE**)ArrayCopy((void**)m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,m_ImgInfo.nBitsPerPixel/8);

	//��ʾ���ؿ���
	//if(m_pDisplayData)
	//	NewHsImg.m_pDisplayData = (BYTE**)ArrayCopy((void**)m_pDisplayData,m_ImgState.nDispalyRow,m_ImgState.nDispalyCol,m_ImgInfo.nBitsPerPixel/8);

	//ԭʼoverlay���ظ���
	if(m_pOriOverlayData)
		NewHsImg.m_pOriOverlayData = (BYTE**)ArrayCopy((void**)m_pOriOverlayData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,1);

	//��ʾ�õ�overlay���ظ���
	//if(m_pDisplayOverlayData)
	//	NewHsImg.m_pDisplayOverlayData = (BYTE**)ArrayCopy((void**)m_pDisplayOverlayData,m_ImgState.nDispalyRow,m_ImgState.nDispalyCol,1);


	//ͼ�������Ϣ����
	NewHsImg.m_ImgInfo = m_ImgInfo;

	//ͼ��Lut����
	int sz = m_pLutV.size();
	for(int i=0;i<sz;i++)
	{
		LutData *pNewData = new LutData;
		*pNewData = *(m_pLutV[i]);

		NewHsImg.m_pLutV.push_back(pNewData);
	}

	//ͼ��ǰ״̬��������
	NewHsImg.m_ImgState = m_ImgState;

	//pBits����
	if (m_ImgInfo.bGrayImg)
		sz = m_ImgState.nCurBitsRow*m_ImgState.nCurBitsCol;
	else
		sz = m_ImgState.nCurBitsRow*m_ImgState.nCurBitsCol*sizeof(COLORREF);
		
	if (m_pBits)
	{
		NewHsImg.m_pBits = new BYTE[sz];
		memcpy(NewHsImg.m_pBits,m_pBits,sz);
	}

	//Wc Lut����
	if(m_pWcLut)
	{
		NewHsImg.m_pWcLut = new long[m_ImgInfo.nWcLutLen];
		memcpy(NewHsImg.m_pWcLut,m_pWcLut,m_ImgInfo.nWcLutLen);
	}

	////��ʾ��Ϣ����
	//NewHsImg.m_DrawInfo = m_DrawInfo;

	////��ӡ��Ϣ����
	//NewHsImg.m_PrintInfo = m_PrintInfo;

	////ע���帴��
	//m_AnnoManager.CopyTo(NewHsImg.m_AnnoManager);
	//m_LocLineManager1.CopyTo(NewHsImg.m_LocLineManager1);
	//m_LocLineManager2.CopyTo(NewHsImg.m_LocLineManager2);
	//m_LocLineManager3.CopyTo(NewHsImg.m_LocLineManager3);

	//
	////�ļ���Ϣ����
	//if (m_DcmInfo.bFilled==false)
	//	GetDcmFileInfo(m_DcmInfo);

	//NewHsImg.m_DcmInfo = m_DcmInfo;
	//
	////�������Ϣ�ǲ��ɿ��ġ��õ�ʱ��Ҫע��
	//NewHsImg.m_pInstance = m_pInstance;

	////Key����
	//NewHsImg.m_sKey = m_sKey;
	//
	//NewHsImg.m_nIdInSeries = m_nIdInSeries;

	////MPR����Ҳ���ܴ�ӡ
	//NewHsImg.m_pSeriesMprFrom = m_pSeriesMprFrom;
	//NewHsImg.m_MprPt0 = m_MprPt0;
	//NewHsImg.m_MprPt1 = m_MprPt1;

	return Ret_Success;
}
int CHsBaseImg::Hs_Cut(long &left,long &top,long &right,long &bottom)
{//���ĸ�����������ڵ�ǰ���ؾ�����Ե�.
//	AtlTrace("\r\n����ͼ��:	%d	%d	%d	%d",left, top, right, bottom);

	//m_bDrawInfoFilled = false;
	//m_bPrintInfoFilled = false;

	if(Hs_IsEmpty()==true)
		return Ret_EmptyImage;

	if(left<0)	left=0;
	if(top<0)	top=0;
	if(right >m_ImgState.nCurOriPixCol-1)	
		right=m_ImgState.nCurOriPixCol-1;

	int w = right - left;
	w = (long(w/4))*4;
	right = left + w;

	if(bottom>m_ImgState.nCurOriPixRow-1)	bottom=m_ImgState.nCurOriPixRow-1;

	if(right-left<=0 || bottom-top<=0)
		return Ret_InvalidPara;

	RECT CutRc = {left,top,right,bottom};

	int nRet = 0;

	BYTE **pNewOriData = (BYTE**)ArrayNew( CutRc.bottom-CutRc.top,CutRc.right-CutRc.left,m_ImgInfo.nBitsPerPixel/8);

	if (m_ImgInfo.nSamplePerPixel==1)
	{
		if (m_ImgInfo.nBitsPerPixel==8)//m_pDisplayDataһ���ֽڱ�ʾһ������
			nRet = ImageCut((BYTE**)m_pOriData,CutRc,(BYTE**)pNewOriData);
		else if(m_ImgInfo.nBitsPerPixel==16)//Ҫ��m_pDisplayData�������ֽڱ�ʾһ������
			nRet = ImageCut((short**)m_pOriData,CutRc,(short**)pNewOriData);
		else if (m_ImgInfo.nBitsPerPixel==32)
			nRet = ImageCut((long**)m_pOriData,CutRc,(long**)pNewOriData);
	}
	else if (m_ImgInfo.nSamplePerPixel==3)
	{
		if (m_ImgInfo.nBitsPerPixel==24)
			nRet = ImageCut((MYDATA24**)m_pOriData,CutRc,(MYDATA24**)pNewOriData);
	}


	ArrayFree((void**)m_pOriData);
	m_pOriData = pNewOriData;

	//Overlay
	if (m_pOriOverlayData)
	{
		BYTE **pNewOriOverlayData = (BYTE**)ArrayNew(CutRc.bottom-CutRc.top,CutRc.right-CutRc.left,1);
		nRet = ImageCut((BYTE**)m_pOriOverlayData,CutRc,(BYTE**)pNewOriOverlayData);

		delete m_pOriOverlayData;
		m_pOriOverlayData = pNewOriOverlayData;

	}

	//�и�����Ҫ���:�˺������ĸ�����,���������"��ǰ"m_pOriData���ؾ������(ע��"��ǰ"����),����������еڶ���,��ô�ٵ���Hs_ApplyImageState�����ͻᷢ������:
	//m_pOriData�Ǹմ��ļ��ж�������ԭʼ���ؾ���,���˴��������ĸ��������ϴμ���,Ҳ���ǵڶ��μ�������ڵ�һ�μ��к���������ؾ�����Ե�.
	//���˴���m_ImgState.pOriRect�Ǹ�Hs_ApplyImageState��Ϊ�������ݵ�.����һ��Ҫ��֤m_ImgState.pOriRectʼ�����������ԭʼ���ؾ������

	if (m_ImgState.nCurOriPixRow==m_ImgInfo.nRows && m_ImgState.nCurOriPixCol == m_ImgInfo.nCols)//��ǰ���ؾ�������ԭʼ����
	{
		if(m_ImgState.pOriRect)
		{	
			delete m_ImgState.pOriRect;
			m_ImgState.pOriRect = NULL;
		}

        m_ImgState.pOriRect = new RECT;
		*(m_ImgState.pOriRect) = CutRc;
	}
	else//����,�Ǿ��Ǳ����й��˵�
	{
		RECT trc = {0,0,m_ImgInfo.nCols,m_ImgInfo.nRows};

		if(m_ImgState.pOriRect)
		{	
			trc = *(m_ImgState.pOriRect);
			delete m_ImgState.pOriRect;
			m_ImgState.pOriRect = NULL;
		}

		//��ʱ��trc�������ԭʼ���ؾ������,CurRc�������"��ǰ"���ؾ������

        m_ImgState.pOriRect = new RECT;

		m_ImgState.pOriRect->left = trc.left + CutRc.left;
		m_ImgState.pOriRect->right = trc.left + CutRc.right;
		m_ImgState.pOriRect->top = trc.top + CutRc.top;
		m_ImgState.pOriRect->bottom = trc.top + CutRc.bottom;
	}


	//�޸ĵ�ǰ����������
	m_ImgState.nCurOriPixRow = CutRc.bottom-CutRc.top;
	m_ImgState.nCurOriPixCol = CutRc.right-CutRc.left;

	//m_ImgState.bWholeImgSized = true;

	return nRet;
}
//
//int CHsBaseImg::Hs_ApplyCurImgState()
//{
//	if(Hs_IsEmpty()==true)
//		return Ret_EmptyImage;
//
//	//Lut
//	int nLut = m_pLutV.size();
//	for (int i=0;i<nLut;i++)
//	{
//		if (m_pLutV[i]->sName.compare(m_ImgState.sCurLutName)==0)
//		{
//			m_pLutV[i]->bUse = true;
//		}
//		else
//		{
//			m_pLutV[i]->bUse = false;
//		}
//	}
//
//	int nRet = Ret_Success;
//	//������Ч��,���һ��Ҫ���ڵ�һλ,��Ϊ����Ч������ԭ�ϴ�Ч���е�m_imgState.nCurOriCol,m_imgState.nCurOriRow
//	if (m_ImgState.pOriRect)
//	{
//		RECT CutRc = *(m_ImgState.pOriRect);
//		nRet = Hs_Cut(CutRc.left,	CutRc.top,	CutRc.right,	CutRc.bottom);
//		if(nRet!=Ret_Success)
//			return nRet;
//	}
//
//	bool bWcUsed = false;//�еĺ����ڲ�������wc
//	//б�ʽؾ�
//	if(m_ImgInfo.fRescaleSlope!=0.00 && m_ImgInfo.fRescaleIntercept!=0.00)
//	{
//		nRet = Hs_ApplySlope(m_ImgState.bUseSlope);
//		if(nRet!=Ret_Success)
//			return nRet;
//
//		bWcUsed = true;
//	}
//
//	//���wc
//	//if (m_ImgState.CurWc.x != int(m_ImgInfo.fWinWidth) || m_ImgState.CurWc.y != int(m_ImgInfo.fWinCenter) )
//	{
//		if (bWcUsed==false)
//		{
//			nRet = Hs_WinLevel(m_ImgState.CurWc.x,m_ImgState.CurWc.y);
//			if(nRet!=Ret_Success)
//				return nRet;
//		}
//
//	}
//	
//	//Hs_Size������
//
//
//
//	//OverLay
//	if (m_pOriOverlayData)
//	{
//		nRet = Hs_ApplyOverlay(m_ImgState.bShowOverLay);
//		if(nRet!=Ret_Success)
//			return nRet;
//	}
//
//	//��Ӱ(��Ҫ��Ӱ����û��Ӱ��
//	if (m_ImgState.bToSubstract==true && m_ImgState.bSubstracted==false)
//	{
//		//��ȡ��׼֡
//		if(m_pInstance->m_pSubtractImg)
//			Hs_Subtraction(m_pInstance->m_pSubtractImg);
//	}
//
//	//��ת
//	if (m_ImgState.nAngleRotated)
//	{
//		Hs_Rotate(-m_ImgState.nAngleRotated,false);
//	}
//
//	//����
//	if (m_ImgState.bMirror)
//	{
//		Hs_Mirror(false);
//	}
//
//	//�Ƿ���
//	int nSharp = m_ImgState.nSharpValueV.size();
//	for (int i=0;i<nSharp;i++)
//	{
//		Hs_Sharp(m_ImgState.nSharpValueV[i],i==nSharp-1,NULL);//���һ�β�UpdateBits
//	}
//
//	return nRet;
//}

int CHsBaseImg::Hs_Restore()
{
	if(Hs_IsEmpty()==false)//���ԭ��������,�����¶�ȡ����
	{
		Hs_FreeMem();
		int nRet = Ret_Success;

		CHsBaseFile ds;

        QByteArray ba = m_ImgInfo.sFileName.toLatin1();
        nRet = ds.Hs_LoadFile(ba.data());
		if(nRet!=Ret_Success)
			return nRet;

		ds.m_BaseFileInfo.nFrame = ds.Hs_GetFrameCount(m_ImgInfo.pEle);
		ds.Hs_GetImageInfo(m_ImgInfo.pEle,m_ImgInfo,m_ImgInfo.iFrame);
		ds.Hs_OffsetWndLevel(*this);

		return ds.Hs_GetImage(m_ImgInfo.pEle,*this,m_ImgInfo.iFrame);
	}
	else
	{
		return Ret_Success;
	}
}

int CHsBaseImg::Hs_InitImgState()
{
	m_ImgState.CurWc.x = int(m_ImgInfo.fWinWidth);
	m_ImgState.CurWc.y = int(m_ImgInfo.fWinCenter);

	m_ImgState.bShowOverLay = true;//�Ƿ���ʾOverlay.........from xml

	m_ImgState.nCurOriPixCol = m_ImgInfo.nCols;
	m_ImgState.nCurOriPixRow = m_ImgInfo.nRows;
	m_ImgState.bInversed = m_ImgInfo.bInverse;

	m_ImgState.bImgStateFilled = true;

	return Ret_Success;
}
//
//int CHsBaseImg::Hs_AddLocLine( CHsBaseImg *pImgOnMe,int nType)
//{
//	if (pImgOnMe==NULL)
//		return -1;
//
//	//��ͼ������Ч��
//	if(pImgOnMe->m_ImgInfo.ImgLocPara.bValide==false)
//		return -1;
//
//	//��ͼ������Ч��
//	if(m_ImgInfo.ImgLocPara.bValide==false)
//		return -1;
//
//
//	geometry::LineCoord line;
//	int r = CalLocLine(pImgOnMe,line);
//
////	AtlTrace("\r\nx0(%.2f)  y0(%.2f) \r\nx1(%.2f)  y1(%.2f) \r\n",line.x1(),line.y1(),line.x2(),line.y2());
//	//////////////////////////////////////////////////////////////////////////
//	//���Ƕ�λ��ע���嵮����
//	CAnnoBase *pLocLine = new CAnnoLocLine;
//
//	//��mm��λת����Pix��λ
//	//HSPOINT fpt1 = { line.x1()/m_ImgInfo.fPixelSpaceX,	line.y1()/m_ImgInfo.fPixelSpaceY };
//	//HSPOINT fpt2 = { line.x2()/m_ImgInfo.fPixelSpaceX,	line.y2()/m_ImgInfo.fPixelSpaceY };
//
//	HSPOINT fpt1 = { line.x1()/m_ImgInfo.ImgLocPara.fPixSpacingX,	line.y1()/m_ImgInfo.ImgLocPara.fPixSpacingY };
//	HSPOINT fpt2 = { line.x2()/m_ImgInfo.ImgLocPara.fPixSpacingX,	line.y2()/m_ImgInfo.ImgLocPara.fPixSpacingY };
//	
//	//Ϊ�µ�����ע�������ò���
//	pLocLine->AddImgPoint(&fpt1);
//	pLocLine->AddImgPoint(&fpt2);
//	pLocLine->AddImgPoint(NULL);
//
//	//����༭
//	pLocLine->LockAnno(true);
//
//	//��¼һ��˭�������Ķ�λ��
//	pLocLine->SetMarkNumber(LONG(pImgOnMe));
//
//	//���´�ע����
//	if(nType==1)
//	{
//		if (m_LocLineManager1.IsEmpty()==true)
//		{
//			CAnnComposeObjTxt *pComObj1 = new CAnnComposeObjTxt(Tool_LocOne);
//			m_LocLineManager1.AddComposeObjToManager(pComObj1);
//		}
//		m_LocLineManager1.AddAnnObjToManger(NULL,pLocLine);
//	}
//	else if(nType==2)
//	{
//		if (m_LocLineManager2.IsEmpty()==true)
//		{
//			CAnnComposeObjTxt *pComObj2 = new CAnnComposeObjTxt(Tool_LocTwo);
//			m_LocLineManager2.AddComposeObjToManager(pComObj2);
//		}
//
//		m_LocLineManager2.AddAnnObjToManger(NULL,pLocLine);
//	}
//	else 
//	{
//		if (m_LocLineManager3.IsEmpty()==true)
//		{
//			CAnnComposeObjTxt *pComObj3 = new CAnnComposeObjTxt(Tool_LocAll);
//			m_LocLineManager3.AddComposeObjToManager(pComObj3);
//		}
//		m_LocLineManager3.AddAnnObjToManger(NULL,pLocLine);
//	}
//
//	return r;
//
//}
//
//int CHsBaseImg::CalLocLine( CHsBaseImg *pImgOnMe,geometry::LineCoord&line )
//{
//	//��ͼ���γɵ�ƽ��
//	ImagePlaneCoord MyIPC( m_ImgInfo.ImgLocPara.fColmm,			m_ImgInfo.ImgLocPara.fRowmm, 
//					Triple(m_ImgInfo.ImgLocPara.fLeftTopPixX,	m_ImgInfo.ImgLocPara.fLeftTopPixY,	m_ImgInfo.ImgLocPara.fLeftTopPixZ), 
//					Triple(m_ImgInfo.ImgLocPara.fFirstRowCosX,	m_ImgInfo.ImgLocPara.fFirstRowCosY,	m_ImgInfo.ImgLocPara.fFirstRowCosZ),
//					Triple(m_ImgInfo.ImgLocPara.fFirstColCosX,	m_ImgInfo.ImgLocPara.fFirstColCosY, m_ImgInfo.ImgLocPara.fFirstColCosZ));
//
////AtlTrace("\r\n%.2f  %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f ",
//	//	 m_ImgInfo.ImgLocPara.fColmm,			m_ImgInfo.ImgLocPara.fRowmm,
//	//	 m_ImgInfo.ImgLocPara.fLeftTopPixX,	m_ImgInfo.ImgLocPara.fLeftTopPixY,	m_ImgInfo.ImgLocPara.fLeftTopPixZ,
//	//	 m_ImgInfo.ImgLocPara.fFirstRowCosX,	m_ImgInfo.ImgLocPara.fFirstRowCosY,	m_ImgInfo.ImgLocPara.fFirstRowCosZ,
//	//m_ImgInfo.ImgLocPara.fFirstColCosX,	m_ImgInfo.ImgLocPara.fFirstColCosY, m_ImgInfo.ImgLocPara.fFirstColCosZ );
//	//����ͼ���ƽ��
//	ImagePlaneCoord OnMeIPC(pImgOnMe->m_ImgInfo.ImgLocPara.fColmm,			pImgOnMe->m_ImgInfo.ImgLocPara.fRowmm, 
//					Triple(pImgOnMe->m_ImgInfo.ImgLocPara.fLeftTopPixX,		pImgOnMe->m_ImgInfo.ImgLocPara.fLeftTopPixY,	pImgOnMe->m_ImgInfo.ImgLocPara.fLeftTopPixZ), 
//					Triple(pImgOnMe->m_ImgInfo.ImgLocPara.fFirstRowCosX,	pImgOnMe->m_ImgInfo.ImgLocPara.fFirstRowCosY,	pImgOnMe->m_ImgInfo.ImgLocPara.fFirstRowCosZ),
//					Triple(pImgOnMe->m_ImgInfo.ImgLocPara.fFirstColCosX,	pImgOnMe->m_ImgInfo.ImgLocPara.fFirstColCosY,	pImgOnMe->m_ImgInfo.ImgLocPara.fFirstColCosZ));
//
////AtlTrace("\r\n%.2f  %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f \r\n",pImgOnMe->m_ImgInfo.ImgLocPara.fColmm,			pImgOnMe->m_ImgInfo.ImgLocPara.fRowmm,
////pImgOnMe->m_ImgInfo.ImgLocPara.fLeftTopPixX,		pImgOnMe->m_ImgInfo.ImgLocPara.fLeftTopPixY,	pImgOnMe->m_ImgInfo.ImgLocPara.fLeftTopPixZ,
////pImgOnMe->m_ImgInfo.ImgLocPara.fFirstRowCosX,	pImgOnMe->m_ImgInfo.ImgLocPara.fFirstRowCosY,	pImgOnMe->m_ImgInfo.ImgLocPara.fFirstRowCosZ,
////pImgOnMe->m_ImgInfo.ImgLocPara.fFirstColCosX,	pImgOnMe->m_ImgInfo.ImgLocPara.fFirstColCosY,	pImgOnMe->m_ImgInfo.ImgLocPara.fFirstColCosZ);
//
////ƽ�оͳ�
//	if (MyIPC.IsParallel(OnMeIPC)==true)
//		return 0;
//
//	//Ϊ�˽������ʾ����ͼ����,�����Ա�ͼ��Ϊ��׼
//	ImagePlaneCutter cuter(MyIPC);
//	line = cuter.getCuttingLine(OnMeIPC);//�����ཻ��λ(mm)
//
//	return 1;
//	//////////////////////////////////////////////////////////////////////////
///*
//	//����Ϊ�˲��ö�λ�߻���ͼ��֮��.��ͼ����Ĳ��ֽص�:
//	double w = m_ImgInfo.ImgLocPara.fColmm;
//	double h = m_ImgInfo.ImgLocPara.fRowmm;
//
//	RECT rc = {0, 0, int(w) + 1, int(h)+1};
//	POINT pt1 = {int(line.x1()),int(line.y1())};
//	POINT pt2 = {int(line.x2()), int(line.y2())};
//	if(PtInRect(&rc, pt1) && PtInRect(&rc, pt2))
//		return 1;
//
//	if(fabs(line.x1() - line.x2()) < 1.0)
//	{//ƽ��y
//		if(line.x1() < 0.000001 || line.x1() - w > 0.000001
//			||(line.y1() < 0.00001 && line.y2() < 0.000001)
//			||(line.y1()-h > 0.000001 && line.y2() - h > 0.000001))
//			return 0;
//
//		if(line.y1() < 0.000001)
//			line._y1 = 0.0;
//		if(line.y2() < 0.000001)
//			line._y2 = 0.0;
//		if(line.y1() - h > 0.000001)
//			line._y1 = h;
//		if(line.y2() - h > 0.000001)
//			line._y2 = h;
//	}
//	else if(fabs(line.y1() - line.y2()) < 1.0)
//	{//ƽ��x
//		if(line.y1() < 0.000001 || line.y1() - w > 0.000001
//			||(line.x1() < 0.00001 && line.x2() < 0.000001)
//			||(line.x1()-w > 0.000001 && line.x2() - w > 0.000001))
//			return 0;
//
//		if(line.x1() < 0.000001)
//			line._x1 = 0.0;
//		if(line.x1() - w > 0.000001)
//			line._x1 = w;
//		if(line.x2() < 0.000001)
//			line._x2 = 0.0;
//		if(line.x2() - w > 0.000001)
//			line._x2 = w;
//	}
//	else
//	{
//		double k = (line.y1()-line.y2())/(line.x1()-line.x2());
//		HSPOINT apt;
//		POINT pt={0};
//		std::vector<HSPOINT> vec;
//		apt.x = 0.0;
//		apt.y = k*(-line.x1()) + line.y1();
//		pt.x = int(apt.x);
//		pt.y = int(apt.y);
//		if(PtInRect(&rc, pt))
//		{
//			vec.push_back(apt);
//		}
//
//		apt.x = w;
//		apt.y = k*(w - line.x1()) + line.y1();
//		pt.x = int(apt.x);
//		pt.y = int(apt.y);
//		if(PtInRect(&rc, pt))
//		{
//			vec.push_back(apt);
//		}
//
//		apt.y = 0.0;
//		apt.x = 1/k * (-line.y1()) + line.x1();
//		pt.x = int(apt.x);
//		pt.y = int(apt.y);
//		if(PtInRect(&rc, pt))
//		{
//			vec.push_back(apt);
//		}
//
//		apt.y = h;
//		apt.x = 1/k *(h - line.y1()) + line.x1();
//		pt.x = int(apt.x);
//		pt.y = int(apt.y);
//		if(PtInRect(&rc, pt))
//		{
//			vec.push_back(apt);
//		}
//
//		if(vec.size() != 2)
//			return 0;
//
//		HSPOINT apt1 = {line.x1() - vec[0].x, line.y1() - vec[0].y};
//		HSPOINT apt2 = {line.x1() - vec[1].x, line.y1() - vec[1].y};
//		HSPOINT apt3 = {0.00,0.00};
//		if((apt1.x*apt2.x + apt1.y*apt2.y) > 0.000001)
//		{
//			double d1 = sqrt(static_cast<double>((apt1.x-apt3.x)*(apt1.x-apt3.x)+(apt1.y-apt3.y)*(apt1.y-apt3.y)));
//			double d2 = sqrt(static_cast<double>((apt2.x-apt3.x)*(apt2.x-apt3.x)+(apt2.y-apt3.y)*(apt2.y-apt3.y)));
//
//			if(d1- d2 < 0.000001)
//			{
//				line._x1 = vec[0].x;
//				line._y1 = vec[0].y;
//			}
//			else
//			{
//				line._x1 = vec[1].x;
//				line._y1 = vec[1].y;
//			}
//		}
//
//		HSPOINT apt4 = {line.x2() - vec[0].x, line.y2() - vec[0].y};
//		HSPOINT apt5 = {line.x2() - vec[1].x, line.y2() - vec[1].y};
//		if((apt4.x*apt5.x + apt4.y*apt5.y) > 0.000001)
//		{
//			double d1 = sqrt(static_cast<double>((apt4.x-apt3.x)*(apt4.x-apt3.x)+(apt4.y-apt3.y)*(apt4.y-apt3.y)));
//			double d2 = sqrt(static_cast<double>((apt5.x-apt3.x)*(apt5.x-apt3.x)+(apt5.y-apt3.y)*(apt5.y-apt3.y)));
//
//			if(  (d1-d2) < 0.000001  )
//			{
//				line._x2 = vec[0].x;
//				line._y2 = vec[0].y;
//			}
//			else
//			{
//				line._x2 = vec[1].x;
//				line._y2 = vec[1].y;
//			}
//		}
//	}
//*/
//}


int CHsBaseImg::GetMinMaxValue( const RECT &ImgPixRc,long &nMin,long &nMax,bool bUseSlope )
{
	RECT rc;
	rc.left = max((long)0,ImgPixRc.left);
	rc.top = max((long)0,ImgPixRc.top);

	rc.right = min( ImgPixRc.right,	m_ImgState.nCurOriPixCol);
	rc.bottom = min( ImgPixRc.bottom,	m_ImgState.nCurOriPixRow);

	if(rc.left>=rc.right || rc.top>=rc.bottom)
		return Ret_InvalidPara;


	int nRet = 0;
	if (m_ImgInfo.nSamplePerPixel==1)
	{
		if (m_ImgInfo.nPixelRepresentation==0)
		{
			if (m_ImgInfo.nBitsPerPixel==8)
				nRet = GetMaxMinPix((unsigned char**)m_pOriData,rc,nMin, nMax);
			else if(m_ImgInfo.nBitsPerPixel==16)
				nRet = GetMaxMinPix((unsigned short**)m_pOriData,rc,nMin, nMax);
			else if (m_ImgInfo.nBitsPerPixel==32)
				nRet = GetMaxMinPix((unsigned long**)m_pOriData,rc,nMin, nMax);
		}
		else
		{
			if (m_ImgInfo.nBitsPerPixel==8)
				nRet = GetMaxMinPix((char**)m_pOriData,rc,nMin, nMax);
			else if(m_ImgInfo.nBitsPerPixel==16)
				nRet = GetMaxMinPix((short**)m_pOriData,rc,nMin, nMax);
			else if (m_ImgInfo.nBitsPerPixel==32)
				nRet = GetMaxMinPix((long**)m_pOriData,rc,nMin, nMax);
		}

		if (m_ImgInfo.fRescaleSlope!=0.00 && m_ImgInfo.fRescaleIntercept!=0.00 && bUseSlope)
		{
			nMin = nMin*m_ImgInfo.fRescaleSlope + m_ImgInfo.fRescaleIntercept;
			nMax = nMax*m_ImgInfo.fRescaleSlope + m_ImgInfo.fRescaleIntercept;
		}
	}
	else 
	{
		return Ret_InvalidPixFormat;
	}

	return nRet;
}

int CHsBaseImg::Hs_Rotate( int nAngle ,bool bSaveFlag)
{
	int nRet = 0;

	nAngle  = nAngle%360;

	if(nAngle%90 != 0)
		return Ret_InvalidPara;

	if (abs(nAngle) != 90)
	{
		if(nAngle == 180 || nAngle == -180)
		{
			Hs_Rotate(90,bSaveFlag);
			Hs_Rotate(90,bSaveFlag);
		}
		if (nAngle == 270)
		{
			Hs_Rotate(-90,bSaveFlag);
		}
		if (nAngle == -270)
		{
			Hs_Rotate(90,bSaveFlag);
		}

		return 0;
	}

	if (nAngle == -90)//��ת ˳ʱ��
	{
		nRet = TransformImg(1);

        QString t = m_ImgState.sLeftPatientPos;
		m_ImgState.sLeftPatientPos = m_ImgState.sBottomPatientPos;
		m_ImgState.sBottomPatientPos = m_ImgState.sRightPatientPos;
		m_ImgState.sRightPatientPos = m_ImgState.sTopPatientPos;
		m_ImgState.sTopPatientPos = t;


		if (bSaveFlag)
		{
			if(nRet==0)//�Ĳ���λ�ð�
			{
				if (1)//Ϊ������PR�ļ�����һ����תʲô�Ƕ���
				{
					//����ʱѧ�ĵѿ�������ϵ�е�-90�Ⱦ���DICOM�й涨��90��(DICOM�涨˳ʱ��Ϊ����)
					//m_ImgState.nAngleRotated += 90;
					//m_ImgState.nAngleRotated %= 360;

					std::swap(m_ImgInfo.nRows,m_ImgInfo.nCols);//����m_ImgState.nCurOriPixCol;m_ImgState.nCurOriPixRow�Ͳ�Ҫ���ˣ���ΪTransformImg�Ѿ�������

					//����ķ�ʽ�����������·�ʽ
					char l = m_ImgState.cLft;char t = m_ImgState.cTop;char r = m_ImgState.cRgt;char b = m_ImgState.cBtm;
					m_ImgState.cLft = b;
					m_ImgState.cTop = l;
					m_ImgState.cRgt = t;
					m_ImgState.cBtm = r;

					IsRotateOrHorFlip(m_ImgState.cLft,m_ImgState.cTop,m_ImgState.cRgt,m_ImgState.cBtm,m_ImgState.nAngleRotated,m_ImgState.bMirror);
				}
			}

			

		}

		return nRet;
	}
	else if(nAngle == 90)//��ת ��ʱ��
	{
		nRet = TransformImg(2);


		if(nRet==0)
		{
            QString t = m_ImgState.sLeftPatientPos;
			m_ImgState.sLeftPatientPos = m_ImgState.sTopPatientPos;
			m_ImgState.sTopPatientPos = m_ImgState.sRightPatientPos;
			m_ImgState.sRightPatientPos = m_ImgState.sBottomPatientPos;
			m_ImgState.sBottomPatientPos = t;


			if (bSaveFlag)
			{
				if (1)//Ϊ������PR�ļ�����һ����תʲô�Ƕ���
				{
					//����ʱѧ�ĵѿ�������ϵ�е�90�Ⱦ���DICOM�й涨��270��(DICOM�涨˳ʱ��Ϊ����)
					//m_ImgState.nAngleRotated += 270;
					//m_ImgState.nAngleRotated %= 360;

					std::swap(m_ImgInfo.nRows,m_ImgInfo.nCols);//����m_ImgState.nCurOriPixCol;m_ImgState.nCurOriPixRow�Ͳ�Ҫ���ˣ���ΪTransformImg�Ѿ�������

					//����ķ�ʽ�����������·�ʽ
					char l = m_ImgState.cLft;char t = m_ImgState.cTop;char r = m_ImgState.cRgt;char b = m_ImgState.cBtm;
					m_ImgState.cLft = t;
					m_ImgState.cTop = r;
					m_ImgState.cRgt = b;
					m_ImgState.cBtm = l;

					IsRotateOrHorFlip(m_ImgState.cLft,m_ImgState.cTop,m_ImgState.cRgt,m_ImgState.cBtm,m_ImgState.nAngleRotated,m_ImgState.bMirror);

				}
			}

			return nRet;
		}
	}

	return Ret_UnSupportPara;

}

int CHsBaseImg::Hs_Flip()
{
	int nRet = TransformImg(3);
	if(nRet==0)//�Ĳ���λ�ð�
	{
        QString t = m_ImgState.sTopPatientPos;
		m_ImgState.sTopPatientPos = m_ImgState.sBottomPatientPos;
		m_ImgState.sBottomPatientPos = t;

		if (1)//Ϊ������PR�ļ�����¼�������
		{
			//�൱��ת180�������Ҿ���һ��
			m_ImgState.nAngleRotated += 180;
			m_ImgState.nAngleRotated %= 360;
			m_ImgState.bMirror = true;

			//����ķ�ʽ�����������·�ʽ
			//����ķ�ʽ�����������·�ʽ
			std::swap(m_ImgState.cTop,m_ImgState.cBtm);
			IsRotateOrHorFlip(m_ImgState.cLft,m_ImgState.cTop,m_ImgState.cRgt,m_ImgState.cBtm,m_ImgState.nAngleRotated,m_ImgState.bMirror);

		}
	}

	return nRet;
}

int CHsBaseImg::Hs_Mirror(bool bSaveFlag)
{
	int nRet = TransformImg(4);
	if(nRet==0)//�Ĳ���λ�ð�
	{
        QString t = m_ImgState.sLeftPatientPos;
		m_ImgState.sLeftPatientPos = m_ImgState.sRightPatientPos;
		m_ImgState.sRightPatientPos = t;


		if (bSaveFlag)//Ϊ������PR�ļ�����¼�������
		{
			//m_ImgState.bMirror = true;

			//����ķ�ʽ�����������·�ʽ
			std::swap(m_ImgState.cLft,m_ImgState.cRgt);
			IsRotateOrHorFlip(m_ImgState.cLft,m_ImgState.cTop,m_ImgState.cRgt,m_ImgState.cBtm,m_ImgState.nAngleRotated,m_ImgState.bMirror);
		}
	}

	return nRet;
}

int CHsBaseImg::TransformImg( int nType )
{
	if(m_pOriData==NULL)//��ʱ��û���أ�ҲҪ��ת���ᵼ�´��󣬴˴�����
		return 0;

	int nRet = 0;

	long pNewOriData = 0;
	unsigned long nNewRow = 0;
	unsigned long nNewCol = 0;

	unsigned long tCurRow = m_ImgState.nCurOriPixRow;
	unsigned long tCurCol = m_ImgState.nCurOriPixCol;

	if (m_ImgInfo.nSamplePerPixel==1)
	{
		if (m_ImgInfo.nBitsPerPixel==8)
			nRet = TransformPix((BYTE**)m_pOriData,		m_ImgState.nCurOriPixRow,		m_ImgState.nCurOriPixCol, nType,		pNewOriData,	nNewRow,	nNewCol);
		else if(m_ImgInfo.nBitsPerPixel==16)
			nRet = TransformPix((short**)m_pOriData,		m_ImgState.nCurOriPixRow,		m_ImgState.nCurOriPixCol, nType,		pNewOriData,	nNewRow,	nNewCol);
		else if (m_ImgInfo.nBitsPerPixel==32)
			nRet = TransformPix((long**)m_pOriData,		m_ImgState.nCurOriPixRow,		m_ImgState.nCurOriPixCol, nType,		pNewOriData,	nNewRow,	nNewCol);
	}
	else if (m_ImgInfo.nSamplePerPixel==3)
	{
		if (m_ImgInfo.nBitsPerPixel==24)
			nRet = TransformPix((MYDATA24**)m_pOriData,		m_ImgState.nCurOriPixRow,		m_ImgState.nCurOriPixCol,	nType,		pNewOriData,	nNewRow,	nNewCol);
	}

	ArrayFree((void**)m_pOriData);
	m_pOriData = (BYTE**)pNewOriData;

	m_ImgState.nCurOriPixRow = nNewRow;
	m_ImgState.nCurOriPixCol = nNewCol;

	//////////////////////////////////////////////////////////////////////////
	//Overlay
	if(m_pOriOverlayData)
	{
		long pNewOverlayData = 0;
		TransformPix((BYTE**)m_pOriOverlayData, tCurRow,	tCurCol, nType,		pNewOverlayData,	nNewRow,	nNewCol);
		ArrayFree((void**)m_pOriOverlayData);
		m_pOriOverlayData = (BYTE**)pNewOverlayData;
	}

	//////////////////////////////////////////////////////////////////////////

	//m_bDrawInfoFilled = false;
	//m_bPrintInfoFilled = false;

	return nRet;
}

int CHsBaseImg::Hs_CreateDisplayBits( RECT ImgRc,BYTE* &pRetBits ,unsigned long &nBitsRow,unsigned long &nBitsCol)
{
	
	if (pRetBits)
	{
		delete []pRetBits;
		pRetBits = NULL;
	}

	int t1 = ImgRc.bottom-ImgRc.top;
	int t2 = ImgRc.right - ImgRc.left;

	if(t1<=0 || t2<=0)
		return Ret_InvalidPara;

	int nRet = Ret_Success;

	unsigned long nOriRow = t1;
	unsigned long nOriCol = t2;

	//ÿ������ռ�����ֽ�?
	int nBytePerPix = m_ImgInfo.nBitsPerPixel/8;

	//�൱����ImgRc��ͼ�����ؾ���Rc�Ľ���:
	int nMinRow = max((long)0,ImgRc.top);		int nMaxRow = min(m_ImgState.nCurOriPixRow,  ImgRc.bottom);
	int nMinCol = max((long)0,ImgRc.left);	int nMaxCol = min(m_ImgState.nCurOriPixCol, ImgRc.right);
	

	//��ʼ��pOriData��˫���Բ�ֵ
	if (nBytePerPix==1)
	{
		BYTE** pOriData = (BYTE**)ArrayNew(nOriRow,nOriCol,nBytePerPix,&nOriRow,&nOriCol);
		//����������Ĭ�ϳ���Сֵ0
		
		BYTE **pAllOriData = (BYTE**)m_pOriData;

		//��ΪImgRc���m_pOriData����Խ��,���Ǵ˴��������,����,Ҫ�Լ��ȸ��Ƴ�һ��pOriData
		long tMinR=100000;long tMaxR=-100000;long tMinC=100000;long tMaxC=-100000;
		for (int r=nMinRow; r<nMaxRow; r++)	//ֻ�Խ��������ظ���
		{
			for (int c=nMinCol;  c<nMaxCol;  c++)
			{
				pOriData[r-ImgRc.top][c-ImgRc.left] = pAllOriData[r][c];
				tMinR = min(tMinR,r-ImgRc.top);
				tMaxR = max(tMaxR,r-ImgRc.top);
				tMinC = min(tMinC,c-ImgRc.left);
				tMaxC = max(tMaxC,c-ImgRc.left);
			}

		}


		//׼����ֵ���buffer
		BYTE** pDataSized = (BYTE**)ArrayNew(nBitsRow,nBitsCol,nBytePerPix,&nBitsRow,&nBitsCol);
		ImgSizeOneSampleResample(pDataSized,nBitsRow,nBitsCol,pOriData,nOriRow,nOriCol,0,0,nOriRow,nOriCol);

		//��Ч��
		if (m_bSharpOri == false)
		{
			int nSharp = m_ImgState.nSharpValueV.size();
			for(int i=0;i<nSharp;i++)
			{
				BYTE** pNewData = ImgSharp(pDataSized,nBitsRow,nBitsCol,m_ImgState.nSharpValueV[i],m_hWnd);
				if(pDataSized)
				{
					ArrayFree((void**)pDataSized);
					pDataSized = NULL;
					pDataSized = pNewData;
				}
			}
		}


		tMinR = long(tMinR*nBitsRow*1.00/nOriRow);
		tMaxR = long(tMaxR*nBitsRow*1.00/nOriRow);
		tMinC = long(tMinC*nBitsCol*1.00/nOriCol);
		tMaxC = long(tMaxC*nBitsCol*1.00/nOriCol);


		//��pDataSized���pRetBits
		RECT ValidRc = {tMinC,tMinR,tMaxC,tMaxR};

		nRet = UpdateBits(nBitsRow,nBitsCol,(BYTE**)pDataSized,NULL,pRetBits,&ValidRc);
		if(pOriData) ArrayFree((void**)pOriData);
		if(pDataSized) ArrayFree((void**)pDataSized,0);
	}
	else if (nBytePerPix==2)
	{
		if(m_ImgInfo.nPixelRepresentation==1)
		{
			short** pOriData = (short**)ArrayNew(nOriRow,nOriCol,nBytePerPix,&nOriRow,&nOriCol);
			short **pAllOriData = (short**)m_pOriData;

			//��ΪImgRc���m_pOriData����Խ��,���Ǵ˴��������,����,Ҫ�Լ��ȸ��Ƴ�һ��pOriData
			long tMinR=100000;long tMaxR=-100000;long tMinC=100000;long tMaxC=-100000;
			for (int r=nMinRow; r<nMaxRow; r++)	//ֻ�Խ��������ظ���
			{
				for (int c=nMinCol;  c<nMaxCol;  c++)
				{
					pOriData[r-ImgRc.top][c-ImgRc.left] = pAllOriData[r][c];
					tMinR = min(tMinR,r-ImgRc.top);
					tMaxR = max(tMaxR,r-ImgRc.top);
					tMinC = min(tMinC,c-ImgRc.left);
					tMaxC = max(tMaxC,c-ImgRc.left);
				}
			}


			//׼����ֵ���buffer
			short** pDataSized = (short**)ArrayNew(nBitsRow,nBitsCol,nBytePerPix,&nBitsRow,&nBitsCol);
			ImgSizeOneSampleResample(pDataSized,nBitsRow,nBitsCol,pOriData,nOriRow,nOriCol,0,0,nOriRow,nOriCol);

			//��Ч��
			if (m_bSharpOri == false)
			{
				int nSharp = m_ImgState.nSharpValueV.size();
				for(int i=0;i<nSharp;i++)
				{
					BYTE** pNewData = ImgSharp((BYTE**)pDataSized,nBitsRow,nBitsCol,m_ImgState.nSharpValueV[i],m_hWnd);
					if(pDataSized)
					{
						ArrayFree((void**)pDataSized);
						pDataSized = NULL;
						pDataSized = (short**)pNewData;
					}
				}
			}


			tMinR = long(tMinR*nBitsRow*1.00/nOriRow);
			tMaxR = long(tMaxR*nBitsRow*1.00/nOriRow);
			tMinC = long(tMinC*nBitsCol*1.00/nOriCol);
			tMaxC = long(tMaxC*nBitsCol*1.00/nOriCol);

			//��pDataSized���pRetBits
			RECT ValidRc = {tMinC,tMinR,tMaxC,tMaxR};
			nRet = UpdateBits(nBitsRow,nBitsCol,(BYTE**)pDataSized,NULL,pRetBits,&ValidRc);
			if(pOriData) ArrayFree((void**)pOriData);
			if(pDataSized) ArrayFree((void**)pDataSized,0);

			//AtlTrace("\r\nBytePerPix==2,Representation==1");
		}
		else
		{
			unsigned short** pOriData = (unsigned short**)ArrayNew(nOriRow,nOriCol,nBytePerPix,&nOriRow,&nOriCol);

			unsigned short** pAllOriData = (unsigned short**)m_pOriData;
			
			long tMinR=100000;long tMaxR=-100000;long tMinC=100000;long tMaxC=-100000;
			for (int r=nMinRow; r<nMaxRow; r++)	//ֻ�Խ��������ظ���
			{
				for (int c=nMinCol;  c<nMaxCol;  c++)
				{
					pOriData[r-ImgRc.top][c-ImgRc.left] = pAllOriData[r][c];
					tMinR = min(tMinR,r-ImgRc.top);
					tMaxR = max(tMaxR,r-ImgRc.top);
					tMinC = min(tMinC,c-ImgRc.left);
					tMaxC = max(tMaxC,c-ImgRc.left);
				}
			}


			//׼����ֵ���buffer
			unsigned short** pDataSized = (unsigned short**)ArrayNew(nBitsRow,nBitsCol,nBytePerPix,&nBitsRow,&nBitsCol);
			ImgSizeOneSampleResample(pDataSized,nBitsRow,nBitsCol,pOriData,nOriRow,nOriCol,0,0,nOriRow,nOriCol);

			//��Ч��
			if (m_bSharpOri == false)
			{
				int nSharp = m_ImgState.nSharpValueV.size();
				for(int i=0;i<nSharp;i++)
				{
					BYTE** pNewData = ImgSharp((BYTE**)pDataSized,nBitsRow,nBitsCol,m_ImgState.nSharpValueV[i],m_hWnd);
					if(pDataSized)
					{
						ArrayFree((void**)pDataSized);
						pDataSized = NULL;
						pDataSized = (unsigned short**)pNewData;
					}
				}
			}


			tMinR = long(tMinR*nBitsRow*1.00/nOriRow);
			tMaxR = long(tMaxR*nBitsRow*1.00/nOriRow);
			tMinC = long(tMinC*nBitsCol*1.00/nOriCol);
			tMaxC = long(tMaxC*nBitsCol*1.00/nOriCol);


			//��pDataSized���pRetBits
			RECT ValidRc = {tMinC,tMinR,tMaxC,tMaxR};
			nRet = UpdateBits(nBitsRow,nBitsCol,(BYTE**)pDataSized,NULL,pRetBits,&ValidRc);
			if(pOriData) ArrayFree((void**)pOriData);
			if(pDataSized) ArrayFree((void**)pDataSized,0);

			//AtlTrace("\r\nBytePerPix==2,Representation==0");
		}
	}
	else if (nBytePerPix==4)
	{
		if(m_ImgInfo.nPixelRepresentation==1)
		{
			long** pOriData = (long**)ArrayNew(nOriRow,nOriCol,nBytePerPix,&nOriRow,&nOriCol);
			long **pAllOriData = (long**)m_pOriData;

			long tMinR=100000;long tMaxR=-100000;long tMinC=100000;long tMaxC=-100000;
			for (int r=nMinRow; r<nMaxRow; r++)	//ֻ�Խ��������ظ���
			{
				for (int c=nMinCol;  c<nMaxCol;  c++)
				{
					pOriData[r-ImgRc.top][c-ImgRc.left] = pAllOriData[r][c];
					tMinR = min(tMinR,r-ImgRc.top);
					tMaxR = max(tMaxR,r-ImgRc.top);
					tMinC = min(tMinC,c-ImgRc.left);
					tMaxC = max(tMaxC,c-ImgRc.left);
				}
			}


			//׼����ֵ���buffer
			long** pDataSized = (long**)ArrayNew(nBitsRow,nBitsCol,nBytePerPix,&nBitsRow,&nBitsCol);
			ImgSizeOneSampleResample(pDataSized,nBitsRow,nBitsCol,pOriData,nOriRow,nOriCol,0,0,nOriRow,nOriCol);

			//��Ч��
			if (m_bSharpOri == false)
			{
				int nSharp = m_ImgState.nSharpValueV.size();
				for(int i=0;i<nSharp;i++)
				{
					BYTE** pNewData = ImgSharp((BYTE**)pDataSized,nBitsRow,nBitsCol,m_ImgState.nSharpValueV[i],m_hWnd);
					if(pDataSized)
					{
						ArrayFree((void**)pDataSized);
						pDataSized = NULL;
						pDataSized = (long**)pNewData;
					}
				}
			}


			tMinR = long(tMinR*nBitsRow*1.00/nOriRow);
			tMaxR = long(tMaxR*nBitsRow*1.00/nOriRow);
			tMinC = long(tMinC*nBitsCol*1.00/nOriCol);
			tMaxC = long(tMaxC*nBitsCol*1.00/nOriCol);

			//��pDataSized���pRetBits
			RECT ValidRc = {tMinC,tMinR,tMaxC,tMaxR};

			nRet = UpdateBits(nBitsRow,nBitsCol,(BYTE**)pDataSized,NULL,pRetBits,&ValidRc);
			if(pOriData) ArrayFree((void**)pOriData);
			if(pDataSized) ArrayFree((void**)pDataSized,0);
		}
		else
		{
			unsigned long** pOriData = (unsigned long**)ArrayNew(nOriRow,nOriCol,nBytePerPix,&nOriRow,&nOriCol);
			unsigned long **pAllOriData = (unsigned long**)m_pOriData;

			long tMinR=100000;long tMaxR=-100000;long tMinC=100000;long tMaxC=-100000;
			for (int r=nMinRow; r<nMaxRow; r++)	//ֻ�Խ��������ظ���
			{
				for (int c=nMinCol;  c<nMaxCol;  c++)
				{
					pOriData[r-ImgRc.top][c-ImgRc.left] = pAllOriData[r][c];
					tMinR = min(tMinR,r-ImgRc.top);
					tMaxR = max(tMaxR,r-ImgRc.top);
					tMinC = min(tMinC,c-ImgRc.left);
					tMaxC = max(tMaxC,c-ImgRc.left);
				}
			}


			//׼����ֵ���buffer
			unsigned long** pDataSized = (unsigned long**)ArrayNew(nBitsRow,nBitsCol,nBytePerPix,&nBitsRow,&nBitsCol);
			ImgSizeOneSampleResample(pDataSized,nBitsRow,nBitsCol,pOriData,nOriRow,nOriCol,0,0,nOriRow,nOriCol);

			//��Ч��
			if (m_bSharpOri == false)
			{
				int nSharp = m_ImgState.nSharpValueV.size();
				for(int i=0;i<nSharp;i++)
				{
					BYTE** pNewData = ImgSharp((BYTE**)pDataSized,nBitsRow,nBitsCol,m_ImgState.nSharpValueV[i],m_hWnd);
					if(pDataSized)
					{
						ArrayFree((void**)pDataSized);
						pDataSized = NULL;
						pDataSized = (unsigned long**)pNewData;
					}
				}
			}


			tMinR = long(tMinR*nBitsRow*1.00/nOriRow);
			tMaxR = long(tMaxR*nBitsRow*1.00/nOriRow);
			tMinC = long(tMinC*nBitsCol*1.00/nOriCol);
			tMaxC = long(tMaxC*nBitsCol*1.00/nOriCol);

			//��pDataSized���pRetBits
			RECT ValidRc = {tMinC,tMinR,tMaxC,tMaxR};
			nRet = UpdateBits(nBitsRow,nBitsCol,(BYTE**)pDataSized,NULL,pRetBits,&ValidRc);
			if(pOriData) ArrayFree((void**)pOriData);
			if(pDataSized) ArrayFree((void**)pDataSized,0);
		}
	}
	else
	{
		nRet = Ret_UnSupportPara;
	}


	return nRet;
}
SIZE CHsBaseImg::Hs_GetImgSize( bool bDisplaySize/*=false*/ )
{
	SIZE sz = {m_ImgState.nCurOriPixCol,m_ImgState.nCurOriPixRow};
	if(bDisplaySize==true)
	{
		sz.cx = m_ImgState.nCurBitsCol;
		sz.cy = m_ImgState.nCurBitsRow;
	}

	return sz;
}
int CHsBaseImg::Hs_Subtraction( CHsBaseImg *pImgB)
{
	//�������
	if(pImgB==NULL)
		return Ret_InvalidImgForSubtract;

	if (pImgB->m_ImgInfo.iHighBit != m_ImgInfo.iHighBit)
		return Ret_InvalidImgForSubtract;
	
	if(pImgB->m_ImgState.nCurOriPixRow!=m_ImgState.nCurOriPixRow || pImgB->m_ImgState.nCurOriPixCol!=m_ImgState.nCurOriPixCol)
		return Ret_InvalidImgForSubtract;

	//��ʼ
	int nRet = Ret_Success;

	unsigned long nRow = m_ImgState.nCurOriPixRow;
//1.2.840.113820.0.1.20120808.142617.0
	if (m_ImgInfo.nSamplePerPixel==1)
	{
		if (m_ImgInfo.nBitsPerPixel==8)
		{
			unsigned char nMin = 0;//(unsigned char)(pImgB->m_ImgInfo.nSmallestPixelValue);
			unsigned char nMax = (1<<m_ImgInfo.nBitStored) - 1;//(unsigned char)(pImgB->m_ImgInfo.nLargestPixelValue);

			nRet = ImgSubstract((unsigned char**)m_pOriData,(unsigned char**)pImgB->m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,nMin,nMax);

			if(m_ImgState.IsUserSubstractWc == false)
			{
				m_ImgState.CurWc.x = nMax/4;
				m_ImgState.CurWc.y = (nMax)/2;	
			}
		}
		else if(m_ImgInfo.nBitsPerPixel==16)
		{
			unsigned short nMin = 0;
			unsigned short nMax = (1<<m_ImgInfo.nBitStored) - 1;

			nRet = ImgSubstract((unsigned short**)m_pOriData,(unsigned short**)pImgB->m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,nMin,nMax);

			if(m_ImgState.IsUserSubstractWc == false)//��Ӱ֮������û����ڹ�wc�������Ϊtrue�ˣ��Ͳ����������Զ�����wcֵ��
			{
				m_ImgState.CurWc.x = nMax/8;
				m_ImgState.CurWc.y = (nMax)/2;
			}

		}
		else if (m_ImgInfo.nBitsPerPixel==32)
		{
			unsigned long nMin = 0;
			unsigned long nMax = (1<<m_ImgInfo.nBitStored) - 1;

			nRet = ImgSubstract((unsigned long**)m_pOriData,(unsigned long**)pImgB->m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,nMin,nMax);

			if(m_ImgState.IsUserSubstractWc == false)
			{
				m_ImgState.CurWc.x = nMax/32;
				m_ImgState.CurWc.y = (nMax)/2;
			}
		}
	}
	else if (m_ImgInfo.nSamplePerPixel==3)
	{
		//������
	}

	m_ImgState.bSubstracted = true;	

	
	return nRet;
}


bool CHsBaseImg::Hs_IsWholeImgSized()
{
	return m_ImgState.bWholeImgSized;
}

bool CHsBaseImg::Hs_IsEmpty()
{
	return m_pOriData==NULL ? true:false;
}


RECT CHsBaseImg::Hs_GetSizeRcOnOriImg()
{
	if(m_pPreSizeRc==NULL)
	{
		RECT rc = {0,0,m_ImgState.nCurOriPixCol,m_ImgState.nCurOriPixRow};
		return rc;
	}

	return *m_pPreSizeRc;
}

int CHsBaseImg::Hs_FillPixel( RECT rcInCurImg,int nNewWidth,int nNewHeight )
{
	if (rcInCurImg.left<0 || rcInCurImg.top<0 || rcInCurImg.right>=m_ImgState.nCurOriPixCol || rcInCurImg.bottom>=m_ImgState.nCurOriPixRow)
		return Ret_InvalidPara;
	
	int nSelectPixW = rcInCurImg.right - rcInCurImg.left;
	int nSelectPixH = rcInCurImg.bottom - rcInCurImg.top;

	if (nNewWidth < nSelectPixW || nNewHeight < nSelectPixH)//��Ȼ�ǲ����أ�Ŀ���ܵñ���Ϊ���ϵ����ش��
		return Ret_InvalidPara;

	int nBytePerPix = m_ImgInfo.nBitsPerPixel/8;
	if(nBytePerPix!=1 && nBytePerPix!=2 && nBytePerPix!=3 && nBytePerPix!=4)
	{
		return Ret_UnSupportPara;
	}

	BYTE **pNewOriData = (BYTE**)ArrayNew(nNewHeight,nNewWidth,nBytePerPix);

	long v = m_ImgInfo.sPhotometric.compare("MONOCHROME1")==0 ? m_ImgInfo.nLargestPixelValue : m_ImgInfo.nSmallestPixelValue;

	if (nBytePerPix==1)
	{
		FillPixel((BYTE**)m_pOriData,(BYTE**)pNewOriData,rcInCurImg,nSelectPixW,nSelectPixH,nNewWidth,nNewHeight,nBytePerPix,BYTE(v));
	}
	else if (nBytePerPix==2)
	{
		FillPixel((short**)m_pOriData,(short**)pNewOriData,rcInCurImg,nSelectPixW,nSelectPixH,nNewWidth,nNewHeight,nBytePerPix,short(v));
	}
	else if (nBytePerPix==3)
	{
		MYDATA24 nSmallestValue;
		nSmallestValue.pData[0] = nSmallestValue.pData[1] = nSmallestValue.pData[2] = 0;
		FillPixel((MYDATA24**)m_pOriData,(MYDATA24**)pNewOriData,rcInCurImg,nSelectPixW,nSelectPixH,nNewWidth,nNewHeight,nBytePerPix,nSmallestValue);
	}
	else if (nBytePerPix==4)
	{
		FillPixel((long**)m_pOriData,(long**)pNewOriData,rcInCurImg,nSelectPixW,nSelectPixH,nNewWidth,nNewHeight,nBytePerPix,v);
	}

	ArrayFree((void**)m_pOriData);
	m_pOriData = NULL;
	m_pOriData = pNewOriData;

	//Overlay
	if (m_pOriOverlayData)
	{
		BYTE **pNewOriOverlayData = (BYTE**)ArrayNew(nNewHeight,nNewWidth,1);

		BYTE b = 0;
		FillPixel((BYTE**)m_pOriOverlayData,(BYTE**)pNewOriOverlayData,rcInCurImg,nSelectPixW,nSelectPixH,nNewWidth,nNewHeight,1,b);

		ArrayFree((void**)m_pOriOverlayData);
		m_pOriOverlayData = NULL;

		m_pOriOverlayData = pNewOriOverlayData;
	}


	//�޸ĵ�ǰ����������
	m_ImgState.nCurOriPixRow = nNewHeight;
	m_ImgState.nCurOriPixCol = nNewWidth;

	return 0;

}


int CHsBaseImg::Hs_GetPatientPos( double x, double y, double z,QString& sLeft, QString& sRight )
{//P=�󱳣�F=�ţ�A=ǰ�棬L=��,R=��,H=ͷ
	sLeft = "";
	sRight = "";	

	if( abs(x) == abs(y)  && abs(x)==abs(z) )   //����ֵ���
		return Ret_InvalidPara;

	if( abs(x)>1 || abs(y)>1  || abs(z)>1 )    //��ֵ����1
		return Ret_InvalidPara;

	if ( abs(x)>=abs(y) && abs(x)>=abs(z)  ) 
	{
		if( x>0 )
		{
			if( abs(y)==0 && abs(z)==0 )
			{ sLeft = "R"; sRight = "L"; } //1,0,0

			if( abs(y)>= abs(z) )
			{
				if( y>0 )
				{ sLeft = "Ra"; sRight = "Lp";}   //1,0.09,0.01
				if( y<0 )
				{ sLeft = "Rp"; sRight = "La";}  //1,-0.09,0.01
			}

			if( abs(z)>abs(y) )  
			{
				if( z>0 )
				{ sLeft = "Rf"; sRight = "Lh";}  //1,0.01,0.09
				if( z<0 )
				{ sLeft = "Rh"; sRight = "Lf";}  //1,0.01,-0.09
			}
		}
		if( x<0 )
		{
			if( abs(y)==0 && abs(z)==0 )
			{ sLeft = "L"; sRight = "R";} // -1,0,0

			if( abs(y)>= abs(z) )
			{
				if( y>0 )
				{ sLeft = "La"; sRight = "Rp"; }  // -1,0.09,0.01
				if( y<0 )
				{ sLeft = "Lp"; sRight = "Ra"; } // -1,-0.09,0.01
			}

			if( abs(z)>abs(y) )  
			{
				if( z>0 )
				{ sLeft = "Lf"; sRight = "Rh"; }	// -1,0.01,0.09
				if( z<0 )
				{ sLeft = "Lh"; sRight = "Rf"; }   // -1,0.01,-0.09
			}
		}
	}


	if ( abs(y)>abs(x) && abs(y)>=abs(z)  ) 
	{
		if( y>0 )
		{
			if( abs(x)==0 && abs(z)==0 )
			{ sLeft = "A"; sRight = "P";}  //0,1,0

			if( abs(x)>= abs(z) )
			{
				if( x>0 )
				{ sLeft = "Ar"; sRight = "Pl";}  // 0.09,1,0.01
				if( x<0 )
				{ sLeft = "Al"; sRight = "Pr";}   // -0.09,1,0.01
			}

			if( abs(z)>abs(x) )   
			{
				if( z>0 )
				{ sLeft = "Af"; sRight = "Ph";}   //0.01,1, 0.09
				if( z<0 )
				{ sLeft = "Ah"; sRight = "Pf";}   //0.01,1,-0.09
			}
		}
		if( y<0 )
		{
			if( abs(x)==0 && abs(z)==0 )
			{ sLeft = "P"; sRight = "A";}  //0,-1,0

			if( abs(x)>= abs(z) )
			{
				if( x>0 )
				{ sLeft = "Pr"; sRight = "Al";}  //0.09,-1,0.01
				if( x<0 )
				{ sLeft = "Pl"; sRight = "Ar";}  //-0.09,-1,0.01
			}

			if( abs(z)>abs(x) )   
			{
				if( z>0 )
				{ sLeft = "Pf"; sRight = "Ah";}  //0.01,-1,0.09
				if( z<0 )
				{ sLeft = "Ph"; sRight = "Af";}  //0.01,-1,-0.09
			}	
		}

	}


	if ( abs(z)>abs(y) && abs(z)>abs(x)  ) 
	{
		if( z>0 )
		{
			if( abs(y)==0 && abs(x)==0 )
			{ sLeft = "F"; sRight = "H"; }  //0,0,1

			if( abs(x)>=abs(y) ) 
			{
				if( x>0 )
				{ sLeft = "Fr"; sRight = "Hl"; }  //0.09,0.01,1
				if( x<0 )
				{ sLeft = "Fl"; sRight = "Hr"; }  //-0.09,0.01,1
			}

			if( abs(y)>abs(x) )
			{
				if( y>0 )
				{ sLeft = "Fa"; sRight = "Hp"; }  //0.01,0.09,1
				if( y<0 )
				{ sLeft = "Fp"; sRight = "Ha"; }  //0.01,-0.09,1
			}
		}

		if( z<0 )
		{
			if( abs(y)==0 && abs(x)==0 )
			{ sLeft = "H"; sRight = "F"; }  //0,0,-1

			if( abs(x)>=abs(y) ) 
			{
				if( x>0 )
				{ sLeft = "Hr"; sRight = "Fl"; }  //0.09,0.01,-1
				if( x<0 )
				{ sLeft = "Hl"; sRight = "Fr"; }  //-0.09,0.01,-1
			}

			if( abs(y)>abs(x) )
			{
				if( y>0 )
				{ sLeft = "Ha"; sRight = "Fp"; }  //0.01,0.09,-1
				if( y<0 )
				{ sLeft = "Hp"; sRight = "Fa"; }  //0.01,-0.09,-1
			}

		}

	}


	return 0;

}

int CHsBaseImg::Hs_Inverse()
{
	m_ImgState.bInversed = !(m_ImgState.bInversed);
	Hs_WinLevel(0,0,true);

	return 0;
}

int CHsBaseImg::Hs_GetCurLutName( QString &sName )
{
	sName = "";
    int n = m_pLutV.size();

	LutData *pSelect = NULL;
    for (int i=0;i<n;i++)
	{
		if(m_pLutV[i]->bUse == true)
		{
			sName = m_pLutV[i]->sName;

			return 0;
		}
	}

	return 0;
}

int CHsBaseImg::Hs_GetCtValuePoint( POINT ImgPt,QString &sTxt )
{
	sTxt = "��Ч����";

	if (m_pOriData == NULL)
		return Ret_EmptyImage;

	if (ImgPt.x < 0 || ImgPt.x >= m_ImgInfo.nCols || ImgPt.y < 0 || ImgPt.y >= m_ImgInfo.nRows)
		return Ret_InvalidPara;

	double fPixValue = Hs_GetPixelValue(ImgPt.y,ImgPt.x);

	if(m_ImgInfo.fPixelSpaceX > 0.000001 && m_ImgInfo.fPixelSpaceY > 0.000001)
        sTxt = QString("[%1mm,	%2mm] = %3").arg(QString::number(ImgPt.x*m_ImgInfo.fPixelSpaceX, 'g', 2)).arg(QString::number(ImgPt.y*m_ImgInfo.fPixelSpaceY, 'g', 2)).arg(QString::number(fPixValue, 'g', 2));
	else
        sTxt = QString("[%1,	%2] = %3f").arg(ImgPt.x).arg(ImgPt.y).arg(QString::number(fPixValue, 'g', 2));

	return Ret_Success;
}

int CHsBaseImg::Hs_GetCtValueEllipse( RECT ImgRc ,QString &sTxt)
{
	sTxt = "��Ч����";

	if (m_pOriData == NULL)
		return Ret_EmptyImage;

	if (ImgRc.left < 0 || ImgRc.right >= m_ImgInfo.nCols || ImgRc.top < 0 || ImgRc.bottom >= m_ImgInfo.nRows)
		return Ret_InvalidPara;

	HRGN hGn = ::CreateEllipticRgn(ImgRc.left, ImgRc.top, ImgRc.right,ImgRc.bottom);

	bool bInit=false;
	double fMin = 0.00;
	double fMax = 0.00;
	double fPixValue = 0;
	long nPixCount = 0;
	double fTotle = 0;

	//short ** pData = (short**)m_pOriData;
	//�����ֵ����Сֵ���������������ܺ�
	for (int r=ImgRc.top;r<ImgRc.bottom; r++)
	{
		for (int c=ImgRc.left;c<ImgRc.right; c++)
		{
			if(PtInRegion(hGn, c, r))
			{
				fPixValue = Hs_GetPixelValue(r,c);

				if (bInit == false)
				{
					fMin = fPixValue;
					fMax = fPixValue;

					bInit = true;
				}
				else
				{
					fMin = min(fMin,fPixValue);
					fMax = max(fMax,fPixValue);
				}

				nPixCount++;
				fTotle += fPixValue;
			}
		}
	}

	if (nPixCount == 0)
	{
		return Ret_InvalidPara;
	}
	//��ƽ��ֵ(�����ܺ�/������)
	double fAverage = fTotle/nPixCount;

	//��������ƽ��ֵ��Ȼ������ƽ�����ٽ����е�ƽ�����
	double fSubPowAll = 0.00;
	for (int r=ImgRc.top;r<ImgRc.bottom; r++)
	{
		for (int c=ImgRc.left;c<ImgRc.right; c++)
		{
			if(PtInRegion(hGn, c, r))
			{
				fPixValue = Hs_GetPixelValue(r,c);

				fSubPowAll += ( (fPixValue - fAverage) * (fPixValue - fAverage) );
			}
		}
	}

	fSubPowAll /= nPixCount;//����ƽ��ֵ
	fSubPowAll = sqrt(fSubPowAll);//�ٿ���

    sTxt = QString("Min = %1;\r\nMax = %2;\r\nAve = %3;\r\nStd dev = %4").arg(QString::number(fMin, 'g', 2)).arg(QString::number(fMax, 'g', 2)).arg(QString::number(fAverage, 'g', 2)).arg(QString::number(fSubPowAll, 'g', 2));

	::DeleteObject(hGn);

	return Ret_Success;
}

int CHsBaseImg::Hs_GetPixelOnLine( const POINT &pt0,const POINT &pt1,int nPixCount,BYTE* pPixel)
{
/*	if(Hs_IsEmpty()==true)
		return Ret_EmptyImage;
	if(pPixel == NULL)
		return Ret_InvalidPara;

	int r0 = pt0.y;
	int c0 = pt0.x;
	int r1 = pt1.y;
	int c1 = pt1.x;

	if (m_ImgInfo.nBitsPerPixel==8)
	{
		BYTE* pData = (BYTE*)pPixel;
		BYTE** pOriData = (BYTE**)m_pOriData;
		if (c0 == c1)//|��ֱ
		{
			int i = 0;
			for (int r=r0;r<=r1;r++)
			{
				pData[i++] = pOriData[r][c0];
			}
		}
		else if (r0 == r1)
		{
			int i = 0;
			for (int c=c0;c<=c1;c++)
			{
				pData[i++] = pOriData[r0][c];
			}
		}
		else
		{
			//�����
			int nLen = nPixCount;
			
			//ֱ�߷��� //Y = k*(X-pt0.x) + pt0.y;
			double k = (pt1.y - pt0.y)/(pt1.x - pt0.x);

			//����x��y����x�Ĳ���Ϊ
			double fStep = nLen*1.00/abs(pt0.x-pt1.x);

			int i=0;
			int R = 0;
			int C = 0;
			for (double fx=pt0.x; fx<pt1.x; fx+=fStep)
			{
				C = int(fx);
				R = int( k*(fx-pt0.x) + pt0.y);

				pData[i++] = pOriData[R][C];
			}

		}
	}
	else if (m_ImgInfo.nBitsPerPixel == 16)
	{
		short* pData = (short*)pPixel;
		short** pOriData = (short**)m_pOriData;
		if (c0 == c1)//|��ֱ
		{
			int i = 0;
			for (int r=r0;r<=r1;r++)
			{
				pData[i++] = pOriData[r][c0];
			}
		}
		else if (r0 == r1)
		{
			int i = 0;
			for (int c=c0;c<=c1;c++)
			{
				pData[i++] = pOriData[r0][c];
			}
		}
		else
		{
			//�����
			int nLen = int( sqrt((r0 - r1)*(r0 - r1)*1.00 + (c0 - c1)*( c0 - c1)*1.00) );

			//ֱ�߷��� //Y = k*(X-pt0.x) + pt0.y;
			double k = (pt1.y - pt0.y)/(pt1.x - pt0.x);

			//����x��y����x�Ĳ���Ϊ
			double fStep = abs(pt0.x-pt1.x)*1.00/nLen;

			int i=0;
			int R = 0;
			int C = 0;

			double fx = 0.00;
			double fy = 0.00;
			for (fx=pt0.x; fx<=pt1.x; fx+=fStep)
			{
				C = int(fx);
				fy = k*(fx-pt0.x) + pt0.y;
				R = int( fy );

				pData[i++] = pOriData[R][C];			
			}

		}
	}
*/
	return Ret_Success;

}

void CHsBaseImg::IsRotateOrHorFlip(char cLft, char cTop, char cRgt, char cBtm, long &nAngle, bool &bFilp)
{
	if (cLft == 'L')
	{
		if (cTop == 'T')
		{
			nAngle = 0; bFilp = false;
		}
		else
		{
			nAngle = 180; bFilp = true;
		}
	}
	else if (cLft == 'T')
	{
		if (cTop == 'L')
		{
			nAngle = 90; bFilp = true;
		}
		else
		{
			nAngle = 270; bFilp = false;
		}
	}
	else if (cLft == 'R')
	{
		if (cTop == 'T')
		{
			nAngle = 0; bFilp = true;
		}
		else
		{
			nAngle = 180; bFilp = false;
		}
	}
	else if (cLft == 'B')
	{
		if (cTop == 'L')
		{
			nAngle = 90; bFilp = false;
		}
		else
		{
			nAngle = 270; bFilp = true;
		}
	}
}


int CHsBaseImg::Hs_Sharp(int v,bool bUpdateBits,HWND hwd)
{
	if (m_pDisplayData == NULL)//m_pOriData
		return Ret_EmptyImage;

	if(v == 0)
		return Ret_Success;

	
	if (m_bSharpOri == false)
	{//����ʾ���� m_pDisplayData---ȱ�㣺ÿ��ֵһ�����ز����µ���ʾ���ض�Ҫ��һ���񻯣��ܿ�

		BYTE** pNewDispData = ImgSharp(m_pDisplayData,m_ImgState.nDispalyRow,m_ImgState.nDispalyCol,v,m_hWnd);
		if (pNewDispData)
		{
			ArrayFree((void**)m_pDisplayData);
			m_pDisplayData = NULL;
			m_pDisplayData = pNewDispData;

			Hs_WinLevel(0,0,1,0,0);

			return 0;
		}	
	}
	else
	{//�����ԭʼ��������,ȱ�㣺�ٶ��������ǷŴ����С�����в��Ῠ
		
		BYTE** pNewOriData = ImgSharp(m_pOriData,m_ImgState.nCurOriPixRow,m_ImgState.nCurOriPixCol,v,hwd);
		if (pNewOriData)
		{
			ArrayFree((void**)m_pOriData);
			m_pOriData = NULL;
			m_pOriData = pNewOriData;

			if(bUpdateBits)
			{
				Hs_Size(NULL,0,0,HSIZE_RESAMPLE,true);
				Hs_WinLevel(0,0,1,0,0);
			}
		}

	}



	return 0;
}

BYTE** CHsBaseImg::ImgSharp( BYTE** pData,int nRow,int nCol, int v,HWND hwd )
{
	BYTE** pNewDispData = NULL;
	if(m_ImgInfo.nSamplePerPixel == 1)
	{
		if (m_ImgInfo.nBitsAllocated == 8)
		{
			if (m_ImgInfo.nPixelRepresentation == 0)
			{
				pNewDispData = (BYTE**)SharpImageUSM((unsigned char**)pData,nRow,nCol,m_ImgInfo.nBitStored,0,(1<<m_ImgInfo.nBitStored)-1,v,hwd);
			}
			else
			{
				pNewDispData = (BYTE**)SharpImageUSM((char**)pData,nRow,nCol,m_ImgInfo.nBitStored,-(1<<(m_ImgInfo.nBitStored-1)),(1<<(m_ImgInfo.nBitStored-1))-1,v,hwd);
			}
		}
		else if (m_ImgInfo.nBitsAllocated == 16)
		{
			if (m_ImgInfo.nPixelRepresentation == 0)
			{
				pNewDispData = (BYTE**)SharpImageUSM((unsigned short**)pData,nRow,nCol,m_ImgInfo.nBitStored,0,(1<<m_ImgInfo.nBitStored)-1,v,hwd);
			}
			else
			{
				pNewDispData = (BYTE**)SharpImageUSM((short**)pData,nRow,nCol,m_ImgInfo.nBitStored,-(1<<(m_ImgInfo.nBitStored-1)),(1<<(m_ImgInfo.nBitStored-1))-1,v,hwd);
			}
		}
		else if (m_ImgInfo.nBitsAllocated == 32)
		{
			if (m_ImgInfo.nPixelRepresentation == 0)
			{
				pNewDispData = (BYTE**)SharpImageUSM((unsigned long**)pData,nRow,nCol,m_ImgInfo.nBitStored,0,(1<<m_ImgInfo.nBitStored)-1,v,hwd);
			}
			else
			{
				pNewDispData = (BYTE**)SharpImageUSM((long**)pData,nRow,nCol,m_ImgInfo.nBitStored,-(1<<(m_ImgInfo.nBitStored-1)),(1<<(m_ImgInfo.nBitStored-1))-1,v,hwd);
			}
		}
	}
	else if (m_ImgInfo.nSamplePerPixel == 3)//RGB
	{
		if(pData == NULL)
			return NULL;

		pNewDispData = (BYTE**)ArrayNew(nRow,nCol,sizeof(MYDATA24));
		if (pNewDispData == NULL)
			return NULL;

		MYDATA24** tData = (MYDATA24**)pData;
		MYDATA24** tNewData = (MYDATA24**)pNewDispData;

		int rad = 6;
		INT64 nFaZhi = 16;

		INT64 tR,tG,tB;
		INT64 tMoHuR,tMoHuG,tMoHuB;
		INT64 ValueR,ValueG,ValueB;

		int nPix = 0;
		INT64 nPlusAllR= 0;
		INT64 nPlusAllG= 0;
		INT64 nPlusAllB= 0;

//		if(hwd)
//			::SendMessage(hwd,WM_ShowSharpPro,0,-100);

		double fStep = 0.00;
		for (int r=0; r<nRow; r++)
		{
			if(hwd)
			{
				double t = (r+1)*1.00/nRow;
				if(t - fStep > 0.01)
				{
//					::SendMessage(hwd,WM_ShowSharpPro,0,t*100);
					fStep = t;
				}
			}

			for (int c=0; c<nCol; c++)
			{
				tR = tData[r][c].pData[0];
				tG = tData[r][c].pData[1];
				tB = tData[r][c].pData[2];

				//ģ��
				nPix = 0;
				nPlusAllR= 0;
				nPlusAllG= 0;
				nPlusAllB= 0;
				for (int mr=r-rad;mr<=r+rad;mr++)
				{
					if(mr<0) 
						continue;
					if(mr>=nRow) 
						continue;

					for (int mc=c-rad;mc<=c+rad;mc++)
					{
						if(mc<0) 
							continue;
						if(mc>=nCol) 
							continue;

						nPlusAllR += tData[mr][mc].pData[0];
						nPlusAllG += tData[mr][mc].pData[1];
						nPlusAllB += tData[mr][mc].pData[2];
						nPix++;
					}
				}			

				tMoHuR = nPlusAllR/nPix;
				tMoHuG = nPlusAllG/nPix;
				tMoHuB = nPlusAllB/nPix;

				//������ģ��ֵ���
				ValueR = tR - tMoHuR;
				ValueG = tG - tMoHuG;
				ValueB = tB - tMoHuB;

				//��೬����ֵ����ͻ���������
				if (abs(ValueR) > nFaZhi)	tR = tR + v * ValueR / 100;
				if (abs(ValueG) > nFaZhi)	tG = tG + v * ValueG / 100;
				if (abs(ValueB) > nFaZhi)	tB = tB + v * ValueB / 100;


				if(tR > 255) 	tR = 255;
				if(tR < 0) 		tR = 0;

				if(tG > 255) 	tG = 255;
				if(tG < 0) 		tG = 0;

				if(tB > 255) 	tB = 255;
				if(tB < 0) 		tB = 0;

				tNewData[r][c].pData[0] = tR;
				tNewData[r][c].pData[1] = tG;
				tNewData[r][c].pData[2] = tB;
			}
		}

	}

//	if(hwd)
//		::SendMessage(hwd,WM_ShowSharpPro,0,-100);

	return pNewDispData;
}


/*֪ʶ�㣬Īɾ��
Lut Descriptor:
��һ��ֵv1:��ʾLut Data���ܹ����ٸ�Lutֵ
�ڶ���ֵv2:���ͼ���е�����ֵPixA<=��ֵv2ʱ,PixA�ڱ�LutData�ж�Ӧ��ֵӦΪv2��Ӧ��ֵ.ע��v2��Ӧ��ֵ,��LutData����ĵ�һ��ֵ.��0��Ԫ��!!!!!!!!!
������ֵv3:LutData��ÿ��ֵռ����λ?


��λ��:
1������ȷ������ϵ:�Բ��˵��ҽŵ���ǰ��,Ϊԭ��,,���ҵ���ΪX����,��ǰ����ΪY����,��������ΪZ����

2��TAG_IMAGE_POSITION_PATIENT(0020,0032)����ֵ:��ʾ����ͼ��������Ͻ������,����������ϵ�е�λ��.

3��TAG_IMAGE_ORIENTATION_PATIENT(0020,0037)����ֵ����:ǰ������Ϊ��ͼ�����������������������ϵX,Y,Z���cosֵ,������=��ͼ���������������X,Y,Z��cosֵ

4�����Pixel Spacing�ɵõ���λ�ߵĳ���.

*/


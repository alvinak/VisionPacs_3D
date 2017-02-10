#pragma once

//#include "AnnManager.h"//ע���崦��

#include "stdafx.h"
#include "AtlImage.h"

extern float SinPiX(const float &x);
extern float Matrix_Bicubic(const float (&A)[4], const float (&B)[4][4], const float (&C)[4]);

template <class T1>
void GetPixValueFromLut(T1 &nPixValue,LutData *pLut)
{
	if(pLut==NULL)
		return ;


	T1 *pData = (T1*)(pLut->pLutData);

	if(nPixValue<=pLut->nMinValue)
		nPixValue = pData[0];
	else if(nPixValue>=pLut->nMaxValue)
		nPixValue = pData[pLut->nLutLen-1];
	else
		nPixValue = pData[nPixValue - pLut->nMinValue];
}
template <class T2>
void FillBits( T2** pDisplayData,BYTE *pBits,BYTE** pDisplayOverlayData,unsigned long nRow,unsigned long nCol,bool bShowOverLay,bool bInverse,const QVector<LutData*> &pLutUsed,long *pLut,RECT* pValidRc)
{
	memset(pBits,0,nRow*nCol);
	int nLut = pLutUsed.size();
	unsigned long i = 0;
	for (long r=nRow-1;r>=0;r--)
	{

		for (long c=0;c<nCol;c++)
		{
			if (pValidRc)
			{
				if(c<=pValidRc->left || c>=pValidRc->right || r<=pValidRc->top || r>=pValidRc->bottom)
				{
					i++;
					continue;
				}
			}

			if (pDisplayOverlayData && bShowOverLay)
			{
				if(pDisplayOverlayData[r][c])
				{
					pBits[i] = 255; 

					i++;
					continue;
				}
			}

			T2 v = pDisplayData[r][c];
			for(int u=0;u<nLut;u++)
				GetPixValueFromLut(v,pLutUsed[u]);

			if(bInverse)
				pBits[i] = 255 - pLut[v];
			else
				pBits[i] = pLut[v];
			//if(v==-127 || v==129)
			//	ATLTRACE("\r\n------pLut[%d] = %d",v,pLut[v]);
			i++;
		}

	}

}

template <class T3>
void ImgSizeOneSampleNormal(T3** pNewData,unsigned long nNewRow,unsigned long nNewCol,T3 **pOriData,unsigned long nOriRow,unsigned long nOriCol, long x0, long y0)
{
	float fZoomX = (nOriCol-x0)*1.00/nNewCol;
	float fZoomY = (nOriRow-y0)*1.00/nNewRow;

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
}
template <class T4>
void ImgSizeOneSampleResample(T4** pNewData,unsigned long nNewRow,unsigned long nNewCol,T4 **pOriData,unsigned long nOriRow,unsigned long nOriCol, long x0, long y0,long x1,long y1,BYTE** pOriOverLayData=NULL)
{
	float fZoomX = (x1-x0)*1.00/nNewCol;
	float fZoomY = (y1-y0)*1.00/nNewRow;


	//��ЩRow����,��������nNewRow��
	float fr = 0.00;
	int Y = 0;
	float fy = 0.00;

	//��ЩCol�������������ѭ���ڽ������� nNewRow*nNewCol��.�ظ���nNewRow*(nNewCol-1)��,����Ҫ�����
	float fc =0.00;
	int *X = new int[nNewCol];		memset(X, 0,nNewCol*sizeof(int));
	float *fx = new float[nNewCol];	memset(fx,0,nNewCol*sizeof(float));


	int preC = 0;
	for (int c=0;c<nNewCol;c++)
	{
		fc = c*fZoomX;
		X[c] = int(fc) + x0;
		fx[c] = float(fc- X[c] + x0);


		if(X[c]+2>=nOriCol)//����ҪԽ��
		{
			fc = preC*fZoomX;//.�������һ����ȷ��cҲ����preC,������
			X[c] = int(fc) + x0;
			fx[c] = float(fc- X[c] + x0);
		}
		else
		{
			preC = c;
		}

	}

	int PreR = 0;
	for(int r=0;r<nNewRow;r++)
	{
		fr = r*fZoomY;
		Y = int(fr) + y0;
		fy = float(fr - Y + y0);


		if ( Y>=nOriRow-2 )//����ҪԽ��.
		{
			fr = PreR*fZoomY;//�������һ����ȷ��rҲ����preR,������
			Y = int(fr) + y0;
			fy = float(fr - Y + y0);
		}
		else
		{
			PreR = r;
		}

		for(int c=0;c<nNewCol;c++)
		{
			//T4 v1 = pNewData[r][c];
			//double s1 = (1-fy)*(1-fx[c])*pOriData[Y+1][X[c]+1];
			//double s2 = (1-fy)* fx[c]	*pOriData[Y+1][X[c]+2];
			//double s3 = fy*(1-fx[c])	*pOriData[Y+2][X[c]+1];
			//double s4 = fy*fx[c]		*pOriData[Y+2][X[c]+2];

			if(pOriOverLayData == NULL)
			{
				pNewData[r][c] = (T4)(
					(1-fy)*(1-fx[c])*pOriData[Y+1][X[c]+1] + 
					(1-fy)* fx[c]	*pOriData[Y+1][X[c]+2] + 
					fy*(1-fx[c])	*pOriData[Y+2][X[c]+1] +
					fy*fx[c]		*pOriData[Y+2][X[c]+2]   
				);//��ȡ��
			}
			else//������ʾOverlay����һ�ַ��������ַ�����OverLay������wc�ı仯��Ҳ���ű�
			{
				pNewData[r][c] = (T4)(
					(1-fy)*(1-fx[c])*( (pOriOverLayData[Y+1][X[c]+1] ? 16000 : pOriData[Y+1][X[c]+1]) ) + 
					(1-fy)* fx[c]	*( (pOriOverLayData[Y+1][X[c]+2] ? 16000 : pOriData[Y+1][X[c]+2]) ) + 
					fy*(1-fx[c])	*( (pOriOverLayData[Y+2][X[c]+1] ? 16000 : pOriData[Y+2][X[c]+1]) ) +
					fy*fx[c]		*( (pOriOverLayData[Y+2][X[c]+2] ? 16000 : pOriData[Y+2][X[c]+2]) )
					);
			}

			T4 v = pNewData[r][c];
			if( v == 64619)
			{
				int aaa = 0;//�˴��Ӷϵ�
			}

		}

		//for(int c=0;c<nNewCol;c++)//�Ż�ǰ:�б�����ȡǰ
		//{
		//	float fc = c*fZoomX;
		//	int X = int(fc) + x0;
		//	float fx = float(fc- X + x0);

		//	if ( X>=nOriCol-2 )
		//		X = nOriCol-3;

		//	pNewData[r][c] = (T4)(   (1-fy)*(1-fx)*pOriData[Y+1][X+1]+(1-fy)*fx*pOriData[Y+1][X+2]+fy*(1-fx)*pOriData[Y+2][X+1]+fy*fx*pOriData[Y+2][X+2]   );
		//}
	}

	delete []X;
	delete []fx;
}
template <class T5>
void ImgSizeOneSampleBicubic(T5** pNewData,unsigned long nNewRow,unsigned long nNewCol,T5 **pOriData,unsigned long nOriRow,unsigned long nOriCol, long x0, long y0,long nMinPixValue,long nMaxPixValue)
{
	float fZoomX = (nOriCol-x0)*1.00/nNewCol;
	float fZoomY = (nOriRow-y0)*1.00/nNewRow;

	float A_Matrix[4]	={1,1,1,0};
	float B_Matrix[4][4]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
	float C_Matrix[4]	={0,0,0,0};

	float i = 0.00;
	float j = 0.00;
	float u = 0.00;
	float v = 0.00;

	float tmp = 0.00;

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
					if(tc>=nOriCol) tc = nOriCol-1;
					if(tr>=nOriRow) tr = nOriRow-1;
					if(tc<=x0) tc = x0;
					if(tr<=y0) tr = y0;
					B_Matrix[t][f] = pOriData[tr][tc];
				}
			}

			tmp = Matrix_Bicubic(A_Matrix,B_Matrix,C_Matrix);

			if ( tmp < nMinPixValue)
				pNewData[r][c] = nMinPixValue;
			else if ( tmp >= nMaxPixValue)
				pNewData[r][c] = nMaxPixValue - 1; //m_imgInfo.HS_LUT_length-1;
			else
				pNewData[r][c] = (T5)tmp;
		}
	}

}
template <class T6>
int ImageCut(T6** pOldData,RECT CutRc,T6** pNewData)
{
	if(pNewData==NULL || pOldData==NULL)
		return Ret_InvalidPara;

	for (int r=CutRc.top;r<CutRc.bottom;r++)
	{
		for (int c=CutRc.left;c<CutRc.right;c++)
		{
			pNewData[r-CutRc.top][c-CutRc.left] = pOldData[r][c];
		}
	}

	return Ret_Success;
}

template <class T7>
int GetMaxMinPix(T7** pOriData,const RECT& PixRc, long& nMin, long& nMax)
{
	nMax = -100000;
	nMin = 100000;

	for (long r=PixRc.top;  r<PixRc.bottom;  r++)
	{
		for (long c=PixRc.left;  c<PixRc.right; c++)
		{
			nMin = min(nMin,long(pOriData[r][c]));
			nMax = max(nMax,long(pOriData[r][c]));
		}
	}

	if (nMin>nMax)
		return Ret_GetPixValueFailed;

	return Ret_Success;
}

template <class T8>
int TransformPix(T8** pOriData,long nRow,long nCol,int nType,long &lpNewOriData, unsigned long &nNewRow,	unsigned long &nNewCol)
{
	if(nType<1 || nType>4)
		return Ret_InvalidPara;

	nCol = ((nCol*8)+31)/32*4;

	long c = 0;//�����pOriData
	long r = 0;//�����pOriData

	T8** pNewOriData = NULL;
	if (nType==1)//˳ʱ��90��,���ǰѾɾ���ĵ�һ�����ر���¾���ĵ�һ������
	{
		pNewOriData = (T8**)ArrayNew(nCol,nRow,sizeof(T8),&nNewRow, &nNewCol);
		for ( c=0;  c<nCol;  c++)
		{
			for ( r=nRow-1;  r>=0; r--)
				pNewOriData[c][nRow-r-1] = pOriData[r][c];
		}
	}
	else if(nType==2)//��ʱ��90,����б��һ��
	{
		pNewOriData = (T8**)ArrayNew(nCol,nRow,sizeof(T8),&nNewRow, &nNewCol);
		for ( c=nCol-1;  c>=0;  c--)
		{
			for ( r=0;  r<nRow; r++)
				pNewOriData[nCol-c-1][r] = pOriData[r][c];
		}
	}
	else if (nType==3)//���·�ת,flip,��һ�б������
	{
		pNewOriData = (T8**)ArrayNew(nRow,nCol,sizeof(T8),&nNewRow, &nNewCol);

		int nRowLen = sizeof(T8)*nCol;
		for ( r=0;  r<nRow;  r++)
			memcpy(pNewOriData[nRow-r-1],pOriData[r],nRowLen);//ÿ�ж��������ڴ�,ֱ�ӿ���
	}
	else if (nType==4)//���Ҿ��� mirror,��һ�б������
	{
		pNewOriData = (T8**)ArrayNew(nRow,nCol,sizeof(T8),&nNewRow, &nNewCol);

		for (r=0; r<nRow;  r++)
		{
			for (c=0; c<nCol; c++)
			{
				pNewOriData[r][c] = pOriData[r][nCol-c-1];
			}
		}
	}


	lpNewOriData = long(pNewOriData);

	return Ret_Success;
}

template <class T9>
int ImgSubstract(T9** pOriDataA,T9** pOriDataB,unsigned long nRow,unsigned long nCol,T9 nMinValue,T9 nMaxValue)
{//A=A-B

	//T9 tMin = nMaxValue;
	//T9 tMax = nMinValue;

	INT64 t = 0;
	for (int r=0;r<nRow;r++)
	{
		for (int c=0;c<nCol;c++)
		{
			t = (pOriDataA[r][c] - pOriDataB[r][c]);
			t = (t + nMaxValue)/2;

			if (t<nMinValue) 
				pOriDataA[r][c] = nMinValue;
			else if(t>nMaxValue)
				pOriDataA[r][c] = nMaxValue;
			else
				pOriDataA[r][c] = t;

			//tMin = min(tMin,t);
			//tMax = max(tMax,t);
		}
	}

	//nMaxValue = tMax;
	//nMinValue = tMin;

	return Ret_Success;
}
template <class TA>
int FillPixel(TA**pOriData,TA** pNewOriData, RECT rcInCurImg,int nSelectPixW,int nSelectPixH,int nNewWidth,int nNewHeight,int nBytePerPix,TA  nSmallestPixelValue)
{
	if (nBytePerPix!=3)
	{
		for (int r=0; r<nNewHeight;	r++)
		{
			for (int c=0;  c<nNewWidth;  c++)
				pNewOriData[r][c] = nSmallestPixelValue;
		}

	}

	//��ʼ�����濽��ǰͼ���rcInCurImg�����ڵ�����
	//��������ͼ���������Ǹô���λ��
	RECT rcNewImg = {0,0,nNewWidth-1,nNewHeight-1};//������ͼ������
	RECT rcPixel;
	rcPixel.left = (rcNewImg.left + rcNewImg.right)/2 - nSelectPixW/2;
	rcPixel.top = (rcNewImg.top + rcNewImg.bottom)/2 - nSelectPixH/2;
	rcPixel.right = rcPixel.left + nSelectPixW;
	rcPixel.bottom = rcPixel.top + nSelectPixH;

	for (int r=rcPixel.top; r<rcPixel.bottom;	r++)
		memcpy(&pNewOriData[r][rcPixel.left],  &pOriData[rcInCurImg.top + r - rcPixel.top][rcInCurImg.left], nSelectPixW*nBytePerPix);

	return 0;
}

template <class T>
bool ConvertCoordForTransformation(T &x,T &y,long w,long h,bool HorFlip,int nAngle)
{
	if (nAngle < 0)
		nAngle += 360;

	T dfinalW;
	T newX;
	T newY;

	dfinalW = w;
	if (nAngle == 0)
	{
		newX = x; newY = y;
	}
	if (nAngle == 90)
	{
		newX = h - y + 1; newY = x;
		dfinalW = h;
	}
	else if (nAngle == 180)
	{
		newX = w - x + 1; newY = h - y + 1;
	}
	else if (nAngle == 270)
	{
		newX = y ; newY = w - x + 1;
		dfinalW = h;
	}

	if (HorFlip == true)
	{
		x = dfinalW - newX + 1; y = newY;
	}
	else
	{
		x = newX; y = newY;
	}

	return true;
}

template <class S>
S** SharpImage(S** pData,unsigned long nRow,unsigned long nCol,unsigned short nBitStore,INT64 nMinValue,INT64 nMaxValue,int v)
{
	INT64 t11,t12,t13,t21,t22,t23,t31,t32,t33;
	S** tempImgData = (S**)ArrayNew(nRow,nCol,sizeof(S));

	long GrayCount=long(pow(2.00,nBitStore*1.00));

	for (int r=0; r<nRow; r++)
	{
		for (int c=0; c<nCol; c++)
		{
			t22 = pData[r][c];

			if(r>0 && c>0 && r<nRow-1 && c<nCol-1)
			{
				t11 = pData[r-1][c-1];
				t12 = pData[r-1][c  ];
				t13 = pData[r-1][c+1];

				t21 = pData[r  ][c-1];
				t23 = pData[r  ][c+1];

				t31 = pData[r+1][c-1];
				t32 = pData[r+1][c  ];
				t33 = pData[r+1][c+1];

				t22 = t22+v*(t22-(t11+t12+t13+t21+t23+t31+t32+t33)/8);
			}

			//if(t22>GrayCount-1)
			//	t22=GrayCount-1;
			//if(t22<0)
			//	t22=0;

			if(t22 > nMaxValue) t22 = (S)nMaxValue;
			if(t22 < nMinValue) t22 = (S)nMinValue;

			tempImgData[r][c] = (S)t22;
		}
	}

	//for (long r=0; r<nRow;r++)
	//{
	//	for (long column=0; column<nCol; column++)
	//	{
	//		pData[r][column] = tempImgData[r][column];
	//	}
	//}


	return tempImgData;
}

template <class U>
U** SharpImageUSM(U** pData,unsigned long nRow,unsigned long nCol,unsigned short nBitStore,INT64 nMinValue,INT64 nMaxValue,int v,HWND hwd)//nFaZhi,int nAmount,int nRad
{
	int rad = 6;//ģ��ʱ�İ뾶

	INT64 nFaZhi = nBitStore * 2;//ģ���������� �� ԭ���� �Ĳ�ֵ ���� ���٣��Ŷ���ͻ���������

	INT64 t22,tMoHu,Value;

	int nPix = 0;
	INT64 nPlusAll= 0;

//	if(hwd)
//		::SendMessage(hwd,WM_ShowSharpPro,0,-1);

	double fStep = 0.00;
	U** tempImgData = (U**)ArrayNew(nRow,nCol,sizeof(U));
	for (int r=0; r<nRow; r++)
	{
		//if(hwd)
		//{
		//	double t = (r+1)*1.00/nRow;
		//	if(t - fStep > 0.01)
		//	{
		//		::SendMessage(hwd,WM_ShowSharpPro,0,t*100);
		//		fStep = t;
		//	}
		//}

		for (int c=0; c<nCol; c++)
		{
			t22 = pData[r][c];

			//ģ��
			nPix = 0;
			nPlusAll= 0;
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

					if(mr == r && mc == c)
						continue;

					nPlusAll += pData[mr][mc];
					nPix++;
				}
			}			

			tMoHu = nPlusAll*1.00/nPix;

			//������ģ��ֵ���
			Value = t22 - tMoHu;

			//��೬����ֵ����ͻ���������
			if (abs(Value) > nFaZhi)
			{
				Value = t22 + v * Value*1.00 / 100;//1232
				t22 = Value;
			}

			if(t22 > nMaxValue) 
				t22 = nMaxValue;

			if(t22 < nMinValue) 
				t22 = nMinValue;

			tempImgData[r][c] = (U)t22;

		}
	}

//	if(hwd)
//		::SendMessage(hwd,WM_ShowSharpPro,0,-100);

	return tempImgData;

}


template <class T4>
void ImgSizeOverlay(T4** pNewData,unsigned long nNewRow,unsigned long nNewCol,T4 **pOriOverlayData,T4 **pOriPixData,unsigned long nOriRow,unsigned long nOriCol, long x0, long y0,long x1,long y1,int nSizeType)
{
	if (nSizeType == HSIZE_RESAMPLE)
	{
		float fZoomX = (x1-x0)*1.00/nNewCol;
		float fZoomY = (y1-y0)*1.00/nNewRow;


		//��ЩRow����,��������nNewRow��
		float fr = 0.00;
		int Y = 0;
		float fy = 0.00;

		//��ЩCol�������������ѭ���ڽ������� nNewRow*nNewCol��.�ظ���nNewRow*(nNewCol-1)��,����Ҫ�����
		float fc =0.00;
		int *X = new int[nNewCol];		memset(X, 0,nNewCol*sizeof(int));
		float *fx = new float[nNewCol];	memset(fx,0,nNewCol*sizeof(float));


		int preC = 0;
		for (int c=0;c<nNewCol;c++)
		{
			fc = c*fZoomX;
			X[c] = int(fc) + x0;
			fx[c] = float(fc- X[c] + x0);


			if(X[c]+2>=nOriCol)//����ҪԽ��
			{
				fc = preC*fZoomX;//.�������һ����ȷ��cҲ����preC,������
				X[c] = int(fc) + x0;
				fx[c] = float(fc- X[c] + x0);
			}
			else
			{
				preC = c;
			}

		}

		int PreR = 0;
		for(int r=0;r<nNewRow;r++)
		{
			fr = r*fZoomY;
			Y = int(fr) + y0;
			fy = float(fr - Y + y0);


			if ( Y>=nOriRow-2 )//����ҪԽ��.
			{
				fr = PreR*fZoomY;//�������һ����ȷ��rҲ����preR,������
				Y = int(fr) + y0;
				fy = float(fr - Y + y0);
			}
			else
			{
				PreR = r;
			}

			for(int c=0;c<nNewCol;c++)
			{
				pNewData[r][c] = (T4)(
					(1-fy)*(1-fx[c])*pOriOverlayData[Y+1][X[c]+1]==0 ? pOriPixData[Y+1][X[c]+1] : pOriOverlayData[Y+1][X[c]+1] + 
					(1-fy)* fx[c]	*pOriOverlayData[Y+1][X[c]+2]==0 ? pOriPixData[Y+1][X[c]+2] : pOriOverlayData[Y+1][X[c]+2] + 
					fy*(1-fx[c])	*pOriOverlayData[Y+2][X[c]+1]==0 ? pOriPixData[Y+2][X[c]+1] : pOriOverlayData[Y+2][X[c]+1] +
					fy*fx[c]		*pOriOverlayData[Y+2][X[c]+2]==0 ? pOriPixData[Y+2][X[c]+2] : pOriOverlayData[Y+2][X[c]+2]  
				);

			}
		}

		delete []X;
		delete []fx;
	}

}

class CHsFile;
class InfoInstance;
class CAnnoMprRotateLine;
class CDicomPr;
class CHsBaseImg
{
public:
	CHsBaseImg(void);
	~CHsBaseImg(void);

	// ͼ���ԭʼ����
	BYTE ** m_pOriData;

	// m_pOriData������ֵ�ӹ�֮�������,�ݴ˿�������ʾ�õ�m_pBits;
	BYTE ** m_pDisplayData;

	// ����ͼԭʼ����
	BYTE **m_pOriOverlayData;

	// ����ͼ��ʾ������
	BYTE **m_pDisplayOverlayData;

	//ͼ��ԭʼ��Ϣ
	ImageInfo m_ImgInfo;

	//��ǰͼ��״̬,Ч���Ȳ���
	ImgState m_ImgState;

	//����Lut.
	QVector <LutData*> m_pLutV;

	//��ʾ���ĸ�HWND��
	HWND m_hWnd;

	SIZE m_PreSize;//Hs_Size�������ܹ��Ĳ���,
	short m_nPreSizeType;//Hs_Size�������ܹ��Ĳ�ֵ����
	RECT *m_pPreSizeRc;//Hs_Size�������ܹ��ľ�������

	//HMY
	bool m_bMpr;//��Ǵ�pImg�Ƿ�Ϊ������mpr;
	QString m_sImgMprMode;////MPRʱ��Ч���Ƿ�ΪMIP,falseʱΪaverage
protected:

	//������ʾ��������(0-255)
	BYTE *m_pBits;

	//WL LUT
	long *m_pWcLut;

	//������ʾ�õ�m_pBits
	int UpdateBits(unsigned long nRow,unsigned long nCol,BYTE** pDisplayData, BYTE** pDisplayOverlayData,BYTE* &pBits,RECT* pValidRc=NULL);
	int UpdateBitsNormal(unsigned long nRow,unsigned long nCol,BYTE** pDisplayData,BYTE** pDisplayOverlayData,BYTE* pBits,RECT* pValidRc=NULL);
	int UpdateBitsPaletteColor(unsigned long nRow,unsigned long nCol,BYTE** pDisplayData,BYTE** pDisplayOverlayData,BYTE* pBits);

	void ImgSizeThreeSample(MYDATA24** pNewData,unsigned long nNewRow,unsigned long nNewCol,MYDATA24 **pOriData,unsigned long nOriRow,unsigned long nOriCol, long x0, long y0,long x1,long y1,short nSizeType);

	//ÿ��Lut��������,��������ȡ��ָ��
	LutData *GetLutByName(QString sLutName);

	//��ͨͼ�����
	virtual int WinLevelNormal(long w,long c,bool bUpdateBits = true);

	//��ֵ
	int ImgSize(BYTE ** pRetData,BYTE ** pRetOverlayData,RECT *pSrcRc, unsigned long nWidth, unsigned long nHeight, short nType);


	////��λ��
	//int CalLocLine(CHsBaseImg *pImgOnMe,geometry::LineCoord&line);


	//����ת���ߵ�
	virtual int TransformImg(int nType);

	//��---��д����Ӧ������ͨ�õģ��������������Ϊ����Ķ���ӿ�ȴ�Ե÷������������ֵ�������񻯺�������������
	BYTE** ImgSharp(BYTE** pData,int nRow,int nCol, int v,HWND hwd);

	//��ʱ����ԭʼ���ػ�����ʾ����
	bool m_bSharpOri;
public:
	//Ӧ��dcm�ļ�������ΪsName��Lut�򴰿�λ
	LutData* Hs_ApplyLut(QString sName);

	//��ȡ��ǰӦ�õ�Lut����
	int Hs_GetCurLutName(QString &sName);

	//����,bChangeValue=true��Ϊw��c�Ǳ仯��.����w��c�Ǿ���ֵ
	virtual int Hs_WinLevel(long w,long c,bool bChangeValue=false,long *pRetW=NULL,long *pRetC=NULL);

	//����m_pDisplayData
	virtual int Hs_DrawImg(HDC dc,const RECT& rc,BYTE *pBits=NULL,unsigned long nRow=0,unsigned long nCol=0);

	//��ȡ��ǰͼ���С.
	SIZE Hs_GetImgSize(bool bDisplaySize=false);

	//��ǰ��ʾ�õ�m_pDisplayData��m_pOriData��Hs_Size�����ӹ���Ĳ���,��ôm_pDisplayData��������m_pOriData�ӹ�����?����ֻ����m_pOriDataһ�������ؼӹ���?--����Hs_Size����֪��
	bool Hs_IsWholeImgSized();

	//�ͷ���Դ
	int Hs_FreeMem();

	//��ͼ���Ƿ�������
	bool Hs_IsEmpty();

	//Ӧ��ʾ������OverLay
	int Hs_ApplyOverlay(bool bShow);

	//Ӧ�û�ȡ��ʹ��б�ʽؾ�
	int Hs_ApplySlope(bool bUse);

	//��ͼ����в�ֵ
	int Hs_Size(RECT *pSrcRc, unsigned long nWidth, unsigned long nHeight, short nType,bool bForce=false);

	//��ȡĳ�������ֵȡ��m_pDisplayData
	double Hs_GetPixelValue(unsigned long r,unsigned long c);

	//����һ������Ķ���
	virtual int Hs_CopyTo(CHsBaseImg &NewHsImg);

	//��ԭ
	virtual int Hs_Restore();

	//��ת
	virtual int Hs_Rotate(int nAngle,bool bSaveFlag = true);

	//���·�ת--���Ϊ��û��SaveFlag��������Ϊ�ٴ�ʵ����ʱ���˲����ᱻ��ת�;�����档
	virtual int Hs_Flip();

	//���Ҿ���
	virtual int Hs_Mirror(bool bSaveFlag = true);


	//����
	virtual int Hs_Cut(long &left,long &top,long &right,long &bottom);


	//��ͼ���ImgRc����,����nBitsRow X nBitsCol��С��pBits---�Ŵ�Ƭ--�ֲ��Ŵ�������
	int Hs_CreateDisplayBits(RECT ImgRc,BYTE* &pRetBits,unsigned long &nBitsRow,unsigned long &nBitsCol);

	//��ȡImgPixRc�����ڵ��������ֵ��Сֵ----����Ȥ��
	int GetMinMaxValue(const RECT &ImgPixRc,long &nMin,long &nMax,bool bUseSlope);

	//��Ӱ(��ͼ - pImgB)
	int Hs_Subtraction(CHsBaseImg *pImgB);

	//��Ƭ
	int Hs_Inverse();

	//���ImgState
	int Hs_InitImgState();

	//��ȡ���һ����ͼ���ϵĲ�ֵ����
	RECT Hs_GetSizeRcOnOriImg();

	//���������Ե�ǰͼ���rcInCurImg��������Ϊ���ģ������ܲ����أ�����nNewWidth, nNewHeight��ô��
	int Hs_FillPixel(RECT rcInCurImg,int nNewWidth,int nNewHeight);

	////д��Ϣ
	//virtual void Hs_DrawInfo(HDC dc,RECT rc,int nInfoType,COLORREF TxtClor,bool bLimitMinFont);

	//��ȡCTֵ
	int Hs_GetCtValuePoint(POINT ImgPt,QString &sTxt);

	//��ȡ��Բ��ΧCTֵ
	int Hs_GetCtValueEllipse(RECT ImgRc,QString &sTxt);

	//��pt1---pt2�߶Σ������ĵ������ֵ
	int Hs_GetPixelOnLine(const POINT &pt0,const POINT &pt1,int nPixCount,BYTE* pPixel);

	////���ͼƬ
	//int Hs_SaveAs(char* sFileName,int nType,bool bRuler,bool bPtInfo,bool bAnno);

	//��
	int Hs_Sharp(int v,bool bUpdateBits,HWND hwd);

	//CT��MR��ͼ���ȡ����λ��
	int Hs_GetPatientPos( double x, double y, double z,QString& sLeft, QString& sRight );

private:

	void IsRotateOrHorFlip(char cLft, char cTop, char cRgt, char cBtm,long &nAngle, bool &bFilp);

};

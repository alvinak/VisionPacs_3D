#include "stdafx.h"

#include "HsBaseFile.h"
#include "io.h"

bool g_bDebug = false;

void** ArrayNew(unsigned long nRow, unsigned long nCol, unsigned long nSize,unsigned long *nNewRow=NULL, unsigned long* nNewCol=NULL);
void ArrayFree(void **pArr,int iflag=0);
void** ArrayCopy(void** pSrcArr,unsigned long nSrcRow, unsigned long nSrcCol, unsigned long nSrcSize);

void** ArrayNew(unsigned long nRow, unsigned long nCol, unsigned long nSize,unsigned long *nNewRow, unsigned long* nNewCol)
{
	if (nRow==0 || nCol==0 || nSize==0)
		return NULL;

	nCol = ((nCol*8)+31)/32*4;

	unsigned long nDataLen = nRow*nCol*nSize;
	BYTE* pData = new BYTE[nDataLen];
	memset(pData,0,nDataLen);

	long *pAdd = new long[nRow];
    for(unsigned long i=0;i<nRow;i++)
		pAdd[i] = long(&(pData[i*nCol*nSize]));

	if (nNewRow) *nNewRow = nRow;
	if (nNewCol) *nNewCol = nCol;

	return (void**)pAdd;
}

void ArrayFree(void **pArr,int iflag)
{
	if(pArr==NULL)
		return;

	BYTE *pData = (BYTE*)(pArr[0]);
	delete[]pData;

	long *pAdd = (long*)pArr;
	delete []pAdd;
}

void** ArrayCopy(void** pSrcArr,unsigned long nSrcRow, unsigned long nSrcCol, unsigned long nSrcSize)
{
	if (nSrcRow==0 || nSrcCol==0 || nSrcSize==0 || pSrcArr==NULL)
		return NULL;

	nSrcCol = ((nSrcCol*8)+31)/32*4;

	unsigned long nDataLen = nSrcRow*nSrcCol*nSrcSize;
	BYTE* pDstData = new BYTE[nDataLen];
	memcpy(pDstData,pSrcArr[0],nDataLen);

	long *pAdd = new long[nSrcRow];
    for(unsigned long i=0;i<nSrcRow;i++)
		pAdd[i] = long(&(pDstData[i*nSrcCol*nSrcSize]));

	return (void**)pAdd;
} 


RECT GetShowRcByImgSize(RECT rc, long ImgWidth, long ImgHeight);
RECT GetShowRcByImgSize(RECT rc, double ImgWidth, double ImgHeight)
{
    RECT reRC;
    reRC.bottom = reRC.left = reRC.right =reRC.top = 0;
    if (rc.right == rc.left)
        return reRC;


    if ((rc.bottom-rc.top)*ImgWidth >= ImgHeight*(rc.right-rc.left))//<=(cy/cx)>=(ImgHeight/ImgWidth)
    {//�������
        double fNewH=0;
        reRC.left = rc.left;
        reRC.right = rc.right;
        fNewH = ( ImgHeight * (rc.right-rc.left) )/ImgWidth;
        reRC.top = int((rc.bottom+rc.top-fNewH)/2);
        reRC.bottom = reRC.top+(long)(fNewH+0.5);
    }
    else
    {//�������
        double fNewW=0;
        reRC.top = rc.top;
        reRC.bottom = rc.bottom;
        fNewW = ( (rc.bottom-rc.top)*ImgWidth)/ImgHeight;
        reRC.left = int((rc.right+rc.left-fNewW)/2);
        reRC.right = reRC.left+(long)(fNewW+0.5);
    }
    return reRC;
}

CHsBaseFile::CHsBaseFile(void)
:m_nDefaultReadSize(40960)
,m_fp(NULL)
,m_buf(NULL)
,m_pMainEle(NULL)
,m_pPreEle(NULL)
,m_nBufLen(0)
,m_bXRay(false)
,m_sModality("")
{
	m_BaseFileInfo.nReadLen = m_nDefaultReadSize;

	//�ж�CPU���С��,��Ӱ���ҵ�GetDouble����
	union ut
	{
		short s;
		char c[2];
	}u;

	u.s = 0x0102;
	if(u.c[0] == 1 && u.c[1] == 2)
		m_BaseFileInfo.bCpuBigEndia = true;
	else
		m_BaseFileInfo.bCpuBigEndia = false;

}

CHsBaseFile::~CHsBaseFile(void)
{
	delete m_pMainEle;
	FreeBuffer();
	Hs_CloseFile();
}
int CHsBaseFile::Hs_CloseFile()
{
	if (m_fp)
		fclose(m_fp);
	m_fp = NULL;

	return Ret_Success;
}

int CHsBaseFile::FreeBuffer(void)
{
	if(m_buf)
		delete []m_buf;
	m_buf = NULL;

	m_nBufLen = 0;
	return Ret_Success;
}

BYTE* CHsBaseFile::Hs_GetByteValue(pHsElement pEle,unsigned long &nValueLen,int &nRet)
{
	nRet = 0;
	if(pEle == NULL)
	{
		nRet = Ret_InvalidPara;
		return NULL;
	}

	if(pEle->nLen==0 || pEle->pValue==NULL)
	{
		nRet = Ret_NoValue;
		return NULL;
	}

	nValueLen = pEle->nLen;
	BYTE* pRet = new BYTE[nValueLen];
	memcpy(pRet,pEle->pValue,nValueLen);
	return pRet;
}

int CHsBaseFile::ShowElement(pHsElement p,int nLevel)
{
	if(p == NULL)
		return 0;

	QString s = "\r\n";
	for(int i=0;i<nLevel;i++)
		s += "	";

	if(p->nVR==VR_SS)//UL SL SS US
	{
		qDebug("\r\n0x%08x	%d	%s", p->nTag, p->nLen, CDcmTag::Find(p->nTag)->pszName);

		unsigned long st = p->nTagPos + p->nOffset;

		for (unsigned long i=0;i<p->nLen;i++)
		{
			qDebug("\r\n%02x,%d,%c", m_buf[st + i], m_buf[st + i], m_buf[st + i]);
		}

		qDebug("\r\n");

		int nRet = 0;
		int n = Hs_GetValueCount(p,nRet);
		
		long v = 0;


		if(n!=0)
			int r = Hs_GetLongValue(p,v,n-1);

		int stp = 0;
	}

	int n = p->pChildEleV.size();
	for (int i = 0; i<n; i++)
	{
		ShowElement(p->pChildEleV[i],nLevel+1);
	}
	return 0;
}

int CHsBaseFile::Hs_GetDoubleValue(pHsElement pEle,	double &fVal,int nValueIndex)
{
	if(pEle==NULL || nValueIndex<0)
		return Ret_InvalidPara;

	if (pEle->nLen==0 || pEle->pValue==NULL)
		return Ret_NoValue;

	BYTE *p = pEle->pValue;

	//unsigned long nPos = pEle->nTagPos + pEle->nOffset;
	unsigned long nPos = 0;
	if (pEle->nVR == VR_DS || pEle->nVR == VR_OF)
	{
		char *pString = new char[pEle->nLen+1];
		memset(pString,0,pEle->nLen+1);
		memcpy(pString,p,pEle->nLen);

		QString sVal;
		int nRet = DivString(pString,"\\",nValueIndex,sVal);
		if(nRet == Ret_Success)
			fVal = sVal.toDouble();

		delete []pString;

		return nRet;
	}

	if (pEle->nVR == VR_FD)
	{
		if(pEle->nLen%8!=0)//����8�ı���,��������
			return Ret_InvalidBuf;

		unsigned long nValueCount = pEle->nLen/8;
		if(nValueIndex>=nValueCount)
			return Ret_OutOfValueCount;

		
        for(unsigned long i=0;i<nValueCount;i++)
		{
			if (i==nValueIndex)
			{
				//unsigned char uc[8] = {m_buf[nPos],m_buf[nPos+1],m_buf[nPos+2],m_buf[nPos+3],m_buf[nPos+4],m_buf[nPos+5],m_buf[nPos+6],m_buf[nPos+7]};
				unsigned char uc[8] = {p[nPos],p[nPos+1],p[nPos+2],p[nPos+3],p[nPos+4],p[nPos+5],p[nPos+6],p[nPos+7]};
				if(pEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia)
				{
					//uc[0] = m_buf[nPos+7];uc[1] = m_buf[nPos+6];uc[2] = m_buf[nPos+5];uc[3] = m_buf[nPos+4];
					//uc[4] = m_buf[nPos+3];uc[5] = m_buf[nPos+2];uc[6] = m_buf[nPos+1];uc[7] = m_buf[nPos];

					uc[0] = p[nPos+7];uc[1] = p[nPos+6];uc[2] = p[nPos+5];uc[3] = p[nPos+4];
					uc[4] = p[nPos+3];uc[5] = p[nPos+2];uc[6] = p[nPos+1];uc[7] = p[nPos];
				}
				double *f = (double*)uc;
				fVal = *f;

				return Ret_Success;
			}

			nPos += 8;
		}

		return Ret_GetValueFailed;
	}
	if (pEle->nVR == VR_FL)
	{
		if(pEle->nLen%4!=0)//����4�ı���,��������
			return Ret_InvalidBuf;

		unsigned long nValueCount = pEle->nLen/4;
		if(nValueIndex>=nValueCount)
			return Ret_OutOfValueCount;

		for(int i=0;i<nValueCount;i++)
		{
			if (i==nValueIndex)
			{
				//unsigned char uc[4] = {m_buf[nPos],m_buf[nPos+1],m_buf[nPos+2],m_buf[nPos+3]};
				unsigned char uc[4] = {p[nPos],p[nPos+1],p[nPos+2],p[nPos+3]};
				if(pEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia)
				{
					//uc[0] = m_buf[nPos+3];	uc[1] = m_buf[nPos+2];	uc[2] = m_buf[nPos+1];	uc[3] = m_buf[nPos];
					uc[0] = p[nPos+3];	uc[1] = p[nPos+2];	uc[2] = p[nPos+1];	uc[3] = p[nPos];
				}

				float *f = (float*)uc;
				fVal = *f;

				return Ret_Success;
			}
			nPos += 4;
		}

		return Ret_GetValueFailed;
	}

	return Ret_InvalidDicomVR;

}

int CHsBaseFile::Hs_GetStringValue(pHsElement pEle, QString &sRet,	int nIndex)
{
	sRet = "";

	if(pEle==NULL)
		return Ret_InvalidPara;

	if(pEle->pValue == NULL && m_buf == NULL)
		return Ret_InvalidPara;

	char *pString = new char[pEle->nLen+1];
	memset(pString,0,pEle->nLen+1);

	if(m_buf)//����������ڳ����ʼʱ����ȡ�����﷨���õģ���ʱ������m_buf����û��pEle->pValue��
		memcpy(pString,   &(m_buf[pEle->nTagPos+pEle->nOffset]), pEle->nLen);
	else 
		memcpy(pString,   pEle->pValue, pEle->nLen);


	QString sValue = QString(QLatin1String(pString));
	QStringList sections = sValue.split("\\");
	sRet = sections.at(nIndex).trimmed();

	delete pString;

	return Ret_Success;
}

int CHsBaseFile::Hs_GetAgeValue(pHsElement pEle,int &nAge,char &cAgeType)
{
	if(pEle == NULL)
		return Ret_InvalidPara;

	if(pEle->nLen==0 || pEle->pValue == NULL)
		return Ret_NoValue;
	

	char *cNumber = new char[pEle->nLen+1];
	memset(cNumber,0,pEle->nLen+1);


	for (unsigned long i=0;i<pEle->nLen;i++)
	{
		char c = pEle->pValue[i];
		if(c>='0' && c<='9')
		{
			cNumber[strlen(cNumber)] = c;
		}
		else
		{
			cAgeType = c;
			break;
		}
	}

	QString stemp = QString(QLatin1String(cNumber));
	nAge = stemp.toInt();
	delete []cNumber;

	return Ret_Success;
}
int CHsBaseFile::Hs_GetDateValue(pHsElement pEle, HsDateTime &DateValue, int nValueIndex)
{
	//D:\WorkRoom\DicomServer\2009\vss0\�����ս��\Debug\temp_WorkList_Come.dic
	//E:\1ͼƬ\Pet-CT\CBK_NECK\34969671
	memset(&DateValue,0,sizeof(HsDateTime));

	

	if(pEle==NULL || nValueIndex<0)
		return Ret_InvalidPara;

	if(pEle->nLen==0 || pEle->pValue==NULL)
		return Ret_NoValue;

	//����һ��Buf
	char *pString = new char[pEle->nLen+1];
	memset(pString,0,pEle->nLen+1);

	int k = 0;
	for(unsigned long i=0;i<pEle->nLen;i++)
	{			
		//char c = m_buf[pEle->nTagPos + pEle->nOffset + i];
		char c = pEle->pValue[i];

		if(c>='0' && c<='9')
			pString[k++] = c;
	}

	QString sValue = QString(QLatin1String(pString)); 
	delete []pString;

	if ((nValueIndex+1)*8 > int( sValue.size() )  )
		return Ret_OutOfValueCount;

	QString sDate = sValue.mid(nValueIndex*8,8);
	DateValue.nYear = sDate.mid(0, 4).toInt();
	DateValue.nMonth = sDate.mid(4, 2).toInt();
	DateValue.nDay = sDate.mid(6,2).toInt();

	return Ret_Success;
}
int CHsBaseFile::Hs_GetTimeValue(pHsElement pEle,	HsDateTime &TimeValue,	int nValueIndex)
{
	memset(&TimeValue,0,sizeof(HsDateTime));

	if(pEle == NULL)
		return Ret_InvalidPara;
	if(pEle->nLen==0 || pEle->pValue == NULL)
		return Ret_NoValue;
	if(nValueIndex<0)
		return Ret_OutOfValueCount;

	char *pString = new char[pEle->nLen+1];
	memset(pString,0,pEle->nLen+1);

	int k = 0;
	for(unsigned long i=0;i<pEle->nLen;i++)
	{
		//char c = m_buf[pEle->nTagPos + pEle->nOffset + i];
		char c = pEle->pValue[i];
		if( (c>='0' && c<='9') || c=='.' )
			pString[k++] = c;
		else
		{
			if(c!=0 && c!=' ')
				pString[k++] = '\\';//������Щ��̬Dcm�ļ�д��'-',���Բ���ô�鷳,�������ôд��,�Ҽ���
		}
	}

	QString sValue = QString(QLatin1String(pString));
	QStringList sections = sValue.split("\\");
	sValue = sections.at(nValueIndex).trimmed();
	delete []pString;

	int nDot = sValue.indexOf('.',0);
	if(nDot==6)
	{
		TimeValue.nHours = sValue.mid(0,2).toInt();
		TimeValue.nMinutes = sValue.mid(2,2).toInt();
		TimeValue.nSeconds = sValue.mid(4,2).toInt();

		QString sMillisecond = sValue.mid(7,sValue.size() - 7);
		TimeValue.nFractions = sMillisecond.toLong();

		return Ret_Success;
	}
	else if (nDot==-1)//1010Ҳ���Ա�ʾΪ10:10��
	{
		int nMemID = 0;

		int nPos = 0;
		while(1)
		{
			if(nPos >= sValue.size()-1)
				break;

			QString s = sValue.mid(nPos,2);
			nPos+=2;

			if (nMemID==0)//ʱ
				TimeValue.nHours = s.toInt();
			else if(nMemID==1)//��
				TimeValue.nMinutes = s.toInt();
			else if(nMemID==2)//��
				TimeValue.nSeconds = s.toInt();
			else
				break;

			nMemID++;
		}

		return Ret_Success;
	}
	else
	{
		return Ret_InvalidBuf;
	}

	return Ret_Success;
}
int CHsBaseFile::Hs_GetDateTimeValue(pHsElement pEle,	HsDateTime &DateTimeValue,	int nValueIndex)
{
	memset(&DateTimeValue,0,sizeof(HsDateTime));

	//����
	if(pEle == NULL)
		return Ret_InvalidPara;
	if(pEle->nLen==0 || pEle->pValue == NULL)
		return Ret_NoValue;
	if(nValueIndex<0)
		return Ret_OutOfValueCount;

	//�����ַ�
	char *pString = new char[pEle->nLen+1];
	memset(pString,0,pEle->nLen+1);

	int k = 0;
	for(unsigned long i=0;i<pEle->nLen;i++)
	{
		//char c = m_buf[pEle->nTagPos + pEle->nOffset + i];
		char c = pEle->pValue[i];
		if( (c>='0' && c<='9') || c=='.' || c=='+' || c=='-')
			pString[k++] = c;
		else
		{
			if(c!=0 && c!=' ')
				pString[k++] = '\\';//������Щ��̬Dcm�ļ�д��'-',���Բ���ô�鷳
		}
	}

	//�õ���ӦID���ַ���
	QString sValue = QString(QLatin1String(pString));
	QStringList sections = sValue.split("\\");
	sValue = sections.at(nValueIndex).trimmed();
	delete []pString;


	//�����õ����ַ���
	int nMemID = 0;

	int nPos = 0;
	while(1)
	{
		if(nPos >= sValue.size()-1)
			break;

		QString s = "";

		if (nMemID==0)//��
		{
			s = sValue.mid(nPos,4);
			nPos += 4;
			DateTimeValue.nYear = s.toInt();
		}
		else if(nMemID==1)//��
		{
			s = sValue.mid(nPos,2);
			nPos += 2;
			DateTimeValue.nMonth = s.toInt();
		}
		else if(nMemID==2)//��
		{
			s = sValue.mid(nPos,2);
			nPos += 2;
			DateTimeValue.nDay = s.toInt();
		}
		else if (nMemID==3)//ʱ
		{
			s = sValue.mid(nPos,2);
			nPos += 2;
			DateTimeValue.nHours = s.toInt();
		}
		else if (nMemID==4)//��
		{
			s = sValue.mid(nPos,2);
			nPos += 2;
			DateTimeValue.nMinutes = s.toInt();
		}
		else if (nMemID==5)//��
		{
			s = sValue.mid(nPos,2);
			nPos += 2;
			DateTimeValue.nSeconds = s.toInt();
		}
		else if (nMemID==6)//"." �� "+" "-"
		{
			if(sValue[nPos]!='.')//û�к��벿�������.����ƫ������-----����,�����
				break;

			//�е���ŵ����
			int tPos = max( (int)sValue.indexOf('+'), (int)sValue.indexOf('-') );
			int nEnd = 0;
			if(tPos>nPos)//�ҵ���
				nEnd = tPos;
			else
				nEnd = int(sValue.size()) - 1;
			
			s = "0";
			s += sValue.mid(nPos,nEnd-nPos);

			DateTimeValue.nFractions = s.toFloat() * 1000000;

			break;

		}

		nMemID++;
	}


	return Ret_Success;
}

int CHsBaseFile::Hs_GetLongValue(pHsElement pEle, long &nValue, int nValueIndex)
{
	nValue = 0;

	if(pEle==NULL)
		return Ret_InvalidPara;
	if (pEle->nLen==0 || pEle->pValue==NULL)
		return Ret_NoValue;
	if(nValueIndex<0)
		return Ret_OutOfValueCount;

	BYTE* p = pEle->pValue;
	//unsigned long nPos = pEle->nTagPos + pEle->nOffset;
	unsigned long nPos = 0;

	if (pEle->nVR == VR_UL)
	{
		if(nValueIndex*4>=pEle->nLen)
			return Ret_OutOfValueCount;
		nPos += nValueIndex*4;

		//unsigned char uc[4] = {m_buf[nPos],m_buf[nPos+1],m_buf[nPos+2],m_buf[nPos+3]};
		unsigned char uc[4] = {p[nPos],p[nPos+1],p[nPos+2],p[nPos+3]};
		if(pEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia)
		{
			uc[0] = p[nPos+3];	uc[1] = p[nPos+2];	uc[2] = p[nPos+1];	uc[3] = p[nPos];
		}

		unsigned long *r = (unsigned long*)uc;
		nValue = long(*r);
	}
	if (pEle->nVR == VR_SL)
	{
		if(nValueIndex*4>=pEle->nLen)
			return Ret_OutOfValueCount;
		nPos += nValueIndex*4;

		//unsigned char uc[4] = {m_buf[nPos],m_buf[nPos+1],m_buf[nPos+2],m_buf[nPos+3]};
		unsigned char uc[4] = {p[nPos],p[nPos+1],p[nPos+2],p[nPos+3]};
		if(pEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia)
		{
			//uc[0] = m_buf[nPos+3];	uc[1] = m_buf[nPos+2];	uc[2] = m_buf[nPos+1];	uc[3] = m_buf[nPos];
			uc[0] = p[nPos+3];	uc[1] = p[nPos+2];	uc[2] = p[nPos+1];	uc[3] = p[nPos];
		}

		long *r = (long*)uc;
		nValue = long(*r);
	}
	if (pEle->nVR == VR_SS)
	{
		if(nValueIndex*2>=pEle->nLen)
			return Ret_OutOfValueCount;
		nPos += nValueIndex*2;

		//unsigned char uc[2] = {m_buf[nPos],m_buf[nPos+1]};
		unsigned char uc[2] = {p[nPos],p[nPos+1]};
		if(pEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia)
		{
			//uc[0] = m_buf[nPos+1];	
			//uc[1] = m_buf[nPos];
			uc[0] = p[nPos+1];	
			uc[1] = p[nPos];
		}

		short *r = (short *)uc;
		nValue = long(*r);
	}
	if (pEle->nVR == VR_US)
	{
		if(nValueIndex*2>=pEle->nLen)
			return Ret_OutOfValueCount;
		nPos += nValueIndex*2;

		//unsigned char uc[2] = {m_buf[nPos],m_buf[nPos+1]};
		unsigned char uc[2] = {p[nPos],p[nPos+1]};
		if(pEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia)
		{
			//uc[0] = m_buf[nPos+1];	uc[1] = m_buf[nPos];
			uc[0] = p[nPos+1];	uc[1] = p[nPos];
		}

		unsigned short *r = (unsigned short *)uc;
		nValue = long(*r);
	}

	if (pEle->nVR == VR_SH || pEle->nVR == VR_IS)
	{
		int nRet = Ret_OutOfValueCount;

		char *pString = new char[pEle->nLen+1];
		memset(pString,0,pEle->nLen+1);

		//memcpy(pString,&(m_buf[pEle->nTagPos+pEle->nOffset]),pEle->nLen);
		memcpy(pString,p,pEle->nLen);

		QString sValue = QString(QLatin1String(pString));
		QStringList sections = sValue.split("\\");
		nValue = sections.at(nValueIndex).trimmed().toInt();
		delete []pString;

		return nRet;

	}

	if (pEle->nVR==VR_OB)
	{
		if(nValueIndex>pEle->nLen)
			return Ret_OutOfValueCount;

		//nValue = long(m_buf[pEle->nTagPos+pEle->nOffset+nValueIndex]);
		nValue = long(p[nValueIndex]);
	}

	if (pEle->nVR==VR_OW)
	{
		if(nValueIndex>pEle->nLen/2)
			return Ret_OutOfValueCount;

		//unsigned long nPos = pEle->nTagPos + pEle->nOffset + 2*nValueIndex;
		nPos = 2*nValueIndex;

		//unsigned char uc[2] = {m_buf[nPos],m_buf[nPos+1]};
		unsigned char uc[2] = {p[nPos], p[nPos+1]};
		if(pEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia)
		{
			//uc[0] = m_buf[nPos+1];	
			//uc[1] = m_buf[nPos];
			uc[0] = p[nPos+1];	
			uc[1] = p[nPos];
		}

		unsigned short *r = (unsigned short*)uc;
		nValue = long(*r);
	}

	return Ret_Success;
}

#pragma endregion ����


//Dcm�ļ�������ʼλ��(��һ��Tag��λ��)
bool CHsBaseFile::GetFirstTagPos(void)
{
	if (m_buf==NULL)
		return false;

	if(m_BaseFileInfo.nReadLen<4)
		return false;

	long iNotZeroPos = -1;
	for (long i=0;i<m_BaseFileInfo.nReadLen-3;i++)
	{
		if(m_buf[i]!=0)//��һ����0�ֽ�
		{
			if(iNotZeroPos==-1)
				iNotZeroPos = i;
		}
		if(m_buf[i]=='D' && m_buf[i+1]=='I' && m_buf[i+2]=='C' && m_buf[i+3]=='M')
		{
			m_BaseFileInfo.nCurPos = i+4;
			return true;
		}
	}
	
	m_BaseFileInfo.nCurPos = iNotZeroPos;

	return true;

}

int CHsBaseFile::Hs_LoadFile(const char *cFileName,bool bFullLoad)
{//�����ļ�,�ɹ�����TRUE,Recode=0;ʧ�ܷ���FALSE,Recode=�������
	Hs_CloseFile();
	FreeBuffer();
	delete m_pMainEle;
	m_pMainEle = NULL;

	if(cFileName==NULL)//Ϊ�ղ���
		return Ret_InvalidPara;

	//if (_access(cFileName,0)!=0)//�ļ������ڲ���---���ж��ˣ�����·���Ļ��˷�ʱ�䣬��������err�ᱨ��
	//	return Ret_InvalidPara;

	errno_t err = fopen_s( &m_fp, cFileName, "rb" );
	if( err !=0 )
		return Ret_LoadFileError;


	fseek(m_fp,0L,SEEK_SET); 
	fseek(m_fp,0L,SEEK_END);

	long nFullSize = ftell(m_fp);  //�ļ�����
	if ( nFullSize <= 4 )//�ļ�̫С����
	{
		if(m_fp)
		{	
			fclose(m_fp);
			m_fp = NULL;
		}

		return Ret_LoadFileError;
	}

	m_BaseFileInfo.sFileName = cFileName;
	m_BaseFileInfo.nFullSize = nFullSize;


	//����һֱ��,ֱ����������(����ͼ���ص�Ȼ����),���߶��������ļ�.�Ա㽫����Tag���������������νṹ
	long nToRead = m_nDefaultReadSize;

	while(1)
	{
		FreeBuffer();
		m_pPreEle = NULL;

		if (bFullLoad)
			nToRead = nFullSize;
		else
			nToRead = min(nFullSize , nToRead);//Ϊ������ٶ�,��ȫ����ȡ.

		m_buf = new unsigned char[nToRead]; //�ֽڴ��,�����ڴ�ռ�
		m_nBufLen = nToRead;

		fseek(m_fp,0L,SEEK_SET);
		size_t nReadCount = fread(m_buf, nToRead, 1,m_fp);

		if(nReadCount != 1)
		{
			FreeBuffer();

			if(m_fp)
			{	
				fclose(m_fp);
				m_fp = NULL;
			}

			return Ret_LoadFileError;
		}


		m_BaseFileInfo.nReadLen = (unsigned long)nToRead;

		if (GetFirstTagPos()==false)
		{
			Hs_CloseFile();
			FreeBuffer();
			return Ret_InvalidDicomFile;
		}

		//�ȶ�ȡ�����﷨
		if (GetTsType()!=Ret_Success)
		{
			FreeBuffer();
			Hs_CloseFile();

			return Ret_InvalidDicomFile;
		}


		if(m_pMainEle)
			delete m_pMainEle;

		m_pMainEle = new HsElement;
		m_pMainEle->nTag = 0;
		m_pMainEle->nTagPos = m_BaseFileInfo.nCurPos;

		int nRet = GetChildElement(m_BaseFileInfo.nCurPos,m_pMainEle);
		if (nRet>0)
			break;

		if (nRet==Ret_ReachPixelData)
			break;

		if (nRet==Ret_ReachFileEnd)
			break;

		if (nRet==Ret_ReachBufferEnd)
		{
			if (m_BaseFileInfo.nReadLen >= m_BaseFileInfo.nFullSize)
			{
				break;
			}
			else
			{
				nToRead = 2*nToRead;
				continue;
			}
		}
	}

	//�����list��ʽ
	BuildListRelation(m_pMainEle);

	//��m_buf�ַ�������Tag
	int nd = 0;
	pHsElement pEle = m_pMainEle;
	while(pEle)
	{
		if(pEle->pValue)
			delete []pEle->pValue;
		pEle->pValue = NULL;
		
		if (pEle->nVR == VR_SQ || pEle->nTag == TAG_ITEM)
		{
			pEle = pEle->pNextEle;
			continue;
		}
		if (pEle->nTag == TAG_PIXEL_DATA)
		{
			pEle = pEle->pNextEle;
			continue;
		}
		if (pEle->nTag == 0x56000020UL)
		{
			pEle = pEle->pNextEle;
			continue;
		}

		if (pEle->pChildEleV.empty() == false)//������tag
		{
			pEle = pEle->pNextEle;
			continue;
		}

		if (pEle->nLen > 0xFFFFFFFF-1)
		{
			pEle = pEle->pNextEle;
			continue;
		}

	
		//˽��Tag������
		unsigned long uGp = pEle->nTag>>16;
		if (uGp%2!=0)
		{//1,731,896
			pEle = pEle->pNextEle;
			continue;
		}

		int nRet = 0;
		unsigned long nByte = 0;
		BYTE* pByte = GetByteFromBuffer(pEle,nByte,nRet);
		pEle->pValue = pByte;

		//if(pEle->nTag == TAG_SOP_INSTANCE_UID)
		//{
		//	for (int i=0;i<pEle->nLen;i++)
		//	{
		//		if(i==55)
		//			int g = 0;
		//		AtlTrace("%02d: --------  [%c] -------  [%d] --------  [%x]\r\n",i,pByte[i],pByte[i],pByte[i]);
		//	}

		//}

		pEle = pEle->pNextEle;
	}

	delete []m_buf;
	m_buf = NULL;

	if(m_fp)//��Ҫ�رգ���Ϊ����̫��FILE*��HMY���ֵ�
	{	
		fclose(m_fp);
		m_fp = NULL;
	}


	int nRet = Hs_GetStringValueA(TAG_MODALITY,m_sModality);
	if( nRet == 0)
		IsXRay();//����ȷ��һ���ǲ����շ�ͼ��

	return 0;

}

int CHsBaseFile::GetTsType()
{
	int nRet = Ret_InvalidDicomFile;
	QString sTransferSnaxType = "";

	unsigned long  nTag = TAG_TRANSFER_SYNTAX_UID;

	//for (unsigned long nPos=0;nPos<m_BaseFileInfo.nReadLen-3;nPos++)
	unsigned long nPos = 0;
	while(nPos < m_BaseFileInfo.nReadLen)
	{//00 02 00 10 TAG_TRANSFER_SYNTAX_UID

		if (m_buf[nPos]==0x02 && m_buf[nPos+1]==0x00 && m_buf[nPos+2]==0x10 && m_buf[nPos+3]==0x00)//���Tag������С����ʽ���
		{
			bool bShowVR;
			unsigned long nVR = GetTagVR(nPos+4,nTag,bShowVR,nRet);
			pHsElement pEle = GetNormalElement(nPos,nTag,nVR,false,bShowVR,nRet);

			if(pEle==NULL)
				break;
			if(nRet!=0)
				break;

			nRet = Hs_GetStringValue(pEle,sTransferSnaxType,0);

			delete pEle;
			break;
		}
		else
		{
			nPos++;
		}

	}

	if(sTransferSnaxType.compare("1.2.840.10008.1.2")==0)			m_BaseFileInfo.nTsType = TS_IMPLICIT_VR_LITTLE_ENDIAN;
	else if(sTransferSnaxType.compare("1.2.840.10008.1.2.1")==0)	m_BaseFileInfo.nTsType = TS_EXPLICIT_VR_LITTLE_ENDIAN;
	else if(sTransferSnaxType.compare("1.2.840.10008.1.2.2")==0)	m_BaseFileInfo.nTsType = TS_EXPLICIT_VR_BIG_ENDIAN;
	else if(sTransferSnaxType.compare("1.2.840.10008.1.2.5")==0)	m_BaseFileInfo.nTsType = TS_RLE_LOSSLESS;
	else if(sTransferSnaxType.compare("1.2.840.10008.1.2.4.50")==0)	m_BaseFileInfo.nTsType = TS_JPEG_BASELINE_1;
	else if(sTransferSnaxType.compare("1.2.840.10008.1.2.4.51")==0)	m_BaseFileInfo.nTsType = TS_JPEG_EXTENDED_2_4;
	else if(sTransferSnaxType.compare("1.2.840.10008.1.2.4.57")==0)	m_BaseFileInfo.nTsType = TS_JPEG_LOSSLESS_NONHIER_14;
	else if(sTransferSnaxType.compare("1.2.840.10008.1.2.4.70")==0)	m_BaseFileInfo.nTsType = TS_JPEG_LOSSLESS_NONHIER_14B;
	else if(sTransferSnaxType.compare("1.2.840.10008.1.2.4.90")==0)	m_BaseFileInfo.nTsType = TS_JPEG2000_LOSSLESS_ONLY;
	else if(sTransferSnaxType.compare("1.2.840.10008.1.2.4.91")==0)	m_BaseFileInfo.nTsType = TS_JPEG2000;
	else
	{
		m_BaseFileInfo.nTsType = 0;
		//nRet = Ret_NoTransferSyntaxes;
        qDebug("\r\nû���ҵ������﷨Ĭ��Ϊ0");
		return Ret_Success;
	}
	//AtlTrace("\r\nTransferSnaxType:%d",m_BaseFileInfo.nTsType);
	return nRet;
}
int CHsBaseFile::GetTagNumber(unsigned long nStartPos,unsigned long &nTag,bool &bBigEndia)
{
	nTag = 0;
	if(nStartPos + 3 >= m_BaseFileInfo.nReadLen )
		return Ret_ReachBufferEnd;
	
	bBigEndia = false;
	nTag = m_buf[nStartPos+1]*16777216 + m_buf[nStartPos]*65536 + m_buf[nStartPos+3]*256 + m_buf[nStartPos+2];//С�˶���

	if ( m_BaseFileInfo.nTsType==TS_EXPLICIT_VR_BIG_ENDIAN )
	{
		if (nTag==0x00020000UL || nTag==0x00020102UL || nTag==0x00020001UL || nTag==0x00020002UL || nTag==0x00020003UL || 
			nTag==0x00020010UL || nTag==0x00020012UL || nTag==0x00020013UL || nTag==0x00020016UL || nTag==0x00020100UL )
		{
			
			return Ret_Success;
		}
		else
		{
			nTag = m_buf[nStartPos+2]*256 + m_buf[nStartPos+3] + m_buf[nStartPos]*16777216 + m_buf[nStartPos+1]*65536;//��˶���
			bBigEndia = true;

			return Ret_Success;
		}	
	}
	else
	{
		return Ret_Success;
	}

}

pHsElement CHsBaseFile::GetNormalElement(unsigned long &nStartPos,unsigned long nTag,unsigned long nVR,bool bBigEndia,bool bVrFieldShow,int &nRet)
{//��ȡ��Item���͵�Ԫ��
	//if(g_bDebug)
	//	AtlTrace("\r\nNormal:%d",nStartPos);

	//if (nStartPos==112554)
	//{
	//	for (int i=nStartPos-10;i<nStartPos+1000;i++)
	//	{
	//		AtlTrace("\r\n%d:	%d	0x%02x	%c",i,m_buf[i],m_buf[i],m_buf[i]);
	//	}
	//}

	unsigned long nPos = nStartPos;

	pHsElement pRetEle = new HsElement;
	pRetEle->nTag = nTag;
	pRetEle->nTagPos = nStartPos;
	pRetEle->nVR = nVR;
	pRetEle->bBigEndian = bBigEndia;

	//========================Tag Number��:4 Byte=======================================================
	if(nPos+3>m_BaseFileInfo.nReadLen)
	{
		pRetEle = DestroyEle(pRetEle) ;
		nRet = Ret_ReachBufferEnd;
		return NULL;
	}
	nPos += 4;//Խ��Tag num��


	//========================  VR��: 2 Byte===========================================================
	//Do nothing!
	pDcmVR pVR = CDcmVR::Find(nVR);

	//========================Value Length��: ���������;���ռ�����ֽ�=======================================================
	if (bVrFieldShow)
		pRetEle->nLenDesc = pVR->nLenOfLenDes;
	else
		pRetEle->nLenDesc = 4;

	if(bVrFieldShow)
		nPos += pVR->nLenOfVrDes;//Խ��������,����ֵ����������ĵ�һ���ֽ�

	//��ǰnPos=������ĵ�һ���ֽ�
	if(nPos+pVR->nLenOfLenDes > m_BaseFileInfo.nReadLen)
	{
		pRetEle = DestroyEle(pRetEle) ;
		nRet = Ret_ReachBufferEnd;
		return NULL;
	}


	unsigned long nDataSize = 0;
	if (bBigEndia==false)
	{
		if(pVR->nLenOfLenDes == 2)
			nDataSize = m_buf[nPos+1]*256 + m_buf[nPos];
		else if (pVR->nLenOfLenDes == 4)
			nDataSize = m_buf[nPos+3]*16777216 + m_buf[nPos+2]*65536 + m_buf[nPos+1]*256 + m_buf[nPos];//nDataSize = m_buf[nPos+1]*16777216 + m_buf[nPos]*65536 + m_buf[nPos+3]*256 + m_buf[nPos+2];
		else
		{
			pRetEle = DestroyEle(pRetEle) ;
			nRet = Ret_InvalidDicomVR;
			return NULL;
		}
	}
	else
	{
		if(pVR->nLenOfLenDes == 2)
			nDataSize = m_buf[nPos+1] + m_buf[nPos]*256;
		else if (pVR->nLenOfLenDes == 4)
			nDataSize = m_buf[nPos]*16777216 + m_buf[nPos+1]*65536 + m_buf[nPos+2]*256 + m_buf[nPos+3];//nDataSize = m_buf[nPos+1]*65536 + m_buf[nPos]*16777216 + m_buf[nPos+3] + m_buf[nPos+2]*256;
		else
		{
			pRetEle = DestroyEle(pRetEle) ;
			nRet = Ret_InvalidDicomVR;
			return NULL;
		}
	}

	if (bVrFieldShow==false)
		pRetEle->nOffset = 4 + pRetEle->nLenDesc;
	else
		pRetEle->nOffset = 4 + pVR->nLenOfVrDes + pRetEle->nLenDesc;

	pRetEle->nLen = nDataSize;

	if (pRetEle->nTagPos + pRetEle->nLen > m_BaseFileInfo.nFullSize  && nTag!=TAG_PIXEL_DATA)//����Pixel Data�����������
	{
		pRetEle = DestroyEle(pRetEle) ;
		nRet = Ret_ReachFileEnd;
		return NULL;
	}

	//========================Data Valueֵ��:(&m_filebuf[nCurPos]),nDataSize=======================================================
	nPos = nStartPos + pRetEle->nOffset;//����ֵ��ĵ�һ���ֽ�

	nRet = Ret_Success;
	if(nDataSize==0xFFFFFFFF && nTag==TAG_PIXEL_DATA)	//�е������п��ܳ���δָ�� 0xFFFFFFFF.E:\1ͼƬ\���ļ�\HugeDcm.dic
		nRet = Ret_ReachFileEnd;
	else
		nPos += nDataSize;//Խ��DataValue��,�����������Ԫ�صĵ�һ���ֽ�

	nStartPos = nPos;
	if(nStartPos>=m_BaseFileInfo.nFullSize)
		nRet = Ret_ReachFileEnd;

	return pRetEle;
}

pHsElement CHsBaseFile::GetSequenceElement(unsigned long &nStartPos,unsigned long nTag,bool bBigEndia,int &nRet)
{
	if(g_bDebug)
		qDebug("\r\nSQ:%d",nStartPos);

	unsigned long nPos = nStartPos;

	pHsElement pRetEle = new HsElement;
	pRetEle->nTag = nTag;
	pRetEle->nTagPos = nStartPos;
	pRetEle->nVR = VR_SQ;
	pRetEle->bBigEndian = bBigEndia;

	//========================��һ����:Tag Number��:4 Byte=======================================================
	//Do nothing!


	nPos += 4;//Խ��TagNm��,�����ڶ�����ĵ�һ���ֽ�
	//========================�ڶ�����:(������������4B ���� �ǳ�����4B)================================================

	if(nPos+3>=m_BaseFileInfo.nReadLen)//�жϷ���
	{
		pRetEle = DestroyEle(pRetEle) ;
		nRet = Ret_ReachBufferEnd;
		return NULL;
	}

	if (m_buf[nPos] == 0x53 && m_buf[nPos+1] == 0x51 && m_buf[nPos+2] == 0x00 && m_buf[nPos+3] == 0x00)	//�������� Table 7.5-2
	{
		//========================�����������������(4B)================================================
		
		nPos += 4;//Խ��������,��������������ĵ�һ���ֽ�
		if(nPos+3>m_BaseFileInfo.nReadLen)//�жϷ���
		{
			pRetEle = DestroyEle(pRetEle) ;
			nRet = Ret_ReachBufferEnd;
			return NULL;
		}

		if (m_buf[nPos] == 0xFF && m_buf[nPos+1] == 0xFF && m_buf[nPos+2] == 0xFF && m_buf[nPos+3] == 0xFF)
			pRetEle->nLen = 0xFFFFFFFF;
		else
		{
			if (bBigEndia)
				pRetEle->nLen = m_buf[nPos]*16777216 + m_buf[nPos+1]*65536 + m_buf[nPos+2]*256 + m_buf[nPos+3];
			else
				pRetEle->nLen = m_buf[nPos] + m_buf[nPos+1]*256 + m_buf[nPos+2]*65536 + m_buf[nPos+3]*16777216;
		}
		
		pRetEle->nOffset = 12;
	}
	else if (m_buf[nPos] == 0xFF && m_buf[nPos+1] == 0xFF && m_buf[nPos+2] == 0xFF && m_buf[nPos+3] == 0xFF)//�������� Table 7.5-3
	{//�ڶ�����Ϊδ���峤�ȵĳ�����:
		pRetEle->nLen = 0xFFFFFFFF;
		pRetEle->nOffset = 8;
	}
	else//�Ǿ�����ȷ���ȵĳ�����://�������� Table 7.5-1
	{

		if (bBigEndia)
			pRetEle->nLen = m_buf[nPos]*16777216 + m_buf[nPos+1]*65536 + m_buf[nPos+2]*256 + m_buf[nPos+3];//m_buf[nPos+1]*65536 + m_buf[nPos]*16777216 + m_buf[nPos+3] + m_buf[nPos+2]*256;
		else
			pRetEle->nLen = m_buf[nPos+3]*16777216 + m_buf[nPos+2]*65536 + m_buf[nPos+1]*256 + m_buf[nPos];//m_buf[nPos+1]*16777216 + m_buf[nPos]*65536 + m_buf[nPos+3]*256 + m_buf[nPos+2];

		pRetEle->nOffset = 8;
	}

	//========================ֵ��======================================================================================
	nPos = nStartPos + pRetEle->nOffset;//����ֵ��ĵ�һ���ֽ�(��������һ����Ŀ�ĵ�һ���ֽ�)
	if(nPos>=m_BaseFileInfo.nReadLen)
	{
		pRetEle = DestroyEle(pRetEle) ;
		nRet = Ret_ReachBufferEnd;
		return NULL;
	}

	if (pRetEle->nLen!=0xFFFFFFFF)//��ȷ���ȵ�
	{
		unsigned long tPos = nPos;


		while(tPos < nPos + pRetEle->nLen)
		{
			pHsElement pChildEle = GetItemElement(tPos,TAG_ITEM,bBigEndia,nRet);

			if (nRet==Ret_ReachFileEnd)
			{
				BuildTreeRelation(pChildEle,pRetEle,m_pPreEle);
				break;
			}
			else if (nRet==Ret_ReachBufferEnd)//��ǰbuffer������
			{				
				pRetEle = DestroyEle(pRetEle);
				pChildEle = DestroyEle(pChildEle);
				return NULL;
			}
			else if (nRet==Ret_ReachPixelData)
			{
				BuildTreeRelation(pChildEle,pRetEle,m_pPreEle);
				break;
			}
			else if (nRet!=Ret_Success)//������
			{
				pRetEle = DestroyEle(pRetEle);
				pChildEle = DestroyEle(pChildEle);
				return NULL;
			}

			//ȷ���ø��ӹ�ϵ
			BuildTreeRelation(pChildEle,pRetEle,m_pPreEle);

			m_pPreEle = pChildEle;			
		}

		nPos = tPos;

	}
	else//δ��ȷ���ȵ�
	{
		unsigned long tPos = nPos;

		while(1)
		{
			if(tPos+8>m_BaseFileInfo.nReadLen)//��ǰbuffer������
			{
				pRetEle = DestroyEle(pRetEle) ;
				nRet = Ret_ReachBufferEnd;
				return NULL;
			}

			//˵�����������ǽ�����.�����Ҷ��н�����--�����﷨Ӱ��--һֱ��С��
			if( (m_buf[tPos] == 0xFF && m_buf[tPos+1] == 0xFE && m_buf[tPos+2] == 0xE0 && m_buf[tPos+3] == 0xDD &&	m_buf[tPos+4] == 0x00 && m_buf[tPos+5] == 0x00 && m_buf[tPos+6] == 0x00 && m_buf[tPos+7] == 0x00) ||
				(m_buf[tPos] == 0xFE && m_buf[tPos+1] == 0xFF && m_buf[tPos+2] == 0xDD && m_buf[tPos+3] == 0xE0 &&	m_buf[tPos+4] == 0x00 && m_buf[tPos+5] == 0x00 && m_buf[tPos+6] == 0x00 && m_buf[tPos+7] == 0x00) )
			{
				tPos += 8;//Խ�����н�����
				nRet = Ret_Success;
				break;
			}

			pHsElement pChildEle = GetItemElement(tPos,TAG_ITEM,bBigEndia,nRet);

			if (nRet==Ret_ReachBufferEnd)//��ǰbuffer������
			{
				pRetEle = DestroyEle(pRetEle) ;
				pChildEle = DestroyEle(pChildEle) ;
				return NULL;
			}
			else if (nRet==Ret_ReachPixelData)
			{
				BuildTreeRelation(pChildEle,pRetEle,m_pPreEle);
				break;
			}
			else if (nRet==Ret_ReachFileEnd)
			{
				//ȷ���ø��ӹ�ϵ
				BuildTreeRelation(pChildEle,pRetEle,m_pPreEle);

				m_pPreEle = pChildEle;
				break;
			}
			else if (nRet!=Ret_Success)//������
			{
				pRetEle = DestroyEle(pRetEle) ;
				pChildEle = DestroyEle(pChildEle) ;
				return NULL;
			}


			//ȷ���ø��ӹ�ϵ
			BuildTreeRelation(pChildEle,pRetEle,m_pPreEle);

			m_pPreEle = pChildEle;
		}

		pRetEle->nLen = tPos - nPos;
		nPos = tPos;

	}

	nStartPos = nPos;

	return pRetEle;
}
pHsElement CHsBaseFile::GetItemElement(unsigned long &nStartPos,unsigned long nTag,bool bBigEndia,int &nRet)
{
	if(g_bDebug)
		qDebug("\r\nItem:%d",nStartPos);

	HsElement *pRetEle = new HsElement;
	pRetEle->nTag = TAG_ITEM;
	pRetEle->nTagPos = nStartPos;
	pRetEle->bBigEndian = bBigEndia;

	//========================��һ����:Tag Number��:4 Byte=======================================================
	//Do nothing!
	unsigned long nPos = nStartPos;//ITEM�ĵ�һ���ֽ�

	unsigned long xTag = 0;
	unsigned long xPos = nStartPos;
	bool xBigEdian = false;
	GetTagNumber(xPos,xTag,xBigEdian);
	pHsElement pVirtualItem = NULL;
	if(xTag!=TAG_ITEM)//�������ķ���,��Ȼ����������ͨԪ��,û��Item����,��ֻ��ֱ������ֵ����
	{
		//��ô�˴����൱��ֱ��Խ��һ������Item��TagNumber�򼰳�����,ֱ����������Item��ֵ����
		pRetEle->nLen = 0xFFFFFFFF;
		pRetEle->nOffset = 0;

		pRetEle->bVirtualItem = true;//���һ��,���Item�������.��ʵ��������,�Ա���GetChildElement���ж����Ľ�����Ӧ�������еĽ�����.

		nRet = GetChildElement(nPos,pRetEle);
		nStartPos = nPos;
		return pRetEle;
	}

	//========================�ڶ�����:(������4B)================================================================
	if(nPos+7>=m_BaseFileInfo.nReadLen)//���ٵ���8λ
	{
		pRetEle = DestroyEle(pRetEle) ;
		nRet = Ret_ReachBufferEnd;
		return NULL;
	}
	nPos += 4;//Խ��TagNum��,���볤����ĵ�һ���ֽ�

	if (m_buf[nPos]==0xFF && m_buf[nPos+1]==0xFF && m_buf[nPos+2]==0xFF && m_buf[nPos+3]==0xFF)//δ���峤��
	{
		pRetEle->nLen = 0xFFFFFFFF;
		pRetEle->nOffset = 8;
	}
	else//�����˳���
	{
		if (bBigEndia)
			pRetEle->nLen = m_buf[nPos]*16777216 + m_buf[nPos+1]*65536 + m_buf[nPos+2]*256 + m_buf[nPos+3];//m_buf[nPos+1]*65536 + m_buf[nPos]*16777216 + m_buf[nPos+3] + m_buf[nPos+2]*256;
		else
			pRetEle->nLen = m_buf[nPos+3]*16777216 + m_buf[nPos+2]*65536 + m_buf[nPos+1]*256 + m_buf[nPos];

		pRetEle->nOffset = 8;

		if (pRetEle->nTagPos + pRetEle->nLen>m_BaseFileInfo.nFullSize)
		{
			pRetEle = DestroyEle(pRetEle) ;
			nRet = Ret_ReachFileEnd;
			return NULL;
		}
	}
	//========================��������:(ֵ���Item������)=============================================================
	nPos += 4;//Խ��������,�����������ĵ�һ���ֽ�
	if(nPos>=m_BaseFileInfo.nReadLen)
	{
		pRetEle = DestroyEle(pRetEle) ;
		nRet = Ret_ReachBufferEnd;
		return NULL;
	}

	nRet = GetChildElement(nPos,pRetEle);

	nStartPos = nPos;

	return pRetEle;
}
unsigned long CHsBaseFile::GetTagVR(unsigned long nPos,unsigned long nTag,bool &bShowField,int &nRet)
{
	nRet = Ret_Success;
	bShowField = false;

	//======��ȥ�ļ���ֱ�Ӷ�,�е�Tag��ֹ��Ӧһ������,�����ļ���ָ�����������Ȳ���==================================
	if(nPos+1 > m_BaseFileInfo.nReadLen)//�����汣��
	{
		nRet = Ret_ReachBufferEnd;
		return 0;
	}

	unsigned long nVR = m_buf[nPos]*256 + m_buf[nPos+1];//��������ô������

	//======������δ�ؾ������VR,��Ϊ�е�Tagû��VR��,���Ƕ�����Ҳ�����ǳ������ǰ2���ֽ�.
	pDcmVR pVR = CDcmVR::Find(nVR);//����VR�ֵ��ж�һ��
	if (pVR->nCode!=0)//�ֵ����ж���,��ô���Ǹ��Ϸ���VR
	{
		bShowField = true;//��ʾ��Ԫ�غ���VR��
		return nVR;
	}
	else if (m_pPreEle!=NULL)
	{
		if (m_pPreEle->nTag != TAG_ITEM && m_pPreEle->nVR!=VR_SQ && m_pPreEle->nTag!=TAG_PIXEL_DATA && m_pPreEle->nLen!=0xFFFFFFFF && m_pPreEle->nLen%2!=0)//�����һ��Ԫ�ز�������Ԫ�ز��ҳ���������(һ�㲻��Ϊ����,��Ϊ������һ��Ԫ�غͿ������0��)
		{
			//�������Ųһ�ֽ�������.
			if(nPos+2 > m_BaseFileInfo.nReadLen)//�����汣��
			{
				nRet = Ret_ReachBufferEnd;
				return 0;
			}

			nVR = m_buf[nPos+1]*256 + m_buf[nPos+2];//��������ô������
			pDcmVR pVR1 = CDcmVR::Find(nVR);//����VR�ֵ��ж�һ��
			if (pVR1->nCode!=0)//�ֵ����ж���,��ô���Ǹ��Ϸ���VR
			{			
				bShowField = true;//��ʾ��Ԫ�غ���VR��
				nRet = Ret_FindOddLen;
				return nVR;
			}
		}
	}
	
	//======��������VR�Ƿ�,����Ϊ��û��VR��,��ȥTag�ֵ��в���.����Tag��Ӧ������
	nVR = CDcmTag::Find(nTag)->nVR;

	if(nVR!=0)//�ҵ���
		return nVR;//ֱ������.

	//======Tag�ֵ���û�ҵ����Tag,��˵�����ǵ��ֵ仹�ǲ�����,����,����Ϊû��VR���,����δ֪������(��һ����˽��Tag,������һ�㶼��4.VR�ֵ���VR_00�Ѷ�����4�ĳ���)
	nVR = VR_00;
	//��׼ Table 7.5-3 ��˵�����һ��Tag��������4��FF����ô���Ǹ�SQ
	if(m_buf[nPos] == 0xFF && m_buf[nPos+1] == 0xFF && m_buf[nPos+2] == 0xFF && m_buf[nPos+3] == 0xFF)
	{
		nVR = VR_SQ;
		bShowField = false;
	}

	return nVR;
}


// ��ȡpParentEle����������Element(��Ԫ��ֵ��ĵ�һ���ֽ�,��Ԫ��)
int CHsBaseFile::GetChildElement(unsigned long &nPos,pHsElement pParentEle)
{//�������ǻ�ȡĳ��Ԫ�ص�������һ����Ԫ��.���1�������ļ�����Ϊһ����ĸ�Ԫ��.���2��ĳItem������Ϊ��Ԫ��.����SQ������,��ΪSQֻ���н�Ϊ�����ITEM
	if(g_bDebug)
		qDebug("\r\nChild:%d",nPos);

	if (pParentEle==NULL)
		return Ret_InvalidPara;

	if (pParentEle->nTag == TAG_ITEM && pParentEle->nLen==0)
		return Ret_Success;


	int nRet = Ret_Success;
	unsigned long nTag = 0;
	unsigned long nCurPos = nPos;
	bool bBigEndia = false;

	while(true)
	{
		if (pParentEle->nTag==TAG_ITEM)//����������ڴ���Item�µĵ�һ��Ԫ��
		{
			if (pParentEle->bVirtualItem)//�Ƿ��������Item.
			{
				if (nCurPos+7>=m_BaseFileInfo.nReadLen)
				{
					nRet = Ret_ReachBufferEnd;
					break;
				}

				//�����Itemֹͣ��λ�ò���Item������,�����丸SQ�����н�����
				if ((m_buf[nCurPos]==0xFF && m_buf[nCurPos+1]==0xFE && m_buf[nCurPos+2]==0xE0 && m_buf[nCurPos+3]==0xDD &&	m_buf[nCurPos+4]==0x00 && m_buf[nCurPos+5]==0x00 && m_buf[nCurPos+6]==0x00 && m_buf[nCurPos+7]==0x00) || 
					(m_buf[nCurPos]==0xFE && m_buf[nCurPos+1]==0xFF && m_buf[nCurPos+2]==0xDD && m_buf[nCurPos+3]==0xE0 &&	m_buf[nCurPos+4]==0x00 && m_buf[nCurPos+5]==0x00 && m_buf[nCurPos+6]==0x00 && m_buf[nCurPos+7]==0x00) )//SQ�����зֽ������
				{					
					break;//���˽�������,���ؿ���.���൱�ڿ�Խ����������Item������.
				}
			}
			else//��Щ�����ڵ�Item��:
			{
				if (pParentEle->nLen==0xFFFFFFFF)//��ûָ������
				{
					if (nCurPos+7>=m_BaseFileInfo.nReadLen)
					{
						nRet = Ret_ReachBufferEnd;
						break;
					}
					if (	(m_buf[nCurPos]==0xFF && m_buf[nCurPos+1]==0xFE && m_buf[nCurPos+2]==0xE0 && m_buf[nCurPos+3]==0x0D &&	m_buf[nCurPos+4]==0x00 && m_buf[nCurPos+5]==0x00 && m_buf[nCurPos+6]==0x00 && m_buf[nCurPos+7]==0x00) || 
							(m_buf[nCurPos]==0xFE && m_buf[nCurPos+1]==0xFF && m_buf[nCurPos+2]==0x0D && m_buf[nCurPos+3]==0xE0 &&	m_buf[nCurPos+4]==0x00 && m_buf[nCurPos+5]==0x00 && m_buf[nCurPos+6]==0x00 && m_buf[nCurPos+7]==0x00) )//��Item�ֽ������
					{
						nCurPos += 8;//Խ��Item�Ľ�����
						break;
					}

				}
				else//��ָ���˳���
				{
					if (nCurPos>=m_BaseFileInfo.nReadLen)
					{
						nRet = Ret_ReachBufferEnd;
						break;
					}

					if (nCurPos >= pParentEle->nTagPos + pParentEle->nOffset + pParentEle->nLen)
					{
						nRet = Ret_Success;
						break;
					}

				}
			}

		}
		else
		{
			if (nCurPos>=m_BaseFileInfo.nReadLen)
			{
				nRet = Ret_ReachBufferEnd;
				break;
			}			
		}


 		nRet = GetTagNumber(nCurPos,nTag,bBigEndia);

		if (nTag == 2145386512)
		{
			int a = 0;
		}

		if(nTag == 0x00190019)
		int dt = 0;

		if( nRet== Ret_ReachBufferEnd)//���굱ǰBuffer��
		{
			qDebug("\r\nnRet== Ret_ReachBufferEnd");
			break;
		}
		else if (nRet!=Ret_Success)//������
		{
			qDebug("\r\nnRet== %d",nRet);
			break;
		}
		else//�ɹ���,����
		{
			//AtlTrace("\r\n0x%08x:%s,",nTag,CDcmTag::Find(nTag)->pszName);
		}

		pHsElement pEle = NULL;

		//Ϊ�˻�ȡVR���ķ���
		if (nCurPos+5 > m_BaseFileInfo.nReadLen)
		{
			nRet = Ret_ReachBufferEnd;
			break;
		}

		bool bShowVR;
		unsigned long nVR = GetTagVR(nCurPos+4,nTag,bShowVR,nRet);
//=============��ЩTag��UNKNOWN�͵ģ�ȴ������Tag��Item�ģ���Ҫ��������=====
		if (nVR == VR_UN)
		{
			if (nTag == 0x0019108A || nTag == 0x0040100A || nTag == 0x00081250 || nTag == 0x0040100A || nTag == 0x7E011010)
			{
				m_buf[nCurPos+4] = 'S';
				m_buf[nCurPos+5] = 'Q';
				m_buf[nCurPos+6] = 0;
				m_buf[nCurPos+7] = 0;
				nVR = VR_SQ;
			}
		}

//==========================================================================
		if (nVR==0)
			break;

		if (nRet ==	Ret_FindOddLen)//����ֵ����Ϊ�������µ�����,������������ȡ�ɹ�
		{	
			//������һ��Ele
			m_pPreEle->nLen+=1;
			nCurPos+=1;
			nRet = GetTagNumber(nCurPos,nTag,bBigEndia);
		}

		if ( nVR == VR_SQ)
		{
			pEle = GetSequenceElement(nCurPos,nTag,bBigEndia,nRet);
			if( nRet== Ret_ReachBufferEnd || nRet== Ret_ReachFileEnd)//���굱ǰBuffer��
			{
				if(pEle)
					BuildTreeRelation(pEle,pParentEle,m_pPreEle);
				break;
			}
			else if (nRet== Ret_ReachPixelData)
			{
			}
			else if (nRet!=Ret_Success)//������
			{
				pEle = DestroyEle(pEle) ;

				pEle = NULL;
				break;
			}
			else//�ɹ���,����
			{

			}
		}
		else if (nTag == TAG_ITEM)//�ټ�
		{
			pEle = GetItemElement(nCurPos,nTag,bBigEndia,nRet);
			if( nRet== Ret_ReachBufferEnd || nRet== Ret_ReachFileEnd)//���굱ǰBuffer��
			{
				if(pEle)
					BuildTreeRelation(pEle,pParentEle,m_pPreEle);
				break;
			}
			else if (nRet!=Ret_Success)//������
			{
				pEle = DestroyEle(pEle) ;
				break;
			}
			else//�ɹ���,����
			{

			}
		}
		else
		{
			pEle = GetNormalElement(nCurPos,nTag,nVR,bBigEndia,bShowVR,nRet);

			if( nRet== Ret_ReachBufferEnd || nRet== Ret_ReachFileEnd)//���굱ǰBuffer��
			{
				if(pEle)
					BuildTreeRelation(pEle,pParentEle,m_pPreEle);
				break;
			}
			else if (nRet!=Ret_Success)//������
			{
				pEle = DestroyEle(pEle);

				break;
			}
			else//�ɹ���,����
			{
				if (nTag == TAG_PIXEL_DATA)//��������ʱ
				{
					//if (pParentEle->nTag != TAG_ITEM)//�����Ԫ�ز���Item,��ôҪ����һ���Ƿ񵽴��ļ�ĩβ
					{
						if (pEle->nTagPos + pEle->nOffset + pEle->nLen > m_BaseFileInfo.nFullSize - Len_AfterPixelData)
						{
							//ҲҪ������,�嵽����
							BuildTreeRelation(pEle,pParentEle,m_pPreEle);

							nRet = Ret_ReachPixelData;
							break;
						}
					}
				}

			}
		}

		BuildTreeRelation(pEle,pParentEle,m_pPreEle);
		m_pPreEle = pEle;
	}

	nPos = nCurPos;
	if(nPos==m_BaseFileInfo.nFullSize)
		nRet = Ret_ReachFileEnd;

	return nRet;
}


void CHsBaseFile::BuildTreeRelation(pHsElement pCurEle,pHsElement pParentEle,pHsElement pPreEle)
{
	//if (pPreEle)
	//	pPreEle->pNextEle = pCurEle;

	//pCurEle->pPreEle = pPreEle;

	pParentEle->pChildEleV.push_back(pCurEle);

	pCurEle->pParentEle = pParentEle;

}

pHsElement CHsBaseFile::Hs_FindFirstEle(pHsElement pSiblingEle,unsigned long nTag,bool bAsTree)
{
	if(m_pMainEle==NULL)
		return NULL;

	if (m_pMainEle->pChildEleV.empty()==true)
		return NULL;
	
	if (bAsTree==true)//���β��ң���ʾ��pSiblingEleͬ������
	{
		if (pSiblingEle==NULL)//��Ϊ�գ���ʾ�ڵ�һ������
			pSiblingEle = m_pMainEle->pChildEleV[0];

		pHsElement pParent = pSiblingEle->pParentEle;
		int n = pParent->pChildEleV.size();

		for (int i=0;i<n;i++)
		{
			if (pParent->pChildEleV[i]->nTag == nTag)
				return pParent->pChildEleV[i];
		}

		return NULL;
	}
	else
	{
		pHsElement pEle = m_pMainEle->pChildEleV[0];

		while(1)
		{
			if (pEle->nTag == nTag)
				return pEle;
			else
				pEle = pEle->pNextEle;

			if(pEle==NULL)
				return NULL;
		}
	}

	return NULL;
}

pHsElement CHsBaseFile::Hs_FindNextEle(pHsElement pEle,bool bTree)
{//��pEle��ʼ����,������pElen��Tag��ȵ�Ԫ��.bTree==true:ֻ��ͬ������,������Ϊlist���������ļ�
	if (pEle==NULL)
		return NULL;

	if(m_pMainEle==NULL)
		return NULL;

	if(m_pMainEle->pChildEleV.empty()==true)
		return NULL;

	if (bTree)
	{
		pHsElement pParentEle = pEle->pParentEle;

		int  n = pParentEle->pChildEleV.size();

		for (int i=0;i<n;i++)
		{
			if (pParentEle->pChildEleV[i]->nTag == pEle->nTag && pParentEle->pChildEleV[i]->nTagPos > pEle->nTagPos)
				return pParentEle->pChildEleV[i];
		}

	}
	else
	{
		pHsElement p = m_pMainEle->pChildEleV[0];

		while(1)
		{
			if (p->nTag == pEle->nTag && p->nTagPos > pEle->nTagPos)
				return p;
			else
				p = p->pNextEle;

			if(p==NULL)
				return NULL;
		}
	}

	return NULL;
}

pHsElement CHsBaseFile::Hs_GetChildEle(pHsElement pEle,unsigned long nTag,int nIndex)
{//����pEle��ֱ���¼�Ԫ����,��nIndex��,��tag��=nTag��Ԫ��
	if (pEle==NULL)
		pEle = m_pMainEle;

	if(pEle == NULL)
		return NULL;

	int n = int(pEle->pChildEleV.size());
	if (nTag>0)//��Ϊ����pEleֱ����Ԫ���в���tag��=nTag��Ԫ��
	{
		for (int i=0;i<n;i++)
		{
			if(pEle->pChildEleV[i]->nTag==nTag)
				return pEle->pChildEleV[i];
		}
	}
	else//��Ϊ��Ҫ���nIndex��Ԫ��,��������Tag��
	{
		if (nIndex>=n || nIndex<0)
			return NULL;

		return pEle->pChildEleV[nIndex];
	}


	return NULL;

}
pHsElement CHsBaseFile::Hs_FindPreEle(pHsElement pEle,bool bTree)
{//��pEle���ϲ�����pELe��nTag��ͬ��Ԫ��.bTree==true:ֻ��pEleͬ���в���,������list��ʽ���������ļ�
	if (pEle==NULL)
		return NULL;

	if(m_pMainEle==NULL)
		return NULL;

	if(m_pMainEle->pChildEleV.empty()==true)
		return NULL;

	if (bTree)
	{
		pHsElement pParentEle = pEle->pParentEle;

		int n = pParentEle->pChildEleV.size();

		for (int i=0;i<n;i++)
		{
			if (pParentEle->pChildEleV[i]->nTag == pEle->nTag && pParentEle->pChildEleV[i]->nTagPos < pEle->nTagPos)
				return pParentEle->pChildEleV[i];
		}
	}
	else
	{
		pHsElement p = m_pMainEle->pChildEleV[0];

		while(1)
		{
			if (p->nTag == pEle->nTag && p->nTagPos < pEle->nTagPos)
				return p;
			else
				p = p->pPreEle;

			if(p==NULL)
				return NULL;
		}
	}

	return NULL;
}
int CHsBaseFile::DivString(char* pString,const char* pDelChar,int nID, QString &sValue)
{
    sValue = QString(QLatin1String(pString));
	QStringList sections = sValue.split(pDelChar);
	if (nID >= sections.size())
		return Ret_OutOfValueCount;

	sValue = sections.at(nID).trimmed();
	return Ret_Success;
}

int CHsBaseFile::Hs_GetValueCount(pHsElement pEle,int &nRet)
{
	if (pEle == NULL)
	{
		nRet = Ret_InvalidPara;
		return 0;
	}

	nRet = Ret_Success;
	if (pEle->nVR == VR_SQ || pEle->nTag==TAG_ITEM)
		return int(pEle->pChildEleV.size());

	if (pEle->nLen==0 || pEle->pValue == NULL)
		return 0;

	if(pEle->nVR == VR_LT || pEle->nVR == VR_ST ||pEle->nVR == VR_UN || pEle->nVR == VR_UT )
		return 1;



	//��������
	if (pEle->nVR == VR_AT || pEle->nVR == VR_AS ||
		pEle->nVR == VR_FL || pEle->nVR == VR_FD || 
		pEle->nVR == VR_SL || pEle->nVR == VR_SS || 
		pEle->nVR == VR_UL || pEle->nVR == VR_US)
	{
		int nSingleLen = CDcmVR::Find(pEle->nVR)->nValueLeng;

		return pEle->nLen/nSingleLen;
	}
	
	//�ַ�������
	if (pEle->nVR == VR_DS || pEle->nVR == VR_PN ||
		pEle->nVR == VR_AE || pEle->nVR == VR_CS ||
		pEle->nVR == VR_IS || pEle->nVR == VR_LO ||
		pEle->nVR == VR_SH || pEle->nVR == VR_UI ||		pEle->nVR == VR_OF)
	{
		int nLen = pEle->nLen+1;
		char *pString = new char[nLen];
 		memset(pString,0,nLen);

		//memcpy(pString,&(m_buf[pEle->nTagPos+pEle->nOffset]),pEle->nLen);
		memcpy(pString,pEle->pValue,pEle->nLen);

		QString sValue = QString(QLatin1String(pString));
		QStringList sections = sValue.split("\\");
		int nValueCount = sections.size();

		delete []pString;
		return nValueCount;
	}
	//����
	if (pEle->nVR == VR_DA)
	{
		int k = 0;
		for(unsigned long i=0;i<pEle->nLen;i++)
		{			
			//char c = m_buf[pEle->nTagPos + pEle->nOffset + i];
			char c = pEle->pValue[i];

			if(c>='0' && c<='9')
				k++;
		}

		return k/8;
	}
	//ʱ��
	if (pEle->nVR == VR_TM)
	{
		char *pString = new char[pEle->nLen+1];
		memset(pString,0,pEle->nLen+1);

		int k = 0;
		for(unsigned long i=0;i<pEle->nLen;i++)
		{
			//char c = m_buf[pEle->nTagPos + pEle->nOffset + i];
			char c = pEle->pValue[i];
			if( (c>='0' && c<='9') || c=='.' )
				pString[k++] = c;
			else
			{
				if(c!=0 && c!=' ')
					pString[k++] = '\\';//������Щ��̬Dcm�ļ�д��'-',���Բ���ô�鷳
			}
		}

		QString sValue = QString(QLatin1String(pString));
		QStringList sections = sValue.split("\\");
		int nValueCount = sections.size();

		delete []pString;
		return nValueCount;
	}
	//����ʱ��
	if (pEle->nVR == VR_DT)
	{
		char *pString = new char[pEle->nLen+1];
		memset(pString,0,pEle->nLen+1);

		int k = 0;
		for(unsigned long i=0;i<pEle->nLen;i++)
		{
			//char c = m_buf[pEle->nTagPos + pEle->nOffset + i];
			char c = pEle->pValue[i];
			if( (c>='0' && c<='9') || c=='.' || c=='+'|| c=='-')//��Щ���ǺϷ��ַ�
				pString[k++] = c;
			else//������ַ�
			{
				if(c!=0 && c!=' ' )//0�Ϳո���
					pString[k++] = '\\';//Ҫ�վ�ת��\\.----������Щ��̬Dcm�ļ�д��'-',���Բ���ô�鷳
			}
		}

		QString sValue = QString(QLatin1String(pString));
		QStringList sections = sValue.split("\\");
		int nValueCount = sections.size();

		delete []pString;
		return nValueCount;
	}

	if (pEle->nVR==VR_OB && pEle->nTag!=TAG_PIXEL_DATA)
	{
		return pEle->nLen;
	}
	if (pEle->nVR==VR_OW && pEle->nTag!=TAG_PIXEL_DATA)
	{
		return pEle->nLen/2;
	}

	//�������� OB OW��һЩ��������������
	if(pEle->nLen==0)
		return 0;
	else
		return 1;

	return 0;
}

QString CHsBaseFile::Hs_GetConvertValue(pHsElement pEle, int nValueIndex ,int& nRet)
{
	if (pEle==NULL)
	{
		nRet = Ret_InvalidPara;
		return "";
	}
	
	if (pEle->nLen==0 || pEle->nTag==TAG_ITEM || pEle->nVR==VR_SQ || pEle->nTag==TAG_PIXEL_DATA)
	{
		nRet = Ret_InvalidPara;
		return "";
	}

	if(pEle->nVR==VR_UL || pEle->nVR==VR_SL || pEle->nVR==VR_SS || pEle->nVR==VR_US)
	{
		long nValue = 0;
		nRet = Hs_GetLongValue(pEle,nValue, nValueIndex);

		if(nRet==Ret_Success)
		{
			char cc[100];
			sprintf(cc,"%d",nValue);

			return cc;
		}
		else
		{
			return "?";
		}
	}

	if (pEle->nVR == VR_AS)
	{
		int nAge = 0;
		char cType = 0;
		nRet = Hs_GetAgeValue(pEle,nAge,cType);
		if (nRet==Ret_Success)
		{
			char cc[100];
			sprintf(cc,"%d%c",nAge,cType);

			return cc;
		}
		else
		{
			return "?";
		}
	}

	if (pEle->nVR==VR_DS || pEle->nVR==VR_FD || pEle->nVR==VR_FL)
	{
		double fValue = 0.00;
		nRet = Hs_GetDoubleValue(pEle,fValue,nValueIndex);
		if (nRet==Ret_Success)
		{
			char cc[100];
			sprintf(cc,"%f",fValue);

			return cc;
		}
		else
		{
			return "?";
		}
	}

	if (pEle->nVR==VR_OF)
		return "����OF,������";

	if (pEle->nVR==VR_OW)
	{
		//return "VR_OW";
		long v;
		Hs_GetLongValue(pEle,v,nValueIndex);

		char cc[100];
		sprintf(cc,"%d",v);

		return cc;
	}

	if (pEle->nVR==VR_OB)
	{			
		long v;
		Hs_GetLongValue(pEle,v,nValueIndex);
		char cc[100];
		sprintf(cc,"%02X",v);

		return cc;
	}


	if (pEle->nVR==VR_DA)
	{
		HsDateTime t;
		nRet = Hs_GetDateValue(pEle,t,nValueIndex);
			
		if (nRet==Ret_Success)
		{
			char cc[200];
			sprintf(cc,"%04d-%02d-%02d",	t.nYear,	t.nMonth,	t.nDay);

			return cc;
		}
		else
		{
			return "?";
		}

	}

	if (pEle->nVR==VR_DT)
	{
		HsDateTime t;
		nRet = Hs_GetDateTimeValue(pEle,t,nValueIndex);

		if (nRet==Ret_Success)
		{
			char cc[200];
			sprintf(cc,"%04d-%02d-%02d %02d:%02d:%02d",	t.nYear,	t.nMonth,	t.nDay,	t.nHours,	t.nMinutes,  t.nSeconds);//.%03d, t.nFractions

			return cc;
		}
		else
		{
			return "?";
		}
	}

	if (pEle->nVR==VR_TM)
	{
		HsDateTime t;
		nRet = Hs_GetTimeValue(pEle,t,nValueIndex);

		if (nRet==Ret_Success)
		{
			char cc[200];
			sprintf(cc,"%02d:%02d:%02d",	t.nHours, t.nMinutes,  t.nSeconds);//.%03d, t.nFractions--ҽԺ��ϲ������

			return cc;
		}
		else
		{
			return "?";
		}
	}

	QString sRet = "";
	nRet = Hs_GetStringValue(pEle,sRet,nValueIndex);
	if (nRet==Ret_Success)
		return sRet;
	else
		return "?";

	
}


int CHsBaseFile::Hs_GetImageInfo(pHsElement pPixEle, ImageInfo& ImgInfo,int iFrame)
{
	if (m_pMainEle==NULL)
		return Ret_InvalidBuf;

	if(pPixEle==NULL)
		return Ret_InvalidPara;

	ImgInfo.sFileName = m_BaseFileInfo.sFileName;
	ImgInfo.bValid = true;
	ImgInfo.iFrame = iFrame;

	//iCompress
	if(m_BaseFileInfo.nTsType==TS_IMPLICIT_VR_LITTLE_ENDIAN || m_BaseFileInfo.nTsType==TS_EXPLICIT_VR_LITTLE_ENDIAN || m_BaseFileInfo.nTsType==TS_EXPLICIT_VR_BIG_ENDIAN)
		ImgInfo.iCompress = 0;
	else
		ImgInfo.iCompress = m_BaseFileInfo.nTsType;

	ImgInfo.bBigEndia = m_BaseFileInfo.nTsType==TS_EXPLICIT_VR_BIG_ENDIAN ? true : false;

	int nRet = 0;
	pHsElement p = NULL;

	//iAcquisitionNum
	p = Hs_FindFirstEle(NULL,TAG_ACQUISITION_NUMBER,true);
	if (p)
		Hs_GetLongValue(p,ImgInfo.iAcquisitionNum,0);

	//ScanOptions
	p = Hs_FindFirstEle(NULL,TAG_SCAN_OPTIONS,true);
	if (p)
		Hs_GetStringValue(p,ImgInfo.sScanOptions,0);

	//sPhotometric
	p = Hs_FindFirstEle(pPixEle,TAG_PHOTOMETRIC_INTERPRETATION,true);
	nRet = Hs_GetStringValue(p,ImgInfo.sPhotometric,0);

	if (ImgInfo.sPhotometric.isEmpty())
		ImgInfo.sPhotometric = "MONOCHROME2";

	if(int( ImgInfo.sPhotometric.indexOf("MONOCHROME1") )>=0)
	{
		ImgInfo.bInverse = true;
		ImgInfo.bGrayImg = true;
	}
	else if (int( ImgInfo.sPhotometric.indexOf("MONOCHROME2") )>=0)
	{
		ImgInfo.bInverse = false;
		ImgInfo.bGrayImg = true;
	}
	else 
	{
		ImgInfo.bInverse = false;
		ImgInfo.bGrayImg = false;
	}

	//Rows
	p = Hs_FindFirstEle(pPixEle,TAG_ROWS,true);
	nRet = Hs_GetLongValue(p,ImgInfo.nRows,0);
	ImgInfo.nOriRows = ImgInfo.nRows;

	//Col
	p = Hs_FindFirstEle(pPixEle,TAG_COLUMNS,true);
	nRet = Hs_GetLongValue(p,ImgInfo.nCols,0);
	ImgInfo.nOriCols = ImgInfo.nCols;

	//nSamplePerPixel
	p = Hs_FindFirstEle(pPixEle,TAG_SAMPLES_PER_PIXEL,true);
	nRet = Hs_GetLongValue(p,ImgInfo.nSamplePerPixel,0);
	if(ImgInfo.nSamplePerPixel==0)
		ImgInfo.nSamplePerPixel = 1;
	if(ImgInfo.nSamplePerPixel>=256)//�����ĵ�MR��ô����׼��,��Ȼx256
		ImgInfo.nSamplePerPixel /=256;

	//iPlanarConfig
	p = Hs_FindFirstEle(pPixEle,TAG_PLANAR_CONFIGURATION,true);
	nRet = Hs_GetLongValue(p,ImgInfo.iPlanarConfig,0);	

	//nBitsAllocated
	p = Hs_FindFirstEle(pPixEle,TAG_BITS_ALLOCATED,true);
	nRet = Hs_GetLongValue(p,ImgInfo.nBitsAllocated,0);
	if(ImgInfo.nBitsAllocated>=256)//�����ĵ�MR��ô����׼��,��Ȼx256
		ImgInfo.nBitsAllocated /=256;

	//nBitStored
	p = Hs_FindFirstEle(pPixEle,TAG_BITS_STORED,true);
	nRet = Hs_GetLongValue(p,ImgInfo.nBitStored,0);
	if(ImgInfo.nBitStored>=256)//�����ĵ�MR��ô����׼��,��Ȼx256
		ImgInfo.nBitStored /=256;

	//iHighBit
	p = Hs_FindFirstEle(pPixEle,TAG_HIGH_BIT,true);
	nRet = Hs_GetLongValue(p,ImgInfo.iHighBit,0);
	if(ImgInfo.iHighBit>=256)//�����ĵ�MR��ô����׼��,��Ȼx256
		ImgInfo.iHighBit /=256;

	//nPixelRepresentation ��λ����
	p = Hs_FindFirstEle(pPixEle,TAG_PIXEL_REPRESENTATION,true);
	nRet = Hs_GetLongValue(p,ImgInfo.nPixelRepresentation,0);

	//fPixelSpace
	p = Hs_FindFirstEle(pPixEle,TAG_PIXEL_SPACING,true);//0028,0030//TAG_IMAGER_PIXEL_SPACING;0018,1164//TAG_NOMINAL_SCANNED_PIXEL_SPACING;0018,2010
	nRet = Hs_GetDoubleValue(p,ImgInfo.fPixelSpaceY,0);
	nRet = Hs_GetDoubleValue(p,ImgInfo.fPixelSpaceX,1);

	if (nRet!=0)
	{
		p = Hs_FindFirstEle(pPixEle,TAG_IMAGER_PIXEL_SPACING,true);
		nRet = Hs_GetDoubleValue(p,ImgInfo.fPixelSpaceY,0);
		nRet = Hs_GetDoubleValue(p,ImgInfo.fPixelSpaceX,1);

		//����ֱ����Imager Pixel Spacing
		if (nRet==Ret_Success)
		{
			pHsElement pZoom = Hs_FindFirstEle(pPixEle,TAG_ESTIMATED_RADIOGRAPHIC_MAGNIFICATION_FACTOR,true);
			double fv = -1.00;

			if (pZoom)
				Hs_GetDoubleValue(pZoom,fv,0);
			
			if(fv < 0.00001)//û�õ�����ֵ,�Լ�ȥ����һ�£�Ҳδ�سɹ�
			{
				pHsElement pd = Hs_FindFirstEle(pPixEle,TAG_DISTANCE_SOURCE_TO_DETECTOR,true);
				pHsElement pp = Hs_FindFirstEle(pPixEle,TAG_DISTANCE_SOURCE_TO_PATIENT,true);
				if (pd && pp)
				{
					double fd = 0.00;
					Hs_GetDoubleValue(pd,fd,0);
					double fp = 0.00;
					Hs_GetDoubleValue(pp,fp,0);

					if (fd>0.00001 && fp>0.00001)
						fv = fd/fp;
				}
			}

			if(fv>0.00001)//���ɹ���
			{
				ImgInfo.fPixelSpaceX/=fv;
				ImgInfo.fPixelSpaceY/=fv;
			}

		}

	}

	//sImageType
	p = Hs_FindFirstEle(pPixEle,TAG_IMAGE_TYPE,true);
	if (p)
	{
		int n = Hs_GetValueCount(p,nRet);
		for(int i=0;i<n;i++)
		{
			QString s = "";
			Hs_GetStringValue(p,s,i);
			if(i>0)
				ImgInfo.sImageType += "\\";

			ImgInfo.sImageType += s;
		}
	}

	//BitsPerPixel
	ImgInfo.nBitsPerPixel = ImgInfo.nBitsAllocated * ImgInfo.nSamplePerPixel;

	//nSmallestPixelValue
	p = Hs_FindFirstEle(pPixEle,TAG_SMALLEST_IMAGE_PIXEL_VALUE,true);
	Hs_GetLongValue(p,ImgInfo.nSmallestPixelValue,0);

	//nLargestPixelValue
	p = Hs_FindFirstEle(pPixEle,TAG_SMALLEST_IMAGE_PIXEL_VALUE,true);
	Hs_GetLongValue(p,ImgInfo.nLargestPixelValue,0);

	//nFrame
	ImgInfo.nFrame = m_BaseFileInfo.nFrame;

	//p = Hs_FindFirstEle(pPixEle,TAG_NUMBER_OF_FRAMES,true);
	//if(p)//����Ƕ�֡�����������Tag,��֡�Ŀ���û��
	//	Hs_GetLongValue(p,ImgInfo.nFrame,0);
	//else
	//	ImgInfo.nFrame = 1;

	/*
	if(ImgInfo.nFrame==0)//û�ҵ�
	{//�ܷ����leadtools �ҵ�֡����
		if (ImgInfo.iCompress==0)
		{ 
			if(pPixEle->nLen==0)
				ImgInfo.nFrame = 0;
			else if(pPixEle->nLen==0xFFFFFFFF)
				ImgInfo.nFrame = -1;//����֮��
			else
			{
				unsigned long nBytePerFrame = (ImgInfo.nBitsAllocated/8) * ImgInfo.nRows * ImgInfo.nCols * ImgInfo.nSamplePerPixel;
				if(nBytePerFrame!=0)
					ImgInfo.nFrame = pPixEle->nLen/nBytePerFrame;
				else
					ImgInfo.nFrame = -1;
			}
		}
		else//LeadTool��ȷ��֡��
		{
			LDicomDS ds;
			char cfile[512];
			strcpy(cfile,m_BaseFileInfo.sFileName.c_str());
			if(ds.LoadDS(cfile, 0)!=DICOM_SUCCESS)
				return Ret_LoadFileError;

			pDICOMELEMENT pLtPix = GetLtEleByMyEle(pPixEle,&ds);

			DICOMIMAGE LtInfo;
			ds.GetInfoImage(pLtPix,&LtInfo,0);
			ImgInfo.nFrame = LtInfo.nFrames;
		}

	}
	*/

	//////////////////////////////////////////////////////////////////////////

	ImgInfo.pEle = pPixEle;

	p = Hs_FindFirstEle(pPixEle,TAG_OVERLAY_TYPE,true);
	Hs_GetStringValue(p,ImgInfo.sOverlayType,0);

	p = Hs_FindFirstEle(pPixEle,TAG_OVERLAY_MAGNIFICATION_TYPE,true);
	Hs_GetStringValue(p,ImgInfo.sOverlayMagnificationType,0);

	p = Hs_FindFirstEle(pPixEle,TAG_OVERLAY_OR_IMAGE_MAGNIFICATION,true);
	Hs_GetStringValue(p,ImgInfo.sOverlayOrImageMagnification,0);

	p = Hs_FindFirstEle(pPixEle,TAG_OVERLAY_SMOOTHING_TYPE,true);
	Hs_GetStringValue(p,ImgInfo.sOverlaySmoothingType,0);

	p = Hs_FindFirstEle(pPixEle,TAG_FRAME_LABEL_VECTOR,true);
	Hs_GetStringValue(p,ImgInfo.sFrameLabelVector,0);

	p = Hs_FindFirstEle(pPixEle,TAG_RESCALE_INTERCEPT,true);
	Hs_GetDoubleValue(p,ImgInfo.fRescaleIntercept,0);

	p = Hs_FindFirstEle(pPixEle,TAG_RESCALE_SLOPE,true);
	if(Hs_GetDoubleValue(p,ImgInfo.fRescaleSlope,0) != Ret_Success)
		ImgInfo.fRescaleSlope = 1.00;

	p = Hs_FindFirstEle(pPixEle,TAG_WINDOW_CENTER,true);
	Hs_GetDoubleValue(p,ImgInfo.fWinCenter,0);

	p = Hs_FindFirstEle(pPixEle,TAG_WINDOW_WIDTH,true);
	Hs_GetDoubleValue(p,ImgInfo.fWinWidth,0);

	p = Hs_FindFirstEle(pPixEle,TAG_OVERLAY_BIT_POSITION,true);
	Hs_GetLongValue(p,ImgInfo.iOverlayBitPosition,0);

	p = Hs_FindFirstEle(pPixEle,TAG_OVERLAY_BITS_ALLOCATED,true);
	Hs_GetLongValue(p,ImgInfo.nOverlayBitsAllocated,0);

	p = Hs_FindFirstEle(pPixEle,TAG_OVERLAY_COLUMNS,true);
	Hs_GetLongValue(p,ImgInfo.nOverlayCols,0);

	p = Hs_FindFirstEle(pPixEle,TAG_OVERLAY_ROWS,true);
	Hs_GetLongValue(p,ImgInfo.nOverlayRows,0);

	p = Hs_FindFirstEle(pPixEle,TAG_OVERLAY_ORIGIN,true);
	Hs_GetLongValue(p,ImgInfo.nOverlayOrigin1,0);

	p = Hs_FindFirstEle(pPixEle,TAG_OVERLAY_ORIGIN,true);
	Hs_GetLongValue(p,ImgInfo.nOverlayOrigin2,1);

	p = Hs_FindFirstEle(pPixEle,TAG_MAGNIFY_TO_NUMBER_OF_COLUMNS,true);
	Hs_GetLongValue(p,ImgInfo.nMagnifyToNumberOfColumns,0);

	p = Hs_FindFirstEle(pPixEle,TAG_LUT_DESCRIPTOR,true);
	Hs_GetLongValue(p,ImgInfo.nLutDescriptor1,0);

	p = Hs_FindFirstEle(pPixEle,TAG_LUT_DESCRIPTOR,true);
	Hs_GetLongValue(p,ImgInfo.nLutDescriptor2,1);

	p = Hs_FindFirstEle(pPixEle,TAG_MODALITY,true);
	Hs_GetStringValue(p,ImgInfo.sModality,0);

	p = Hs_FindFirstEle(pPixEle,TAG_IMAGE_POSITION_PATIENT,true);
	Hs_GetDoubleValue(p,ImgInfo.fImagePosition[0],0);

	p = Hs_FindFirstEle(pPixEle,TAG_IMAGE_POSITION_PATIENT,true);
	Hs_GetDoubleValue(p,ImgInfo.fImagePosition[1],1);

	p = Hs_FindFirstEle(pPixEle,TAG_IMAGE_POSITION_PATIENT,true);
	if(Hs_GetDoubleValue(p,ImgInfo.fImagePosition[2],2) != Ret_Success)
	{
		p = Hs_FindFirstEle(pPixEle,TAG_SLICE_LOCATION,true);
		Hs_GetDoubleValue(p,ImgInfo.fImagePosition[2],0);
	}

	p = Hs_FindFirstEle(pPixEle,TAG_ULTRASOUND_COLOR_DATA_PRESENT,true);
	Hs_GetLongValue(p,ImgInfo.nUltrasound_Color_Data_Present,0);


	//����WL_Lut�ĳ���

	if ( ImgInfo.nPixelRepresentation == 1 ) //��λ����
		ImgInfo.nWcLutLen = 1<<ImgInfo.nBitsAllocated;
	else
		ImgInfo.nWcLutLen = 1<<(ImgInfo.iHighBit+1);

	if(ImgInfo.nWcLutLen>256)
		ImgInfo.fWinLevelStep = min(ImgInfo.nWcLutLen/1000,10);


	if (ImgInfo.nPixelRepresentation==0)
	{
		ImgInfo.iLutStart = 0;
		ImgInfo.iLutEnd = ImgInfo.nWcLutLen;
	}
	else //��λ����(����ֵ���и���)
	{
		ImgInfo.iLutStart = -(ImgInfo.nWcLutLen/2);
		ImgInfo.iLutEnd = ImgInfo.nWcLutLen/2;
	}

	pHsElement pOverlayData = Hs_FindFirstEle(pPixEle,TAG_OVERLAY_DATA,true);
	if(ImgInfo.iOverlayBitPosition>ImgInfo.iHighBit)//Ƕ��ʽImgInfo.iOverlayBitPosition==16 || ImgInfo.iOverlayBitPosition==8
	{
		ImgInfo.nOverLayType = OverLay_Pixel;
	}
	else if (pOverlayData!=NULL && ImgInfo.nOverlayCols>0 && ImgInfo.nOverlayRows>0)//����ʽ
	{
		//if (ImgInfo.nOverlayCols*ImgInfo.nOverlayRows == pOverlayData->nLen*8)
			ImgInfo.nOverLayType = OverLay_Bits;
		//else if(ImgInfo.nOverlayCols*ImgInfo.nOverlayRows*ImgInfo.nOverlayBitsAllocated == pOverlayData->nLen*8)
		//	ImgInfo.nOverLayType = OverLay_Byte;
		//else
		//	ImgInfo.nOverLayType = OverLay_None;
	}
	else//ûOverLay
	{
		ImgInfo.nOverLayType = OverLay_None;
	}

	//if (ImgInfo.iOverlayBitPosition!=16 && ImgInfo.iOverlayBitPosition!=8 && ImgInfo.iOverlayBitPosition!=0 )
	//{
	//	AtlTrace("\r\n�����ϱ�׼��OverLay");
	//	return Ret_UnKnownCase;
	//}
	if (ImgInfo.nPixelRepresentation == 1 && ImgInfo.nOverLayType == 2)
	{
		qDebug("\r\nOverLay:���и�λ����,����Ƕ��ʽOverlay");
	}

	//��ȡ��λ�߻���Ԫ��
	GetImageLocPara(ImgInfo);

	pHsElement pFunGpSeqEle = Hs_FindFirstEle(NULL,0x52009230,true);
	while(pFunGpSeqEle)
	{
		//��ȡpixspacing
		pHsElement pItemEle = GetItemFromPerFramFunGpSeq(pFunGpSeqEle,iFrame);
		if(pItemEle == NULL)
			break;

		//Pixel Spacing /Slice thick
		pHsElement pPixMeasSeqEle = Hs_GetChildEle(pItemEle,0x00289110,-1);
		if(pPixMeasSeqEle)
		{
			pHsElement tItemEle0 = Hs_GetChildEle(pPixMeasSeqEle,TAG_ITEM,-1);
			if(tItemEle0)
			{
				pHsElement pPixSpacingEle = Hs_GetChildEle(tItemEle0,TAG_PIXEL_SPACING,true);

				if (pPixSpacingEle)
				{
					Hs_GetDoubleValue(pPixSpacingEle,ImgInfo.fPixelSpaceY,0);
					Hs_GetDoubleValue(pPixSpacingEle,ImgInfo.fPixelSpaceX,1);
				}
			}
		}
		//Image Position
		pHsElement pPlanePosSeqEle = Hs_GetChildEle(pItemEle,0x00209113,-1);
		if (pPlanePosSeqEle)
		{
			pHsElement tItemEle0 = Hs_GetChildEle(pPlanePosSeqEle,TAG_ITEM,-1);
			if (tItemEle0)
			{
				pHsElement pImgPosition = Hs_GetChildEle(tItemEle0,TAG_IMAGE_POSITION_PATIENT,true);
				if (pImgPosition)
				{
					Hs_GetDoubleValue(pImgPosition,ImgInfo.fImagePosition[0],0);
					Hs_GetDoubleValue(pImgPosition,ImgInfo.fImagePosition[1],1);
					Hs_GetDoubleValue(pImgPosition,ImgInfo.fImagePosition[2],2);
				}

			}
		}
		//
		//Rescale Para
		if (m_sModality.compare("CT") == 0)
		{
			pHsElement pPixValueTrsSeqEle = Hs_GetChildEle(pItemEle,0x00289145,-1);
			if (pPixValueTrsSeqEle)
			{
				pHsElement tItemEle0 = Hs_GetChildEle(pPixValueTrsSeqEle,TAG_ITEM,-1);
				if (tItemEle0)
				{
					pHsElement pRescale = Hs_GetChildEle(tItemEle0,TAG_RESCALE_INTERCEPT,true);
					if (pRescale)
						Hs_GetDoubleValue(pRescale,ImgInfo.fRescaleIntercept,0);

					pRescale = Hs_GetChildEle(tItemEle0,TAG_RESCALE_SLOPE,true);
					if (pRescale)
						Hs_GetDoubleValue(pRescale,ImgInfo.fRescaleSlope,0);
				}
			}
		}

		//WL
		if (m_sModality.compare("CT") == 0)
		{
			pHsElement pWLEle = Hs_GetChildEle(pItemEle,0x00289132,-1);
			if (pWLEle)
			{
				pHsElement tItemEle0 = Hs_GetChildEle(pWLEle,TAG_ITEM,-1);
				if (tItemEle0)
				{
					pHsElement pWL = Hs_GetChildEle(tItemEle0,TAG_WINDOW_CENTER,true);
					if (pWL)
						Hs_GetDoubleValue(pWL,ImgInfo.fWinCenter,0);

					pWL = Hs_GetChildEle(tItemEle0,TAG_WINDOW_WIDTH,true);
					if (pWL)
						Hs_GetDoubleValue(pWL,ImgInfo.fWinWidth,0);
				}
			}
		}

		//��ȡһ�¶�λ����Ŀ
		if (ImgInfo.ImgLocPara.bValide == false)
		{
			GetImageLocParaMulityFrame(iFrame,ImgInfo);
		}

		break;
	}


	//һЩ����Ĵ���
	if (1)
	{
		//301ҽԺ���ܵ�RFͼ�񣬵�Tag:0x10411002=1ʱ����Ҫ���ٽ���һ�θ�Ƭ
		if (m_sModality.compare("RF") == 0)
		{
			QString sv = "";
			Hs_GetStringValueA(0x00080070,sv);
			QString sManufacture = sv;
			sManufacture.toLower();
			int iPos = sManufacture.indexOf("shimadzu");//�����豸�ı�־
			if (iPos >= 0)
			{
				p = Hs_FindFirstEle(NULL,0x10411002,true);
				if(p)
				{
					long lv = 0;
					Hs_GetLongValue(p,lv,0);
					if(lv == 1)
					{
						ImgInfo.bInverse = !ImgInfo.bInverse;
						ImgInfo.bGrayImg = true;
					}
				}
			}

		}
	}

	//��ɢ����
	if (m_sModality.compare("MR") == 0)
	{
		pHsElement pDiffisionSq = Hs_FindFirstEle(NULL,0x00189117,true);
		if (pDiffisionSq)
		{
			pHsElement tItemEle0 =Hs_GetChildEle(pDiffisionSq,TAG_ITEM,-1);
			if (tItemEle0)
			{
				pHsElement pDiffusionBvalueEle = Hs_GetChildEle(tItemEle0,TAG_DIFFUSION_B_VALUE,true);
				if (pDiffusionBvalueEle)
					Hs_GetDoubleValue(pDiffusionBvalueEle,ImgInfo.fDifusionBvalue,0);
			}
		}
		else
		{
			pHsElement pFunGpSeqEle = Hs_FindFirstEle(NULL,0x52009230,true);
			if(pFunGpSeqEle)
			{
				pHsElement pItemEle = GetItemFromPerFramFunGpSeq(pFunGpSeqEle,iFrame);
				if (pItemEle)
				{
					pHsElement pDiffisionSq = Hs_GetChildEle(pItemEle,0x00189117,-1);
					if(pDiffisionSq)
					{
						pHsElement tItemEle0 = Hs_GetChildEle(pDiffisionSq,TAG_ITEM,-1);
						if(tItemEle0)
						{
							pHsElement pDiffusionBvalueEle = Hs_GetChildEle(tItemEle0,TAG_DIFFUSION_B_VALUE,true);
							if (pDiffusionBvalueEle)
								Hs_GetDoubleValue(pDiffusionBvalueEle,ImgInfo.fDifusionBvalue,0);
							if (pDiffusionBvalueEle == NULL)
								ImgInfo.fDifusionBvalue = -100;
						}
					}
				}
			}
			else
			{
				pHsElement pDiffisionEle = Hs_FindFirstEle(NULL,TAG_DIFFUSION_B_VALUE,true);
				if (pDiffisionEle)
					Hs_GetDoubleValue(pDiffisionEle,ImgInfo.fDifusionBvalue,0);
				else
					ImgInfo.fDifusionBvalue = -10240;
			}
		}
	}

	return 0;
}

//E:\1ͼƬ\�ػʵ���ҽԺ\�ػʵ���ҽԺ-DR\1.2.826.0.1.3680043.1.1.10606269.20080227.151635.0.0.1173.dcm
//���ͼ��ƭ�˵�,˵û��OverLay��ʵ����͵͵�ķ���OverLay���������ؿ�λ,�������
int CHsBaseFile::Hs_GetImage(pHsElement pPixEle,CHsBaseImg &Img, int iFrame)
{
	//CTstRunTime tsm("Hs_GetImage");
//CLog a;
//a.Log("\r\n��ʼ��ȡ����");
	//���������ж�
	if(pPixEle==NULL)
		return Ret_InvalidPara;

	if(pPixEle->nLen==0 )
		return Ret_NoValue;

	int nRet = 0;

	bool bCloseFile = false;//�Ƿ���Ҫ�ر��ļ�?----���ԭ���ļ��ǹرյ�,�������ر�,���򱣳ִ�״̬

	//��ȡ������Ϣ
	if(Img.m_ImgInfo.bValid == false)
	{
		nRet = Hs_GetImageInfo(pPixEle,Img.m_ImgInfo,iFrame);
		if(nRet) 
			return nRet;
	}

	////������һ��ͼ�����Ȼ��PixSpacing�ĵط���
	//Img.m_AnnoManager.SetPixSpacing(Img.m_ImgInfo.fPixelSpaceX,Img.m_ImgInfo.fPixelSpaceY);

	//����̫����.�Ҹ���������
	unsigned long nRow = Img.m_ImgInfo.nOriRows;
	unsigned long nCol = Img.m_ImgInfo.nOriCols;

	if(nRow==0 || nCol==0)
		return Ret_NoValue;

	//��ʼ��һ��,״̬����
	if(Img.m_ImgState.bImgStateFilled==false)
		Img.Hs_InitImgState();

	Img.m_ImgState.nCurOriPixCol = nCol;
	Img.m_ImgState.nCurOriPixRow = nRow;
	Img.m_ImgState.bSubstracted = false;

	long nFrame = Img.m_ImgInfo.nFrame;
	int iCompress = Img.m_ImgInfo.iCompress;
	int nSample = Img.m_ImgInfo.nSamplePerPixel;

	if (iFrame>=Img.m_ImgInfo.nFrame)
		return Ret_OutOfValueCount;

	//һ������Ҫ���ֽ�?
	unsigned long nBytePerPix = Img.m_ImgInfo.nBitsPerPixel/8;
	//����ÿ֡�����ֽ�
	unsigned long nSizePerFrame = nBytePerPix * nRow * nCol;
	//��һ֡����ʼλ��
	unsigned long nFrameStartPos = pPixEle->nTagPos + pPixEle->nOffset +  iFrame*nSizePerFrame;
	//ÿ�м�����Ч�ֽ�,4�ı������µĲ����ֽڲ���
	unsigned long nBytePerRow = nCol*nBytePerPix;

	//�����ļ�ĩβ,--dcm�ļ���ȱһ��������.û��ϵ,�ж���������ʾ����
	//if(nSizePerFrame + nFrameStartPos>m_BaseFileInfo.nFullSize)
	//	return Ret_GetImgError;


	LDicomDS *pDs = NULL;//ds(_T("c:\\"));
	LBitmap lbm;

	if (Img.m_ImgInfo.iCompress==0)
	{
		//���ļ�
		if(m_fp == NULL)
		{
			QByteArray ba = m_BaseFileInfo.sFileName.toLatin1();
			errno_t err = fopen_s( &m_fp, ba.data(), "rb" );
			if( err !=0 )
				return Ret_LoadFileError;

			bCloseFile = true;
		}
	}
	else
	{
        pDs = new LDicomDS("c:\\");
		//LeadTools���ظ��ļ�
		char cfile[512];
		QByteArray ba = m_BaseFileInfo.sFileName.toLatin1();
		strcpy(cfile,ba.data());
		if(pDs->LoadDS(cfile, 0)!=DICOM_SUCCESS)
		{
			delete pDs;
			pDs = NULL;
			return Ret_LoadFileError;
		}
		
		pDICOMELEMENT pLtPix = GetLtEleByMyEle(pPixEle,pDs);

		L_UINT16 r = pDs->GetImage(pLtPix,lbm.GetHandle(),sizeof(BITMAPHANDLE),iFrame,0,0,0,0,0);
		if(r)
		{
			delete pDs;
			pDs = NULL;
			return Ret_LedtolsGetImgError;
		}

		delete pDs;
		pDs = NULL;
	}


	//��ȡһ֡������
	//�����ڴ�׼�����ܸ�֡����
	if (Img.m_pOriData)
	{
		ArrayFree((void**)Img.m_pOriData);
		Img.m_pOriData = NULL;
	}

	unsigned long nNewRow = 0;
	unsigned long nNewCol = 0;
	Img.m_pOriData = (BYTE**)ArrayNew(nRow,		nCol,		nBytePerPix,&nNewRow, &nNewCol);

	if(Img.m_pOriData==NULL)
		return Ret_AllocateMemFailed;

	unsigned long nSamplePerRow = nCol*nSample;//ÿ�м���Sample
	unsigned long nBt = Img.m_ImgInfo.nBitsAllocated/8;//ÿ��Sample�м����ֽ�?
	int nStep = int(nBt/2);//BigEnian�����,Sample�ڲ��ֽڶԻ�ʱ,Ҫ��������

	bool bGetMaxMinValue = true;//��Ҫ�õ����ֵ��Сֵ��?
	//if (Img.m_ImgInfo.nSmallestPixelValue == 0 && Img.m_ImgInfo.nLargestPixelValue == 0)
	//	bGetMaxMinValue = true;

	long nMax = -2147483640;
	long nMin = 2147483640;

	if (iCompress == 0)//��ѹ���Ļ�
	{
		fseek(m_fp,nFrameStartPos, SEEK_SET);//���ļ���������֡��ʼ��
		size_t nCount = fread(Img.m_pOriData[0], nRow*nCol*nSample*nBt, 1,m_fp);//�����ڴ�m_pOriData[0]һ���Ӷ�ȡ��֡��������
		if(nCount != 1)//ʧ�ܴ���,��Ҫ������,�������Կ�����ȱ�Ļ�ͼ��.
		{
			ArrayFree((void**)Img.m_pOriData);
			return Ret_GetImgError;
		}

		//R1 R2 R3 R4...Rx. G1 G2 G3 G4...Gx. B1 B2 B3 B4...Bx.�ڷ���ʽ.�����RGB RGB....
		if (Img.m_ImgInfo.iPlanarConfig==1 && Img.m_ImgInfo.nBitsPerPixel==24 && int(Img.m_ImgInfo.sPhotometric.indexOf("RGB"))>=0)
		{
			unsigned long sz = nCol*nRow*3;//�����ܹ������ֽ�

			BYTE * pByte = new BYTE[sz];//������ʱ�ڴ�
			memcpy(pByte,Img.m_pOriData[0],sz);//�����ؿ�����ʱ�ڴ�

			unsigned long nSec = sz/3;
			BYTE *pImgData = Img.m_pOriData[0];

			unsigned long nSec2 = 2*nSec;

			//����ʱ�ڴ�����������
			for (unsigned long i=0;i<nSec;i++)
			{
				pImgData[3*i] = pByte[i];
				pImgData[3*i+1] = pByte[nSec+i];
				pImgData[3*i+2] = pByte[nSec2+i];
			}

			delete []pByte;
		}

		int nAppend = nNewCol - nCol;
		if (nAppend)//����Ϊ4�ֽڶ��룬���������ֽڵ����
		{
			unsigned long nBytePerRow4 = nNewCol * nBytePerPix;//��4֮���ÿ���ֽ���

			BYTE** pTmpData = (BYTE**)ArrayNew(nNewRow,	nNewCol,nBytePerPix);

			for (int r=0;r<nNewRow;r++)//nNewRow��nRow��Զ��һ�µ�
			{
				BYTE* p = Img.m_pOriData[0] + r * nBytePerRow;//ԭʼ��������һ�е���ʼλ��
				BYTE* pTmp = pTmpData[0] + r * nBytePerRow4;

				memcpy(pTmp,p,nBytePerRow);//nBytePerRow4
			}
			ArrayFree((void**)Img.m_pOriData);
			Img.m_pOriData = pTmpData;
		}
	}
	
	//��ʼ��:һ�ζ�һ��,ֱ�ӷŵ�Img��,�����ٱ����ڴ�
	for (int r=0;r<nRow;r++)
	{
		if(iCompress==0)
		{
			//����ע�͵��Ķ�ȡ��ʽ���ᵼ��nRow�δ��̶�ȡ������Զ��ͼ��Ļ������ᵼ��nRow�ε����紫�䣬��������һ���Ӷ�ȡ�������ص��ڴ棬�����ڴ��ﵹ�ڵĺ�
			//fseek(m_fp,nFrameStartPos + r*nBytePerRow, SEEK_SET);//���ļ���������֡��ʼ��
			//size_t nCount = fread(Img.m_pOriData[r], nBytePerRow, 1,m_fp);//�����ڴ�m_pOriData[r]
			//if(nCount != 1)//ʧ�ܴ���,��Ҫ������,�������Կ�����ȱ�Ļ�ͼ��.
			//{
			//	nRet = Ret_GetImgError;
			//	break;
			//}

			// ��Ϊ��˻�Ҫ��Sample�ڲ����ֽ�˳��ߵ�����.��ʵӦ��˵CPU���ļ���С�˲�һ�¾͵û�.
			if (pPixEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia && Img.m_ImgInfo.nBitsAllocated>8)
			{
				BYTE *pSamleData0 = Img.m_pOriData[r];

				for (unsigned long i=0;i<nSamplePerRow;i++)
				{
					BYTE *pSamleDatai = pSamleData0 + i*nBt;

					for (int b=0;b<nStep;b++)//12->21
					{
						BYTE t = pSamleDatai[b];
						pSamleDatai[b] = pSamleDatai[nBt-b-1];
						pSamleDatai[nBt-b-1] = t;
					}
				}
			}

			//�޸�λ����ʱ,�Ҷ�ͼ��Ҫ�����ֵ��Сֵ ������Ƕ��ʽ��OverLayҪ�۵�,����Ҫ����ÿ������
			if (Img.m_ImgInfo.nSamplePerPixel==1 && Img.m_ImgInfo.nPixelRepresentation==0)
			{
				SeparatePixdataAndOverlayByRow(Img,r,nCol,nMin,nMax);	
			}
			else
			{
				if (nBytePerPix == 1)
				{
					if (Img.m_ImgInfo.nPixelRepresentation==0)
					{
						unsigned char *t = (unsigned char*)Img.m_pOriData[r];
						for ( int i=0; i<nCol; i++)
						{
							if(t[i] > nMax)
								nMax = t[i];

							if(t[i] < nMin)
								nMin = t[i];
						}
					}
					else
					{
						char *t = (char*)(Img.m_pOriData[r]);
						for ( int i=0; i<nCol; i++)
						{
							if(t[i] > nMax)
								nMax = t[i];

							if(t[i] < nMin)
								nMin = t[i];
						}
					}
				}
				else if (nBytePerPix == 2)
				{
					if (Img.m_ImgInfo.nPixelRepresentation==0)
					{
						unsigned short *t = (unsigned short*)Img.m_pOriData[r];
						for ( int i=0; i<nCol; i++)
						{
							if(t[i] > nMax)
								nMax = t[i];

							if(t[i] < nMin)
								nMin = t[i];
						}
					}
					else
					{
						short *t = (short*)Img.m_pOriData[r];
						for ( int i=0; i<nCol; i++)
						{
							if(t[i] > nMax)
								nMax = t[i];

							if(t[i] < nMin)
								nMin = t[i];
						}
					}

				}
			}
		}
		else//Leadtools��æ��ѹ�������
		{
			if(nBytePerPix==1)
			{
				unsigned short nBitOffset = 8 - Img.m_ImgInfo.nBitStored;
				for ( int i=0; i<nCol; i++)
				{
					lbm.GetPixelData(&(Img.m_pOriData[r][i]) , r, i,  nBytePerPix);//��LeadTools���õ���ѹ�������

					//1.�����ڴ��С������
					
					//2.����Overlay�����ء������ֵ��Сֵ
					if (Img.m_ImgInfo.bGrayImg && Img.m_ImgInfo.nPixelRepresentation==0)
					{
						
						if (Img.m_pOriData[r][i] >> Img.m_ImgInfo.nBitStored )
						{
							if(Img.m_pOriOverlayData==NULL)
							{
								Img.m_pOriOverlayData = (BYTE**)ArrayNew(Img.m_ImgInfo.nRows,Img.m_ImgInfo.nCols,1);
								Img.m_ImgInfo.nOverLayType = OverLay_Pixel;
								Img.m_ImgInfo.nOverlayRows = Img.m_ImgInfo.nRows;
								Img.m_ImgInfo.nOverlayCols = Img.m_ImgInfo.nCols;
							}

							Img.m_pOriOverlayData[r][i] = OverlayValue;
						}


						//�Ҳ�����OverLay�ڼ�λ,ȫ������:���Ƽ���ȥ,���Ƴ����,��ֻʣ������.��һ����ȷ�������ڿɿط�Χ��;�е�Dcm�ļ���ƭ�˵�.û˵��OverLay,ȴ͵͵����OverLay����.��������ֵ����nBitStoredָ���ķ�Χ
						Img.m_pOriData[r][i] = Img.m_pOriData[r][i] << nBitOffset;
						Img.m_pOriData[r][i] = Img.m_pOriData[r][i] >> nBitOffset;

						if(Img.m_pOriData[r][i] > nMax)
							nMax = Img.m_pOriData[r][i];

						if(Img.m_pOriData[r][i] < nMin)
							nMin = Img.m_pOriData[r][i];
					}

				}
			}
			else if (nBytePerPix==2)
			{
				unsigned short nBitOffset = 16 - Img.m_ImgInfo.nBitStored;

				unsigned short **pOutData = (unsigned short **)(Img.m_pOriData);

				for ( int i=0; i<nCol; i++)
				{
					lbm.GetPixelData(&pOutData[r][i] , r, i,  nBytePerPix);

					//���ڴ�С������
					if (pPixEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia)
					{
						BYTE *pSamleData0 = (BYTE*)pOutData[r][i];

						BYTE t = pSamleData0[0];
						pSamleData0[0] = pSamleData0[1];
						pSamleData0[1] = t;

					}

					if (Img.m_ImgInfo.bGrayImg && Img.m_ImgInfo.nPixelRepresentation==0)
					{
						//����OverLay�����ء������ֵ��Сֵ					
						if (pOutData[r][i] >> Img.m_ImgInfo.nBitStored )//�Ҳ�������û��Overlay,Ҳ���ܷŵ��ڼ�λ.������ȥ���ز���ʣ�µĲ���0���Ǿ���OverLay
						{
							if(Img.m_pOriOverlayData==NULL)
							{
								Img.m_pOriOverlayData = (BYTE**)ArrayNew(Img.m_ImgInfo.nRows,Img.m_ImgInfo.nCols,1);
								Img.m_ImgInfo.nOverLayType = OverLay_Pixel;
								Img.m_ImgInfo.nOverlayRows = Img.m_ImgInfo.nRows;
								Img.m_ImgInfo.nOverlayCols = Img.m_ImgInfo.nCols;
							}

							Img.m_pOriOverlayData[r][i] = OverlayValue;
						}

						//�Ҳ�����OverLay�ڼ�λ,ȫ������,���Ƽ���ȥ,���Ƴ����,��ֻʣ������.��һ����ȷ�������ڿɿط�Χ��,�е�Dcm�ļ���ƭ�˵�.û˵��OverLay,ȴ͵͵����OverLay����.��������ֵ����nBitStoredָ���ķ�Χ
						pOutData[r][i] = pOutData[r][i] << nBitOffset;
						pOutData[r][i] = pOutData[r][i] >> nBitOffset;

						if(pOutData[r][i] > nMax)
							nMax = pOutData[r][i];

						if(pOutData[r][i] < nMin)
							nMin = pOutData[r][i];
					}
				}
			}
			else if (nBytePerPix==3)
			{
				MYDATA24 **pOutData = (MYDATA24 **)(Img.m_pOriData);

				for ( int i=0; i<nCol; i++)
				{
					lbm.GetPixelData(&pOutData[r][i] , r, i,  nBytePerPix);
					BYTE t = pOutData[r][i].pData[0];
					pOutData[r][i].pData[0] = pOutData[r][i].pData[2];
					pOutData[r][i].pData[2] = t;
				}
			}
		}
	}



	if (Img.m_ImgInfo.nPixelRepresentation==1)//�����з��ŵ�ͼ�����������ֽڵ�ͼ�������ֵ�п��ܳ�����Χ
	{//E:\1ͼƬ\��Ƭ��������ͼ��\vlut_p06.dcm
		if (nBytePerPix==2)//���ֽڵ�//��ֻ�����ֽڵġ�
		{
			short nCorrectMin = -(2<<(Img.m_ImgInfo.iHighBit-1));//������Сֵ
			short nCorrectMax = (2<<(Img.m_ImgInfo.iHighBit-1)) - 1;//�������ֵ

			if (nMax > nCorrectMax || nMin < nCorrectMin)//Υ����
			{
				short nOffset = nCorrectMax - nCorrectMin;
				CorrectPixelValue((short**)Img.m_pOriData, Img.m_ImgInfo.nRows,Img.m_ImgInfo.nCols,nCorrectMin,nCorrectMax,nOffset);

				nMin = nCorrectMin;
				nMax = nCorrectMax;
			}			
		}
	}


	if(bGetMaxMinValue && nMax!=-2147483640 && nMin!=2147483640)
	{
		Img.m_ImgInfo.nLargestPixelValue = nMax;
		Img.m_ImgInfo.nSmallestPixelValue = nMin;

		//if (Img.m_ImgInfo.fWinWidth==0.00)
		//{
		//	Img.m_ImgInfo.fWinCenter = (nMin+nMax)*1.00/2;
		//	Img.m_ImgInfo.fWinWidth = nMax - nMin;
		//}

		if (Img.m_ImgInfo.fWinWidth==0.00)
		{
			//���ͼ����б�ʽؾ࣬��ô����ֵ����б�ʽؾ�ӹ�����ʹ�ã�����Ҫ��������ֵ�㴰��λ����ؿ���б�ʽؾ�
			int tMax = nMax;
			int tMin = nMin;
			if (Img.m_ImgInfo.fRescaleSlope!=0.00 && Img.m_ImgInfo.fRescaleIntercept!=0.00)
			{
				tMax = tMax*Img.m_ImgInfo.fRescaleSlope + Img.m_ImgInfo.fRescaleIntercept;
				tMin = tMin*Img.m_ImgInfo.fRescaleSlope + Img.m_ImgInfo.fRescaleIntercept;
			}

			Img.m_ImgInfo.fWinCenter = (tMin+tMax)*1.00/2;
			Img.m_ImgInfo.fWinWidth = tMax - tMin;
		}

	}



	Img.m_ImgInfo.iFrame = iFrame;

	//����ж���OverLay,�˴�������
	if (Img.m_ImgInfo.nOverLayType==OverLay_Bits)
	{
		pHsElement pOverlayData = Hs_FindFirstEle(pPixEle,TAG_OVERLAY_DATA,true);
		if(m_fp == NULL)
		{
			QByteArray sFileName = m_BaseFileInfo.sFileName.toLatin1();
			errno_t err = fopen_s(&m_fp, sFileName.data(), "rb");
			if( err !=0 )
				return Ret_LoadFileError;

			bCloseFile = true;
		}

		fseek(m_fp,pOverlayData->nTagPos+pOverlayData->nOffset, SEEK_SET);//���ļ���������֡��ʼ��

		unsigned long nNeedLen = pOverlayData->nLen;//Img.m_ImgInfo.nOverlayCols * Img.m_ImgInfo.nOverlayRows/8 +1; 
		BYTE *pByte = new BYTE[nNeedLen];//pOverlayData->nLen
		size_t nCount = fread(pByte,nNeedLen , 1,m_fp);//pOverlayData->nLen
		if(nCount!=1)
		{
			delete []pByte;
			Img.m_pOriOverlayData = NULL;
		}
		else
		{
			//���ɹ���,���Ǵ����OW�͵ģ����õߵ�һ��
			if (pOverlayData->nVR == VR_OW && pPixEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia && nNeedLen%2==0)
			{
				int nTimes = nNeedLen/2;
				for (unsigned long i=0;i<nTimes;i++)
				{
					qSwap(pByte[2*i],pByte[2*i+1]);
				}
			}

			unsigned long nNewRow = 0;
			unsigned long nNewCol = 0;

			if(Img.m_pOriOverlayData==NULL)//�ж���ʽ��OverLay
				Img.m_pOriOverlayData = (BYTE**)ArrayNew(Img.m_ImgInfo.nOverlayRows,Img.m_ImgInfo.nOverlayCols,1,&nNewRow,&nNewCol);


			//��Ϊ4����������˼�������
			unsigned long nAppend = nNewCol - Img.m_ImgInfo.nOverlayCols;
			unsigned long nAllLen = nNewCol*nNewRow;

			BYTE b[8] = {1, 2, 4, 8, 16, 32, 64, 128};
			unsigned long iOverlay = 0;


			for (long iByte=0;iByte<pOverlayData->nLen;iByte++)//nNeedLen
			{
				for(int iBit=0;iBit<8;iBit++)
				{
					if (nAppend > 0)
					{
						int x = nNewCol - (iOverlay+1)%nNewCol;

						if (x<nAppend)
						{
							iOverlay += nAppend;
						}
					}

					if (iOverlay>=nAllLen)
						break;

					if(pByte[iByte] & b[iBit])
					{
						Img.m_pOriOverlayData[0][iOverlay] = OverlayValue;	
					}
					else
					{
						Img.m_pOriOverlayData[0][iOverlay] = 0;
					}

					iOverlay++;	
				}

				if (iOverlay>=nAllLen)
					break;
			}

			delete []pByte;
		}
	}


	if (Img.m_pDisplayData)
	{
		ArrayFree((void**)Img.m_pDisplayData,0);
		Img.m_pDisplayData = NULL;
	}

	if (Img.m_bMpr == false)
	{
		Img.m_pDisplayData = (BYTE**)ArrayCopy((void**)Img.m_pOriData,nRow,nCol,nBytePerPix);
		Img.m_ImgState.nDispalyCol = nCol;
		Img.m_ImgState.nDispalyRow = nRow;
	}
	
	if (Img.m_pDisplayOverlayData)
	{
		ArrayFree((void**)Img.m_pDisplayOverlayData);
		Img.m_pDisplayOverlayData = NULL;
	}

	if (Img.m_pOriOverlayData)
	{
		if (Img.m_ImgInfo.nOverlayRows==Img.m_ImgInfo.nRows && Img.m_ImgInfo.nOverlayCols == Img.m_ImgInfo.nCols)
		{
			Img.m_pDisplayOverlayData = (BYTE**)ArrayCopy((void**)Img.m_pOriOverlayData,nRow,nCol,1);
		}
		else
		{
			//sos �ڴ˴�Ҫ�ǵñ�֤OverLaydata����Ҫ����������һ��.---�е��ļ�����һ��.�����ٴ���
			//::MessageBox(0,"OverLay�������������в�ͬ","�����",0);
			ArrayFree((void **)Img.m_pOriOverlayData);
			Img.m_pOriOverlayData = NULL;
		}

	}

	Hs_GetLuts(pPixEle,Img);

	//���ĳЩͼ��û������wl����wlΪ0������£�����λ���Ե�����
	if(Img.m_ImgInfo.fWinWidth <= 0.000001 )//û�ҵ�WC�������,
	{
		if (Img.m_ImgInfo.nSmallestPixelValue!=0 || Img.m_ImgInfo.nLargestPixelValue!=0)
		{
			//Img.m_ImgInfo.fWinWidth =  Img.m_ImgInfo.nLargestPixelValue - Img.m_ImgInfo.nSmallestPixelValue;
			//Img.m_ImgInfo.fWinCenter = (Img.m_ImgInfo.nLargestPixelValue + Img.m_ImgInfo.nSmallestPixelValue)/2;

			int tMax = Img.m_ImgInfo.nLargestPixelValue;
			int tMin = Img.m_ImgInfo.nSmallestPixelValue;
			if (Img.m_ImgInfo.fRescaleSlope!=0.00 && Img.m_ImgInfo.fRescaleIntercept!=0.00)
			{
				tMax = tMax*Img.m_ImgInfo.fRescaleSlope + Img.m_ImgInfo.fRescaleIntercept;
				tMin = tMin*Img.m_ImgInfo.fRescaleSlope + Img.m_ImgInfo.fRescaleIntercept;
			}

			Img.m_ImgInfo.fWinCenter = (tMin+tMax)*1.00/2;
			Img.m_ImgInfo.fWinWidth = tMax - tMin;
		}
		else
		{
			//Img.m_ImgInfo.fWinWidth =  1 << Img.m_ImgInfo.nBitStored;
			//Img.m_ImgInfo.fWinCenter = 1 << (Img.m_ImgInfo.nBitStored-1);

			int tMax = 1 << Img.m_ImgInfo.nBitStored;
			int tMin = 0;
			if (Img.m_ImgInfo.fRescaleSlope!=0.00 && Img.m_ImgInfo.fRescaleIntercept!=0.00)
			{
				tMax = tMax*Img.m_ImgInfo.fRescaleSlope + Img.m_ImgInfo.fRescaleIntercept;
				tMin = tMin*Img.m_ImgInfo.fRescaleSlope + Img.m_ImgInfo.fRescaleIntercept;
			}

			Img.m_ImgInfo.fWinCenter = (tMin+tMax)*1.00/2;
			Img.m_ImgInfo.fWinWidth = tMax - tMin;
		}

	}


	if (lbm.IsAllocated())//LeadtoolsӦ���Լ����ͷŰ�.��ȷ�����Դ˴����ͷ�һ��
		lbm.Free();
	
	//ע����Ϣ
	//CAppConfig::GetInfoSet()

	if(bCloseFile==true)//���ԭ���ļ��ǹرյ�,��ر�.��������Ӧ�øı��ļ��Ĵ�״̬.
		Hs_CloseFile();

	//���˵�λ��
	if (Img.m_ImgInfo.ImgLocPara.bValide==true)
	{
		
		Img.Hs_GetPatientPos(Img.m_ImgInfo.ImgLocPara.fFirstColCosX,
							Img.m_ImgInfo.ImgLocPara.fFirstColCosY,
							Img.m_ImgInfo.ImgLocPara.fFirstColCosZ,
							Img.m_ImgState.sTopPatientPos,
							Img.m_ImgState.sBottomPatientPos);

		Img.Hs_GetPatientPos(Img.m_ImgInfo.ImgLocPara.fFirstRowCosX,
							Img.m_ImgInfo.ImgLocPara.fFirstRowCosY,
							Img.m_ImgInfo.ImgLocPara.fFirstRowCosZ,
							Img.m_ImgState.sLeftPatientPos,
							Img.m_ImgState.sRightPatientPos);
	}

	//a.Log("\r\n��ȡ�������");
	return nRet;
}
//���Modality Lut��Voi Lut���� module������(Lut��Wl��Ҫ�õ�)
int CHsBaseFile::Hs_GetLuts(pHsElement pPixelEle,CHsBaseImg &Img)
{//Ѱ������LutData
	
	int nRet = 0;

	//��һ����:VoiLut======================================================================
	pHsElement pVoiSeq = Hs_FindFirstEle(pPixelEle,TAG_VOI_LUT_SEQUENCE,true);

	if(pVoiSeq)
	{
		int i = 0;
		while(1)
		{
			pHsElement pItem = Hs_GetChildEle(pVoiSeq,0,i++);
			if(pItem==NULL)
				break;

			LutData *pNewLut = new LutData;
			pNewLut->bModality = false;
			pNewLut->bWc = false;
			pNewLut->iLutID = i;

			if (Hs_GetLutDataItem(pItem,*pNewLut) == false)
			{
				delete pNewLut;
				continue;
			}
			else
			{
				Img.m_pLutV.push_back(pNewLut);
			}	
		}

	}

	//VOI WinLevel:
	Hs_GetWcLutItem(NULL,Img.m_pLutV);
	/*
	pHsElement pWinCenter = Hs_FindFirstEle(0,TAG_WINDOW_CENTER,true);
	pHsElement pWinWidth = Hs_FindFirstEle(0,TAG_WINDOW_WIDTH,true);
	pHsElement pWinExplan = Hs_FindFirstEle(0,0x00281055,true);

	if (pWinCenter)
	{
		int i = -1;
		while(1)
		{
			i++;
			LutData *pNewWcLut = new LutData;
			pNewWcLut->bModality = false;
			pNewWcLut->bWc = true;
			nRet = Hs_GetDoubleValue(pWinCenter,pNewWcLut->nC,i);
			if(nRet)
			{
				delete pNewWcLut;
				break;
			}

			nRet = Hs_GetDoubleValue(pWinWidth,pNewWcLut->nW,i);
			if(nRet)
			{
				delete pNewWcLut;
				break;
			}

			nRet = Hs_GetStringValue(pWinExplan,pNewWcLut->sName,i);
			if(pNewWcLut->sName.empty())
			{
				char cName[100];
				sprintf(cName,"WL:%d",i+1);
				pNewWcLut->sName = cName;
			}
			else
			{
				char cName[200] = {'\0'};
				sprintf(cName,"WL:%s",pNewWcLut->sName.c_str());
				pNewWcLut->sName = cName;
			}

			Img.m_pLutV.push_back(pNewWcLut);

		}
	}
*/
	//�ڶ�����:Modality Lut================================================================
	pHsElement pModSeq = Hs_FindFirstEle(pPixelEle,TAG_MODALITY_LUT_SEQUENCE,true);

	if(pModSeq)
	{
		int i = 0;
		while(1)
		{
			pHsElement pItem = Hs_GetChildEle(pVoiSeq,0,i++);
			if(pItem==NULL)
				break;

			LutData *pNewLut = new LutData;
			pNewLut->bModality = true;
			pNewLut->bWc = false;
			pNewLut->iLutID = i;

			if (Hs_GetLutDataItem(pItem,*pNewLut) == false)
			{
				delete pNewLut;
				continue;
			}
			else
			{
				Img.m_pLutV.push_back(pNewLut);
			}
		}

	}

	//��������:PALETTE COLOR LUT================================================================
	int bPaletteClor = int(Img.m_ImgInfo.sPhotometric.indexOf("PALETTE COLOR"));//����compare.��Ϊ���пո�����
	if (bPaletteClor>=0)
	{
		unsigned long nTagDes = TAG_RED_PALETTE_COLOR_LOOKUP_TABLE_DESCRIPTOR;
		unsigned long nTagData = TAG_RED_PALETTE_COLOR_LOOKUP_TABLE_DATA;
		QString sName = "R";

		int nColorLutCount = 0;//��һ��,������lut����?
		for(int i=0;i<3;i++)
		{
			if(i==1)
			{
				nTagDes = TAG_GREEN_PALETTE_COLOR_LOOKUP_TABLE_DESCRIPTOR;
				nTagData = TAG_GREEN_PALETTE_COLOR_LOOKUP_TABLE_DATA;
				sName = "G";
			}
			if (i==2)
			{
				nTagDes = TAG_BLUE_PALETTE_COLOR_LOOKUP_TABLE_DESCRIPTOR;
				nTagData = TAG_BLUE_PALETTE_COLOR_LOOKUP_TABLE_DATA;
				sName = "B";
			}

			LutData *pNewLut = new LutData;
			pNewLut->bModality = false;
			pNewLut->bWc = false;
			pNewLut->sName = sName;


			// Des
			pHsElement pLutDes = Hs_FindFirstEle(pPixelEle,nTagDes,true);
			nRet = Hs_GetLongValue(pLutDes,pNewLut->nLutLen,0);//Lut�೤? �����ٸ�Data?
			if(nRet)
			{
				delete pNewLut;
				break;
			}

			nRet = Hs_GetLongValue(pLutDes,pNewLut->nMinValue,1);//��Сֵ
			if(nRet)
			{
				delete pNewLut;
				break;
			}

			nRet = Hs_GetLongValue(pLutDes,pNewLut->nBitsPerData,2);//Lut��ÿ��Data����ռ��λ
			if(nRet)
			{
				delete pNewLut;
				break;
			}

			// Data
			pHsElement pLutData = Hs_FindFirstEle(pPixelEle,nTagData,true);
			if (pNewLut->nLutLen==0)
				pNewLut->nLutLen= Hs_GetValueCount(pLutData,nRet);

			pNewLut->nBytePerData = pNewLut->nBitsPerData/8 + (pNewLut->nBitsPerData%8 > 0 ? 1:0);

			unsigned long nSize = pNewLut->nLutLen * pNewLut->nBytePerData ;//Ԥ��Ӧ�г���
			if(nSize==0)
			{
				delete pNewLut;
				break;
			}

			unsigned long nSizeGot = 0;
			BYTE *pBuf = Hs_GetByteValue(pLutData,nSizeGot,nRet);			

			if(nRet)
			{
				delete pNewLut;
				delete []pBuf;
				break;
			}

			if(nSizeGot!=nSize)//Ԥ�Ƴ�����ʵ�ʳ��Ȳ�����
			{
				delete pNewLut;
				delete []pBuf;
				break;
			}

			//��pBuf�����0-255
			long nMaxLutData = 1<<pNewLut->nBitsPerData;
			double f = 256.00/nMaxLutData;
			pNewLut->pLutData = new BYTE[pNewLut->nLutLen];

			if (pNewLut->nBytePerData==1)
			{
				BYTE *tData = (BYTE*)pBuf;
				for (int k=0;k<pNewLut->nLutLen;k++)
				{
					pNewLut->pLutData[k] = tData[k]*f;
					if(pNewLut->pLutData[k]>=255)
						pNewLut->pLutData[k] = 255;

					//AtlTrace("\r\n%d-%d",tData[k],pNewLut->pLutData[k]);
				}

			}
			if (pNewLut->nBytePerData==2)
			{
				unsigned short *tData = (unsigned short*)pBuf;
				for (int k=0;k<pNewLut->nLutLen;k++)
				{
					pNewLut->pLutData[k] = tData[k]*f;
					if(pNewLut->pLutData[k]>=255)
						pNewLut->pLutData[k] = 255;

					//BYTE *pp = (BYTE*)(&tData[k]);

					//if(k==236)
					//	AtlTrace("\r\n%d:%d-%d  (%d %d)",k,tData[k],pNewLut->pLutData[k],pp[0],pp[1]);
				}
			}

			delete []pBuf;

			pNewLut->nMaxValue = pNewLut->nMinValue + pNewLut->nLutLen - 1;

			Img.m_pLutV.push_back(pNewLut);
			nColorLutCount++;

		} 

		////������Lut�����ֵ��Сֵ.�ѳ�ʼWC�����.�������������:
		////1.����������LUT.
		////2.����Lut������DESCRIPTOR������ȫһ��,Ҳ���������Ҫ��һ��.������û����.
		//if (nColorLutCount==3)
		//{
		//	LutData *tLut = Img.m_pLutV[Img.m_pLutV.size()-1];
		//	Img.m_ImgInfo.fWinWidth = tLut->nMaxValue - tLut->nMinValue;
		//	Img.m_ImgInfo.fWinCenter = (tLut->nMaxValue + tLut->nMinValue)/2;
		//}

	}

	return Ret_Success;
}
/*
ͼ���Ϊѹ��ͼ���ԭʼͼ��:

һ��ԭʼͼ��
1��	ÿ֡ͼ���ǽ�������һ���,û�зֽ����.

2��	ÿ�����ط�Ϊn��Sample,ÿ��Sample������nBitAllocate(8�ı���)λ��
	ÿ��Sampleʵ��ռ��nBitsStoredλ,
	ÿ��Sample�����λ�ڵ�iHighBitλ(��Ȼ�д���nBitStored�����),
	����ÿ��������Ҫ��BitsPerPixel = Sample*nBitAllocateλ.

3��	�κ�ͼ������ҪWL_LUT��,���WL_LUT�������Լ�������.����ֵ(unsigned short��)��Ϊ����,ҪȥWL_LUT�ж�Ӧ��һ��0-255֮���ֵ��Ϊ������ʾֵ.
	��ôLut��ĳ�����pow(2,nBitsStored);
	������λiHighBit���滹��������ֵ,����OverLay����HighBit�ں�,��ô�ɿ��ǽ�������Ϊpow(2,nBitsStored+1)��WL_LUT��,ʹ�������и���.����Ҳ���.
	ֻ������ʱ���Lut���ϰ벿�ֲ���OVerlay��Ӱ��,�°벿��Ҫ����һ��OverLay����ʾ��ͻ������.(ͻ��:��Ϊһֱ��ʾ���ɫ,���Ǳ��ϰ벿�ָ�ֵ����һ��,���������)

	������λ�и�λ����,��Lut���Ⱦ�Ҫ��pow(2,nBitAllocate)��.

4��	ModalityLUT:��׼�й涨����Ӧ�õ�LUT.���Lut�������Ӧ��0-255������,��Ӧ����������Ȼ��0-pow(2,�����������).
	���ս���ModalityLut����Ӧ��������,��ȥ�����Լ���WL_LUT�ж�Ӧ��0-255������.
	VOI Lut�ǿ�ѡ��lut,���豸�Ƽ��ļ���LutЧ��. ��ʵҽԺʵ��Ӧ��ʱ,Modality Lut��ʱ��ҽԺҲϣ����ѡ.--Ҫ��������.

5��	��ʱ����Lut,����Ҫax+b : 0x00281053*x + 0x00281052

����ѹ��ͼ��,����취��ѹ��.

*/

pHsElement CHsBaseFile::DestroyEle(pHsElement pEle)
{
	if (pEle==NULL)
		return NULL;

	if (pEle->pPreEle)
		pEle->pPreEle->pNextEle = NULL;
	
	if (pEle->pParentEle)
	{
		int n = pEle->pParentEle->pChildEleV.size();
		for(int i=0;i<n;i++)
		{
			if(pEle->pParentEle->pChildEleV[i] == pEle)
				pEle->pParentEle->pChildEleV.erase(pEle->pParentEle->pChildEleV.begin()+i);
		}
	}

	delete pEle;

	return NULL;
}

pHsElement CHsBaseFile::BuildListRelation(pHsElement pEle)
{//�к��ӵ�,�뽻�����һ������,û���ӽ������Լ�,�Ա���Ϊ�´εݹ�� "��һ��"
	int n = pEle->pChildEleV.size();
	pHsElement pPre = pEle;

	for (int i=0;i<n;i++)
	{
		pEle->pChildEleV[i]->pPreEle = pPre;
		if(pPre)
			pPre->pNextEle = pEle->pChildEleV[i];

		pPre = BuildListRelation(pEle->pChildEleV[i]);
	}

	return pPre;
}

pDICOMELEMENT CHsBaseFile::GetLtEleByMyEle(pHsElement pMyEle, LDicomDS *pDS)
{
	if(pDS==NULL)
		return NULL;

	//pMyEle�ڱ��ļ����ǵڼ���ͬ����Ele
	int nPixID0 = 0;
	pHsElement tHsPixEle = Hs_FindFirstEle(0,pMyEle->nTag,false);
	while(tHsPixEle)
	{
		tHsPixEle = Hs_FindNextEle(tHsPixEle,FALSE);
		if(tHsPixEle)
			nPixID0++;
	}

	//��pDS��ҲӦ���ǵ�nPixID0��
	int nPixID1 = 0;
	pDICOMELEMENT pLtPix = pDS->FindFirstElement(NULL,pMyEle->nTag,FALSE);
	while(pLtPix)
	{
		if(nPixID1==nPixID0)
			return pLtPix;

		pLtPix = pDS->FindNextElement(pLtPix,FALSE);
		nPixID1++;
	}

	return NULL;
}

int CHsBaseFile::SeparatePixdataAndOverlayByRow(CHsBaseImg &Img,unsigned long iRow,unsigned long nCol,long &nMin,long &nMax)
{
	//�����ĵ�DSA���ү��,���Overlay����TMD���ڵڼ�λ?
	if(Img.m_ImgInfo.nBitsPerPixel==8)
	{
		unsigned short nBitOffset = 8 - Img.m_ImgInfo.nBitStored;

		for(unsigned long i=0;i<nCol;i++)
		{
			//�����Ƕ��ʽOverLayҪ�ٳ���,�ŵ������Ķ�ά������				

			//�Ҳ�����Overlay�ŵ��ڼ�λ.��ȥ���ز���ʣ�µĲ���0���Ǿ���OverLay
			if (Img.m_pOriData[iRow][i] >> Img.m_ImgInfo.nBitStored )
			{
				if(Img.m_pOriOverlayData==NULL)
				{
					Img.m_pOriOverlayData = (BYTE**)ArrayNew(Img.m_ImgInfo.nRows,Img.m_ImgInfo.nCols,1);
					Img.m_ImgInfo.nOverLayType = OverLay_Pixel;
					Img.m_ImgInfo.nOverlayRows = Img.m_ImgInfo.nRows;
					Img.m_ImgInfo.nOverlayCols = Img.m_ImgInfo.nCols;
				}

				Img.m_pOriOverlayData[iRow][i] = OverlayValue;
			}


			//�Ҳ�����OverLay�ڼ�λ,ȫ������:���Ƽ���ȥ,���Ƴ����,��ֻʣ������.��һ����ȷ�������ڿɿط�Χ��;�е�Dcm�ļ���ƭ�˵�.û˵��OverLay,ȴ͵͵����OverLay����.��������ֵ����nBitStoredָ���ķ�Χ
			Img.m_pOriData[iRow][i] = Img.m_pOriData[iRow][i] << nBitOffset;
			Img.m_pOriData[iRow][i] = Img.m_pOriData[iRow][i] >> nBitOffset;

			if(Img.m_pOriData[iRow][i] > nMax)
				nMax = Img.m_pOriData[iRow][i];

			if(Img.m_pOriData[iRow][i] < nMin)
				nMin = Img.m_pOriData[iRow][i];

		}
	}
	else if (Img.m_ImgInfo.nBitsPerPixel==16)
	{
		unsigned short nBitOffset = 16 - Img.m_ImgInfo.nBitStored;

		unsigned short* pRowDataAsShort = (unsigned short*)(Img.m_pOriData[iRow]);//����һ�п���ÿ���ֽڱ�ʾһ�����ֵ�short����
		for(unsigned long i=0;i<nCol;i++)
		{
			//�Ҳ�������û��Overlay,Ҳ���ܷŵ��ڼ�λ.������ȥ���ز���ʣ�µĲ���0���Ǿ���OverLay
			if (pRowDataAsShort[i] >> Img.m_ImgInfo.nBitStored )
			{
				if(Img.m_pOriOverlayData==NULL)
				{
					Img.m_pOriOverlayData = (BYTE**)ArrayNew(Img.m_ImgInfo.nRows,Img.m_ImgInfo.nCols,1);
					Img.m_ImgInfo.nOverLayType = OverLay_Pixel;
					Img.m_ImgInfo.nOverlayRows = Img.m_ImgInfo.nRows;
					Img.m_ImgInfo.nOverlayCols = Img.m_ImgInfo.nCols;
				}
				Img.m_pOriOverlayData[iRow][i] = OverlayValue;
			}

			//�Ҳ�����OverLay�ڼ�λ,ȫ������,���Ƽ���ȥ,���Ƴ����,��ֻʣ������.��һ����ȷ�������ڿɿط�Χ��,�е�Dcm�ļ���ƭ�˵�.û˵��OverLay,ȴ͵͵����OverLay����.��������ֵ����nBitStoredָ���ķ�Χ
			pRowDataAsShort[i] = pRowDataAsShort[i] << nBitOffset;
			pRowDataAsShort[i] = pRowDataAsShort[i] >> nBitOffset;

			if(pRowDataAsShort[i] > nMax)
				nMax = pRowDataAsShort[i];

			if(pRowDataAsShort[i] < nMin)
				nMin = pRowDataAsShort[i];

		}
	}
	else if (Img.m_ImgInfo.nBitsPerPixel==32)
	{
		unsigned short nBitOffset = 32 - Img.m_ImgInfo.nBitStored;

		unsigned long* pRowDataAsShort = (unsigned long*)(Img.m_pOriData[iRow]);//����һ�п���ÿ���ֽڱ�ʾһ�����ֵ�short����
		for(unsigned long i=0;i<nCol;i++)
		{
			//�Ҳ�������û��Overlay,Ҳ���ܷŵ��ڼ�λ.������ȥ���ز���ʣ�µĲ���0���Ǿ���OverLay
			if (pRowDataAsShort[i] >> Img.m_ImgInfo.nBitStored )
			{
				if(Img.m_pOriOverlayData==NULL)
				{
					Img.m_pOriOverlayData = (BYTE**)ArrayNew(Img.m_ImgInfo.nRows,Img.m_ImgInfo.nCols,1);
					Img.m_ImgInfo.nOverLayType = OverLay_Pixel;
					Img.m_ImgInfo.nOverlayRows = Img.m_ImgInfo.nRows;
					Img.m_ImgInfo.nOverlayCols = Img.m_ImgInfo.nCols;
				}
				Img.m_pOriOverlayData[iRow][i] = OverlayValue;
			}

			//�Ҳ�����OverLay�ڼ�λ,ȫ������,���Ƽ���ȥ,���Ƴ����,��ֻʣ������.��һ����ȷ�������ڿɿط�Χ��,�е�Dcm�ļ���ƭ�˵�.û˵��OverLay,ȴ͵͵����OverLay����.��������ֵ����nBitStoredָ���ķ�Χ
			pRowDataAsShort[i] = pRowDataAsShort[i] << nBitOffset;
			pRowDataAsShort[i] = pRowDataAsShort[i] >> nBitOffset;

			if(pRowDataAsShort[i] > nMax)
				nMax = pRowDataAsShort[i];

			if(pRowDataAsShort[i] < nMin)
				nMin = pRowDataAsShort[i];

		}
	}

	return Ret_Success;
}

bool CHsBaseFile::GetImageLocPara( ImageInfo &info )
{
	if(info.ImgLocPara.bValide == true)
		return true;

	memset(&info.ImgLocPara,0,sizeof(ImageLoc));

	//if (m_buf==NULL)
	//	return false;
	if(m_pMainEle==NULL)
		return false;

	//Row
	pHsElement pRowEle = Hs_FindFirstEle(NULL,TAG_ROWS,true);

	if(Hs_GetLongValue(pRowEle,info.ImgLocPara.nRow)!=Ret_Success) 
		return false;

	//Col
	pHsElement pColEle = Hs_FindFirstEle(NULL,TAG_COLUMNS,true);

	if(Hs_GetLongValue(pColEle,info.ImgLocPara.nCol)!=Ret_Success) 
		return false;

	//������Ч��
	if(info.ImgLocPara.nRow==0 || info.ImgLocPara.nCol==0)
		return false;


	//Pixel Spacing
	pHsElement pPixSpacingEle = Hs_FindFirstEle(NULL,TAG_PIXEL_SPACING,true);

	if(Hs_GetDoubleValue(pPixSpacingEle,info.ImgLocPara.fPixSpacingY,0)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pPixSpacingEle,info.ImgLocPara.fPixSpacingX,1)!=Ret_Success) 
		return false;

	// TAG_IMAGE_POSITION_PATIENT 0x00200032
	pHsElement pPosEle = Hs_FindFirstEle(NULL,TAG_IMAGE_POSITION_PATIENT,true);

	if(Hs_GetDoubleValue(pPosEle,info.ImgLocPara.fOriLeftTopPixX,0)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pPosEle,info.ImgLocPara.fOriLeftTopPixY,1)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pPosEle,info.ImgLocPara.fOriLeftTopPixZ,2)!=Ret_Success) 
		return false;

	info.ImgLocPara.fLeftTopPixX = info.ImgLocPara.fOriLeftTopPixX;
	info.ImgLocPara.fLeftTopPixY = info.ImgLocPara.fOriLeftTopPixY;
	info.ImgLocPara.fLeftTopPixZ = info.ImgLocPara.fOriLeftTopPixZ;

	// TAG_IMAGE_ORIENTATION_PATIENT 0x00200037UL
	pHsElement pOriEle = Hs_FindFirstEle(NULL,TAG_IMAGE_ORIENTATION_PATIENT,true);

	if(Hs_GetDoubleValue(pOriEle,info.ImgLocPara.fOriFirstRowCosX,0)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pOriEle,info.ImgLocPara.fOriFirstRowCosY,1)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pOriEle,info.ImgLocPara.fOriFirstRowCosZ,2)!=Ret_Success) 
		return false;

	if(Hs_GetDoubleValue(pOriEle,info.ImgLocPara.fOriFirstColCosX,3)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pOriEle,info.ImgLocPara.fOriFirstColCosY,4)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pOriEle,info.ImgLocPara.fOriFirstColCosZ,5)!=Ret_Success) 
		return false;


	info.ImgLocPara.fFirstRowCosX = info.ImgLocPara.fOriFirstRowCosX;
	info.ImgLocPara.fFirstRowCosY = info.ImgLocPara.fOriFirstRowCosY;
	info.ImgLocPara.fFirstRowCosZ = info.ImgLocPara.fOriFirstRowCosZ;

	info.ImgLocPara.fFirstColCosX = info.ImgLocPara.fOriFirstColCosX;
	info.ImgLocPara.fFirstColCosY = info.ImgLocPara.fOriFirstColCosY;
	info.ImgLocPara.fFirstColCosZ = info.ImgLocPara.fOriFirstColCosZ;


	info.ImgLocPara.bValide = true;

	info.ImgLocPara.fRowmm = info.ImgLocPara.nRow * info.ImgLocPara.fPixSpacingY;
	info.ImgLocPara.fColmm = info.ImgLocPara.nCol * info.ImgLocPara.fPixSpacingX;

	if (info.fPixelSpaceX < 0.000001)
		info.fPixelSpaceX = info.ImgLocPara.fPixSpacingX;
	if (info.fPixelSpaceY < 0.000001)
		info.fPixelSpaceY = info.ImgLocPara.fPixSpacingY;

	//���Ҳ�㵽�ְ�
	Hs_GetDoubleValueA(TAG_SLICE_THICKNESS,info.ImgLocPara.fSliceThickness);

	//��λ��Ҳ��
	Hs_GetDoubleValueA(TAG_SLICE_LOCATION,info.ImgLocPara.fSliceLoction);

	return true;
}

int CHsBaseFile::Hs_CopyTo( CHsBaseFile &NewFile)
{
	NewFile.m_BaseFileInfo = m_BaseFileInfo;
	NewFile.m_fp = NULL;
	NewFile.m_nDefaultReadSize = m_nDefaultReadSize;
	NewFile.m_pPreEle = NULL;
	NewFile.m_nBufLen = m_nBufLen;

	//copy buf
	if (NewFile.m_buf)
			delete []NewFile.m_buf;
	
	NewFile.m_buf = NULL;

	if(m_buf)
	{
		NewFile.m_buf = new BYTE[m_nBufLen];
		memcpy(NewFile.m_buf,m_buf, m_nBufLen);
	}

	//copy element
	if(NewFile.m_pMainEle)
		delete NewFile.m_pMainEle;

	NewFile.m_pMainEle = NULL;

	if (m_pMainEle)
	{
		NewFile.m_pMainEle = new HsElement;
		CopyEle(m_pMainEle,NewFile.m_pMainEle);

		m_pMainEle->pParentEle = NULL;

		NewFile.BuildListRelation(NewFile.m_pMainEle);
	}
	
	return 0;

}

void CHsBaseFile::CopyEle( HsElement* pFrom,HsElement* pTo )
{//pFrom��NUll�������700������Ǹ����п�����

	if(pFrom==NULL || pTo==NULL)
	{
		QByteArray ba = m_BaseFileInfo.sFileName.toLatin1();
		qDebug("\r\nFileName = %s;pFrom==0x%08x || pTo==0x%08x",ba.data(),pFrom,pTo);
		return;
	}

	*pTo = *pFrom;
	//pTo->bBigEndian = pFrom->bBigEndian;
	//pTo->bVirtualItem = pFrom->bVirtualItem;
	//pTo->nLen = pFrom->nLen;
	//pTo->nLenDesc = pFrom->nLenDesc;
	//pTo->nOffset = pFrom->nOffset;
	//pTo->nTag = pFrom->nTag;
	//pTo->nTagPos = pFrom->nTagPos;
	//pTo->nValueCount = pFrom->nValueCount;
	//pTo->nVR = pFrom->nVR;
	//pTo->pNextEle = NULL;
	//pTo->pPreEle = NULL;

	int n = int(pFrom->pChildEleV.size());
	for (int i=0;i<n;i++)
	{
		HsElement *pNewEle = new HsElement;
		CopyEle(pFrom->pChildEleV[i],pNewEle);
		pNewEle->pParentEle = pTo;

		pTo->pChildEleV.push_back(pNewEle);
	}
	
}


bool TAGSORT(const pHsElement &a,const pHsElement &b)
{
	return a->nTag < b->nTag;
}

//�༭DICOM�ļ�
pHsElement CHsBaseFile::Hs_InsertElement(pHsElement pNeighbor,bool bChild, unsigned long nTag,unsigned long nVR,bool bSquence,int nIndex,int &nRet)
{
	if (m_pMainEle == NULL)
	{
		m_pMainEle = new HsElement;
	}

	if (pNeighbor == NULL)
	{
		pNeighbor = m_pMainEle;
		pNeighbor->nVR = VR_SQ;
		bChild = true;
	}

	if ((nTag/65536)%2 == 0 && nTag!=TAG_ITEM)
	{
		if (CDcmTag::Find(nTag)->nVR!=nVR)
		{
			nRet = Ret_InvalidPara;
			return NULL;
		}
		else if (nVR != VR_SQ && bSquence == true)
		{
			nRet = Ret_InvalidPara;
			return NULL;
		}
	}

	if (nTag == TAG_ITEM)
	{
		int nItemNum;
		if (bChild == 0)
			nItemNum = pNeighbor->pParentEle->pChildEleV.size();
		else
			nItemNum = pNeighbor->pChildEleV.size();
		if (nIndex+1-nItemNum>1)
			nIndex = nItemNum;
	}


	HsElement *pInsertEle = new HsElement;
	pInsertEle->nLenDesc = CDcmVR::Find(nVR)->nLenOfLenDes;
	pInsertEle->nLen = 0;
	pInsertEle->bSquence = bSquence;
	pInsertEle->bNewTag = TRUE;

	if (nTag == TAG_ITEM)
		pInsertEle->nOffset = 8;	
	else
		pInsertEle->nOffset = 4 + CDcmVR::Find(nVR)->nLenOfVrDes + CDcmVR::Find(nVR)->nLenOfLenDes;

	if (nTag!=0)
	{
		pInsertEle->nTag = nTag;
	}
	else
	{
		DestroyEle(pInsertEle);
		nRet = Ret_InvalidPara;
		return NULL;
	}

	if (nTag == TAG_ITEM)
	{
		nVR = NULL;
	}
	else
	{
		if ((nTag/65536)%2!=0 && bSquence==true)
			nVR = VR_SQ;
		else
		{
			if(nVR)
			{
				pDcmVR pVR = CDcmVR::Find(nVR);
				if (pVR->nCode == 0)	
				{
					nRet = Ret_InvalidPara;
					return NULL;
				}
				pInsertEle->nVR = nVR;
			}
			else
			{
				pInsertEle->nVR = CDcmTag::Find(nTag)->nVR;
			}

		}
	}


	if (bChild ==0)
	{
		if (pNeighbor->pParentEle)
		{
			pInsertEle->pParentEle = pNeighbor->pParentEle;
		}
		else
		{
			DestroyEle(pInsertEle);
			nRet = Ret_InvalidPara;
			return NULL;
		}
	}
	else
	{
		if ((pNeighbor->nVR == VR_SQ && pInsertEle->nTag == TAG_ITEM)|| pNeighbor->nTag == TAG_ITEM || pNeighbor->nVR == VR_SQ)
		{
			pInsertEle->pParentEle = pNeighbor;
		}
		else
		{
			DestroyEle(pInsertEle);
			nRet = Ret_InvalidPara;
			return NULL;
		}		
	}

	if (nTag == TAG_ITEM)
	{
		if ((bChild == 0 && pNeighbor->pParentEle->nVR != VR_SQ)||
			(bChild && pNeighbor->nVR != VR_SQ) )
		{
			DestroyEle(pInsertEle);
			nRet = Ret_InvalidPara;
			return NULL;
		}

		if (nIndex == 0)
		{
			pInsertEle->pPreEle = pInsertEle->pParentEle;
			if (pInsertEle->pParentEle->pChildEleV.size() != 0)
				pInsertEle->pNextEle = pInsertEle->pParentEle->pChildEleV[0];
			else
				pInsertEle->pNextEle = pInsertEle->pPreEle->pNextEle;

		}
		else
		{
			pInsertEle->pPreEle = pInsertEle->pParentEle->pChildEleV[nIndex-1];
			pInsertEle->pNextEle = pInsertEle->pParentEle->pChildEleV[nIndex-1]->pNextEle;
		}
	}
	else
	{
		int nOldChildNum = pInsertEle->pParentEle->pChildEleV.size();
		for (int i = 0; i<nOldChildNum; i++)
		{
			unsigned long nPreTag = pInsertEle->pParentEle->pChildEleV[i]->nTag;
			if (nTag == nPreTag)
			{
				DestroyEle(pInsertEle);
				return NULL;
			}
			else if (nTag<nPreTag)
			{
				if (i == 0)
				{
					pInsertEle->pPreEle = pInsertEle->pParentEle;
					pInsertEle->pNextEle = pInsertEle->pParentEle->pChildEleV[i];
				}
				else
				{
					pInsertEle->pPreEle = pInsertEle->pParentEle->pChildEleV[i-1];
					pInsertEle->pNextEle = pInsertEle->pParentEle->pChildEleV[i];
					break;
				}
			}
		}
	}

	pInsertEle->pParentEle->pChildEleV.push_back(pInsertEle);
	qSort(pInsertEle->pParentEle->pChildEleV.begin(),pInsertEle->pParentEle->pChildEleV.end(),TAGSORT);

	nRet = Ret_Success;
	return pInsertEle;

}


int CHsBaseFile::Hs_DeleteElement(pHsElement pEle)
{
	if(pEle==NULL)
		return Ret_InvalidPara;

	if (pEle->pParentEle)
	{
		for (QVector<pHsElement>::iterator i =pEle->pParentEle->pChildEleV.begin(); i<pEle->pParentEle->pChildEleV.end();i++ )
		{
			if ((*i) == pEle)
			{
				pEle->pParentEle->pChildEleV.erase(i);
				break;
			}
		}
	}
	if (pEle->pPreEle)
	{
		pEle->pPreEle->pNextEle = pEle->pNextEle;
	}
	if (pEle->pNextEle)
	{
		pEle->pNextEle->pPreEle = pEle->pPreEle;
	}

	delete pEle;

	return Ret_Success;
}


int CHsBaseFile::Hs_SetLongValue(pHsElement pEle, long nValue,bool bCover=false,int nValueIndex=0)
{
	if (pEle == NULL)
		return Ret_InvalidElement;
	if(nValueIndex<0)
		return Ret_OutOfValueCount;
	if (pEle->nVR != VR_SL&& pEle->nVR != VR_UL &&pEle->nVR != VR_AT&&pEle->nVR!=VR_IS)
		return Ret_InvalidPara;

	this->ValueToEle((void*)&nValue,pEle,sizeof(long),bCover,nValueIndex);

	pEle->bNewTag = TRUE;

	return Ret_Success;
}

int CHsBaseFile::Hs_SetShortValue(pHsElement pEle, short nValue,bool bCover=false,int nValueIndex=0)
{
	if (pEle == NULL)
		return Ret_InvalidPara;
	if(nValueIndex<0)
		return Ret_OutOfValueCount;
	if (pEle->nVR != VR_SS && pEle->nVR != VR_US && pEle->nVR != VR_OW && pEle->nVR != VR_OB)
		return Ret_InvalidPara;

	if (pEle->nVR == VR_OB)
		this->ValueToEle((void*)&nValue,pEle,1,bCover,nValueIndex);
	else
		this->ValueToEle((void*)&nValue,pEle,sizeof(short),bCover,nValueIndex);


	pEle->bNewTag = TRUE;

	return Ret_Success;
}

int CHsBaseFile::Hs_SetDoubleValue(pHsElement pEle, double nValue,bool bCover=false,int nValueIndex=0 )
{
	if (pEle == NULL)
		return Ret_InvalidPara;
	if(nValueIndex<0)
		return Ret_OutOfValueCount;
	if (pEle->nVR !=VR_FD && pEle->nVR != VR_DS)// &&pEle->nVR != VR_OF && pEle->nVR != VR_FL  )
		return Ret_InvalidPara;

	this->ValueToEle((void*)&nValue,pEle,sizeof(double),bCover,nValueIndex);

	pEle->bNewTag = TRUE;
	return Ret_Success;
}

int CHsBaseFile::Hs_SetFloatValue(pHsElement pEle, float nValue,bool bCover=false,int nValueIndex=0 )
{
	if (pEle == NULL)
		return Ret_InvalidPara;
	if(nValueIndex<0)
		return Ret_OutOfValueCount;
	if (pEle->nVR != VR_FL)
		return Ret_InvalidPara;

	this->ValueToEle((void*)&nValue,pEle,sizeof(float),bCover,nValueIndex);

	pEle->bNewTag = TRUE;
	return Ret_Success;
}

int CHsBaseFile::Hs_SetStringValue(pHsElement pEle, QString nValue,bool bCover=false,int nValueIndex=0 )
{
	if (pEle == NULL)
		return Ret_InvalidPara;
	if(nValueIndex<0)
		return Ret_OutOfValueCount;
	if (pEle->nVR!=VR_CS && pEle->nVR!=VR_SH && pEle->nVR!=VR_LO && pEle->nVR!=VR_AE && pEle->nVR!=VR_LT
		&& pEle->nVR!=VR_ST && pEle->nVR!=VR_UI && pEle->nVR!=VR_UT && pEle->nVR!=VR_PN)
		return Ret_InvalidPara;

	QByteArray ba = nValue.toLatin1();
	this->ValueToEle((void*)ba.data(), pEle, nValue.length(), bCover, nValueIndex);

	pEle->bNewTag = TRUE;
	return Ret_Success;
}

int CHsBaseFile::Hs_SetDataValue(pHsElement pEle, HsDateTime nValue,bool bCover=false,int nValueIndex=0)
{
	if (pEle == NULL)
		return Ret_InvalidPara;
	if(nValueIndex<0)
		return Ret_OutOfValueCount;
	if (pEle->nVR != VR_DA )
		return Ret_InvalidPara;
	if (nValue.nMonth>12|| nValue.nDay>31)
		return Ret_InvalidPara;

	char temp[9] ;
	memset(temp,0,9);
	sprintf_s(temp,"%04d%02d%02d",nValue.nYear,nValue.nMonth,nValue.nDay);

	this->ValueToEle((void*)&temp,pEle,8,bCover,nValueIndex);

	pEle->bNewTag = TRUE;
	return Ret_Success;
}

int CHsBaseFile::Hs_SetTimeValue(pHsElement pEle, HsDateTime nValue,bool bCover=false,int nValueIndex=0)
{
	if (pEle == NULL)
		return Ret_InvalidPara;
	if(nValueIndex<0)
		return Ret_OutOfValueCount;
	if (pEle->nVR != VR_TM )
		return Ret_InvalidPara;
	if (nValue.nHours>23|| nValue.nMinutes>59||nValue.nSeconds>59||nValue.nFractions>999999)
		return Ret_InvalidPara;

	char temp[100] ;
	memset(temp,0,100);
	if (nValue.nFractions!=0)
		sprintf_s(temp,"%02d%02d%02d.%d",nValue.nHours,nValue.nMinutes,nValue.nSeconds,nValue.nFractions);
	else
		sprintf_s(temp,"%02d%02d%02d",nValue.nHours,nValue.nMinutes,nValue.nSeconds);


	int nLen = strlen(temp);
	this->ValueToEle((void*)&temp,pEle,nLen,bCover,nValueIndex);

	pEle->bNewTag = TRUE;
	return Ret_Success;
}
int CHsBaseFile::Hs_SetAgeValue(pHsElement pEle,int nAge,char cAgeType,bool bCover=false,int nValueIndex=0)
{
	if (pEle == NULL)
		return Ret_InvalidPara;
	if(nValueIndex<0)
		return Ret_OutOfValueCount;
	if (pEle->nVR != VR_AS )
		return Ret_InvalidPara;
	if (nAge>999)
		return Ret_InvalidPara;

	int nRet = 0;
	int nCount = Hs_GetValueCount(pEle,nRet);
	if (nCount>0)
		return Ret_InvalidPara;

	QString sAgeType;
	sAgeType = QString("%1").arg(cAgeType);
	if (!(sAgeType.compare("D") || sAgeType.compare("M") || sAgeType.compare("Y")))
		return Ret_InvalidPara;

	char Temp[100];
	memset(Temp,0,100);

	sprintf(Temp,"%1%2",nAge,cAgeType);	


	this->ValueToEle((void*)&Temp,pEle,4,bCover,nValueIndex);

	pEle->bNewTag = TRUE;
	return Ret_Success;
}
int CHsBaseFile::Hs_SetDataTimeValue(pHsElement pEle, HsDateTime nValue,bool bCover=false,int nValueIndex=0)
{
	if (pEle == NULL)
		return Ret_InvalidPara;
	if(nValueIndex<0)
		return Ret_OutOfValueCount;
	if (pEle->nVR != VR_DT )
		return Ret_InvalidPara;
	if (nValue.nHours>23|| nValue.nMinutes>59||nValue.nSeconds>59||nValue.nFractions>999999||nValue.nMonth>12|| nValue.nDay>31)
		return Ret_InvalidPara;

	char temp[100] ;
	memset(temp,0,100);
	QString sOffsetState="";
	if (nValue.nOffset>=0)
		sOffsetState = "+";
	else
		sOffsetState = "-";

	QByteArray ba = sOffsetState.toLatin1();
	sprintf_s(temp,"%04d%02d%02d%02d%02d%02d.%d%c%04d",nValue.nYear,nValue.nMonth,nValue.nDay,nValue.nFractions,ba.data(),nValue.nOffset);

	int nLen = strlen(temp);

	this->ValueToEle((void*)&temp,pEle,nLen,bCover,nValueIndex);

	pEle->bNewTag = TRUE;
	return Ret_Success;
}

int CHsBaseFile::ValueToEle(void *pData,pHsElement pEle, int nNewValueLen,bool bCover, int nValueIndex)
{
	if (nNewValueLen ==0 || pData == NULL || pEle == NULL || nValueIndex<0)
	{
		return 0;
	}

	if (pEle->nVR == VR_LT || pEle->nVR == VR_ST || pEle->nVR == VR_UT)  //������VR���Ͳ����ж�ֵ
	{
		if (nValueIndex>0)
			return Ret_OutOfValueCount;
	}
	int iFunState;
	unsigned long nValueCount = Hs_GetValueCount(pEle,iFunState);
	BYTE *pDataByte = new BYTE[nNewValueLen];
	memcpy(pDataByte,pData,nNewValueLen);
	BYTE *pTempValue = new BYTE[nNewValueLen];

	if (pEle->nVR == VR_SL  || pEle->nVR == VR_UL || pEle->nVR == VR_FL || pEle->nVR == VR_OF)
	{		
		if (pEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia)
		{
			pTempValue[0] = pDataByte[3];pTempValue[1] = pDataByte[2]; pTempValue[2] = pDataByte[1]; pTempValue[3] = pDataByte[0];
		}
		else
		{
			memcpy(pTempValue,pDataByte,nNewValueLen);
		}
	}
	else if (pEle->nVR == VR_AT)
	{		
		if (pEle->bBigEndian== false)
		{
			if (m_BaseFileInfo.bCpuBigEndia  == false)
			{
				pTempValue[0] = pDataByte[2]; pTempValue[1] = pDataByte[3]; pTempValue[2] = pDataByte[0]; pTempValue[3] = pDataByte[1];
			}
			else
			{
				pTempValue[0] = pDataByte[3]; pTempValue[1] = pDataByte[2]; pTempValue[2] = pDataByte[1]; pTempValue[3] = pDataByte[0];
			}
		}
		else
		{
			if (m_BaseFileInfo.bCpuBigEndia  ==false)
			{
				pTempValue[0] = pDataByte[1]; pTempValue[1] = pDataByte[0]; pTempValue[2] = pDataByte[3]; pTempValue[3] = pDataByte[2];
			}
			else
			{
				memcpy(&pTempValue,&pDataByte,4);
			}
		}
	}
	else if (pEle->nVR == VR_SS|| pEle->nVR == VR_US|| pEle->nVR == VR_OW )
	{
		if (pEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia)
		{
			pTempValue[0] = pDataByte[1]; pTempValue[1] = pDataByte[0];
		}
		else
		{
			memcpy(pTempValue,pDataByte,nNewValueLen);
		}
	}
	else if (pEle->nVR ==VR_FD )
	{
		if (pEle->bBigEndian!=m_BaseFileInfo.bCpuBigEndia)
		{
			pTempValue[0] = pDataByte[7]; pTempValue[1] = pDataByte[6];pTempValue[2] = pDataByte[5];pTempValue[3] = pDataByte[4];
			pTempValue[4] = pDataByte[3]; pTempValue[5] = pDataByte[2];pTempValue[6] = pDataByte[1];pTempValue[7] = pDataByte[0];
		}
		else
		{
			memcpy(pTempValue,pDataByte,nNewValueLen);
		}
	}
	else if (pEle->nVR == VR_IS || pEle->nVR == VR_DS)
	{
		char Is[100];
		memset(Is,0,100);
		if (pEle->nVR == VR_IS)
			sprintf(Is,"%d",*(long*)pDataByte);
		else
			sprintf(Is,"%f",*(double*)pDataByte);		
		nNewValueLen = min(nNewValueLen,(int)strlen(Is));//memcpy(pTempValue,Is,nNewValueLen);pTempValue new��8�����˴�����10�������ܲ���

		if (pEle->nVR == VR_IS)
		{
			if (nNewValueLen>12)
				return 0;
		}
		else
		{
			if (nNewValueLen>16)
				return 0;
		}

		memcpy(pTempValue,Is,nNewValueLen);
	}
	else if (pEle->nVR==VR_CS || pEle->nVR==VR_SH || pEle->nVR==VR_LO || pEle->nVR==VR_AE || pEle->nVR==VR_LT|| pEle->nVR==VR_ST  
		|| pEle->nVR==VR_UT || pEle->nVR==VR_PN||pEle->nVR == VR_DA || pEle->nVR == VR_DT|| pEle->nVR == VR_TM || pEle->nVR == VR_AS)
	{
		memcpy(pTempValue,pDataByte,nNewValueLen);
	}
	else if (pEle->nVR == VR_OB|| pEle->nVR==VR_UI)
	{
		memcpy(pTempValue,pDataByte,nNewValueLen);
	}
	else if(pEle->nVR == VR_UN || pEle->nVR == VR_00)
	{
		//char Is[100];
		//memset(Is,0,100);
		//sprintf(Is,"%c",pDataByte);
		//iTypeSize = strlen(Is);
		//if (iTypeSize%2!=0)
		//	iNewTypeSize = iTypeSize + 1;	
		//if (pTempValue)
		//{
		//	delete pTempValue;
		//	pTempValue = NULL;
		//}
		//memset(pTempValue,0x20,iNewTypeSize);
		//if (m_BaseFileInfo.bCpuBigEndia == true)
		//{
		//	for (int i =0; i<iTypeSize;i++)
		//	{
		//		pTempValue[i] = pDataByte[iTypeSize-1-i];
		//	}
		//}
		//else
		//{
		//	memcpy(pTempValue,pDataByte,iTypeSize);
		//}
	}
	if (nValueCount == 0)//���1��ԭ��Ele��û��ֵ��
	{
		pEle->nLen = pEle->nLen + nNewValueLen;
		int nOldLen = pEle->nLen;
		if (pEle->nVR !=VR_UL && pEle->nVR != VR_FL && pEle->nVR != VR_FD && pEle->nVR != VR_SS  && pEle->nVR != VR_SL  && pEle->nVR != VR_US && pEle->nVR != VR_AT && pEle->nVR != VR_OB)//add by wxs for hmy
		{
			if (pEle->nLen%2 != 0)
				pEle->nLen = pEle->nLen + 1;
		}
		pEle->pValue = new BYTE[pEle->nLen];
		memset(pEle->pValue,0,pEle->nLen);//DICOM�涨ֵ�򳤶�Ϊ����ʱ��Ӧ��0x20(��:�ո�)����ż�����ֽ�---��Ҫ����֤������
		memcpy(pEle->pValue,pTempValue,nOldLen);	

		//for (int ii=0;ii<pEle->nLen;ii++)
		//{
		//	if(ii==55)
		//		int g = 0;
		//	AtlTrace("%02d: --------  [%c] -------  [%d] --------  [%x]\r\n",ii,pEle->pValue[ii],pEle->pValue[ii],pEle->pValue[ii]);
		//}
	}
	else if (nValueCount-1<nValueIndex)//���2�����뵽ԭ��Eleֵ�������������£���ִ�Ele�Ƿ��ڱ��α����ļ�ǰ���޸Ĺ�
	{
		BYTE *pNewValue = NULL;
		if (pEle->nVR ==VR_UL || pEle->nVR == VR_FL || pEle->nVR == VR_FD || pEle->nVR == VR_SS || pEle->nVR == VR_SL || pEle->nVR == VR_US || pEle->nVR == VR_AT)
			pNewValue = new BYTE[pEle->nLen + nNewValueLen];
		else
			pNewValue = new BYTE[pEle->nLen + nNewValueLen + 1];

		memcpy(pNewValue,pEle->pValue,pEle->nLen);
		delete pEle->pValue;
		pEle->pValue = NULL;

		pNewValue += pEle->nLen;

		if (pEle->nVR!=VR_UL && pEle->nVR != VR_FL && pEle->nVR != VR_FD && pEle->nVR != VR_SS && pEle->nVR != VR_SL && pEle->nVR != VR_US && pEle->nVR != VR_AT && pEle->nVR != VR_OB)
		{
			pNewValue[0] = 92;
			pNewValue += 1;
			memcpy(pNewValue,pTempValue,nNewValueLen);
			pNewValue =pNewValue - pEle->nLen - 1;
			pEle->nLen = pEle->nLen + nNewValueLen + 1;
		}
		else
		{		
			memcpy(pNewValue,pTempValue,nNewValueLen);
			pNewValue -= pEle->nLen;
			pEle->nLen = pEle->nLen + nNewValueLen;
		}

		int nOldLen = pEle->nLen;
		if (pEle->nLen%2 != 0)
			pEle->nLen = pEle->nLen + 1;

		pEle->pValue = new BYTE[pEle->nLen];
		memset(pEle->pValue,0,pEle->nLen);
		memcpy(pEle->pValue,pNewValue,nOldLen);
		delete []pNewValue;
	}
	else if (nValueCount-1>=nValueIndex)//���3������ԭ��ֵ���У����ҷ�Ϊ�Ƿ��Ӧλ��ԭ��ֵ
	{
		BYTE *pNewValue = NULL;
		if (pEle->nVR ==VR_UL || pEle->nVR == VR_FL || pEle->nVR == VR_FD || pEle->nVR == VR_SS || pEle->nVR == VR_SL || pEle->nVR == VR_US || pEle->nVR == VR_AT || pEle->nVR == VR_OB)
		{
			if (bCover == false)
				pNewValue = new BYTE[pEle->nLen+nNewValueLen];
			else
				pNewValue = new BYTE[pEle->nLen];
			BYTE *pOldPos = pNewValue;
			memcpy(pNewValue,pEle->pValue,nValueIndex*nNewValueLen);
			pNewValue += nValueIndex*nNewValueLen;
			memcpy(pNewValue,pTempValue,nNewValueLen);
			pNewValue += nNewValueLen;
			if (bCover == false)
			{
				memcpy(pNewValue,pEle->pValue+nValueIndex*nNewValueLen,pEle->nLen - nValueIndex*nNewValueLen);
				pEle->nLen += nNewValueLen;
			}
			else
			{
				memcpy(pNewValue,pEle->pValue+nValueIndex*nNewValueLen,pEle->nLen - nValueIndex*nNewValueLen);
			}
			pNewValue = pNewValue - nValueIndex*nNewValueLen - nNewValueLen; 
			if (pEle->pValue!=NULL)
			{
				delete []pEle->pValue;
				pEle->pValue = NULL;
			}
			pEle->pValue = new BYTE[pEle->nLen];
			memcpy(pEle->pValue,pNewValue,pEle->nLen);
			delete []pOldPos;
		}
		else
		{
			QVector<QString> OldValues;
			QVector<int> ValueLen;			
			for (int i = 0; i<nValueCount; i++)
			{
				QString Value;
				Hs_GetStringValue(pEle,Value,i);
				OldValues.push_back(Value);
				int Len = Value.length();
				ValueLen.push_back(Len);
			}
			if (bCover == false)
				pNewValue = new BYTE[pEle->nLen+nNewValueLen+1];
			else
				pNewValue = new BYTE[pEle->nLen - ValueLen[nValueIndex] + nNewValueLen];

			int iPreLen = 0;
			for (int i = 0; i<nValueIndex;i++)
			{
				iPreLen = iPreLen + ValueLen[i] + 1;
			}

			memcpy(pNewValue,pEle->pValue,iPreLen);			

			pNewValue += iPreLen;
			memcpy(pNewValue,pTempValue,nNewValueLen);
			pNewValue += nNewValueLen;
			if (bCover == false)
			{
				pNewValue[0] = 92;//'\';
				pNewValue += 1;
				memcpy(pNewValue,pEle->pValue+iPreLen,pEle->nLen - iPreLen);
				pEle->nLen = pEle->nLen + nNewValueLen + 1;
				pNewValue = pNewValue - iPreLen - nNewValueLen -1;
			}
			else
			{
				memcpy(pNewValue,pEle->pValue+iPreLen+ValueLen[nValueIndex],pEle->nLen - iPreLen - ValueLen[nValueIndex]);
				pEle->nLen = pEle->nLen - ValueLen[nValueIndex] + nNewValueLen;
				pNewValue = pNewValue - iPreLen - nNewValueLen;
			}
			if (pEle->pValue!=NULL)
			{
				delete pEle->pValue;
				pEle->pValue = NULL;
			}

			int nOldLen = pEle->nLen;
			if (pEle->nLen%2 != 0)
				pEle->nLen = pEle->nLen + 1;
			pEle->pValue = new BYTE[pEle->nLen];
			memset(pEle->pValue,0,pEle->nLen);
			memcpy(pEle->pValue,pNewValue,nOldLen);
			delete []pNewValue;
		}		
	}
	delete []pDataByte;
	delete []pTempValue;
	//TRACE("\r\n %d %d %c %d",pEle->pValue[6],pEle->pValue[7],pEle->pValue[8],pEle->pValue[9]);
	return Ret_Success;
}

int CHsBaseFile::Hs_SaveFile(const char *cFileName,int nTsType)
{
	bool bBigEndia = false;
	bool bConvert = false;

	CalcInformationLength(nTsType);

	if (nTsType == TS_EXPLICIT_VR_BIG_ENDIAN)
		bBigEndia = true; 

    QString str = QString(QLatin1String(cFileName));
    HANDLE hf = ::CreateFile(str.toStdWString().data(),GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		0);

	if (hf==INVALID_HANDLE_VALUE)
		return Ret_CreateFileFail;

	//�����ǰ����
	::SetFilePointer(hf,0,NULL,FILE_BEGIN);
	::SetEndOfFile(hf);

	DWORD nBt=0;

	char ucFirstTag[132];
	memset(ucFirstTag,0,132);
	ucFirstTag[128] = 'D'; ucFirstTag[129] = 'I';ucFirstTag[130] = 'C';ucFirstTag[131] = 'M';
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,ucFirstTag,132,&nBt,0);

	/*int nRet = 0;
	pHsElement pTransEle = Hs_FindFirstEle(NULL,TAG_TRANSFER_SYNTAX_UID,true);
	if(pTransEle == NULL)
		Hs_InsertElement(NULL,true,TAG_TRANSFER_SYNTAX_UID,CDcmTag::Find(TAG_TRANSFER_SYNTAX_UID)->nVR,false,0,nRet);
	*/

	int n = m_pMainEle->pChildEleV.size();

	for (int i = 0; i<n; i++)
	{
		pHsElement pTempEle = m_pMainEle->pChildEleV[i];

		unsigned long nTag = pTempEle->nTag;

		if ((nTag>>16)%2!=0)
			continue;

		if (pTempEle->nTag == TAG_TRANSFER_SYNTAX_UID)
		{
			if (m_BaseFileInfo.nTsType != nTsType)
			{
				SaveTsType(pTempEle,nTsType,hf);
			}
			else
			{
				this->SaveNormalEle(pTempEle,hf,bBigEndia,bConvert);
			}
			if ((m_BaseFileInfo.nTsType == TS_EXPLICIT_VR_BIG_ENDIAN && nTsType != TS_EXPLICIT_VR_BIG_ENDIAN) ||
				(m_BaseFileInfo.nTsType != TS_EXPLICIT_VR_BIG_ENDIAN && nTsType == TS_EXPLICIT_VR_BIG_ENDIAN))
				bConvert = TRUE;
		}
		else if (pTempEle->nVR == VR_SQ)
		{
			this->SaveSquenceEle(pTempEle,hf,bBigEndia,bConvert);
		}
		else if (pTempEle->nTag == TAG_ITEM)
		{
			this->SaveItemEle(pTempEle,hf,bBigEndia,bConvert);
		}	
		else if (pTempEle->nTag == TAG_PIXEL_DATA)
		{
			this->SavePixelDataEle(pTempEle,hf,bBigEndia,bConvert);
		}
		else
		{
			this->SaveNormalEle(pTempEle,hf,bBigEndia,bConvert);
		}		

	}

	CloseHandle(hf);
	return Ret_Success;
}

int CHsBaseFile::SaveTsType(pHsElement pEle,int nTsType,HANDLE &hf)
{
	DWORD nBt=0;

	//1��дTagNumber
	BYTE *TempTagNum = (BYTE*) &pEle->nTag;
	BYTE *TagNum = WritePara(pEle,TempTagNum,4,false,Para_TagNum);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,TagNum,4,&nBt,0);
	delete []TagNum;

	//2��VR
	pDcmVR pVR = CDcmVR::Find(pEle->nVR);
	BYTE *TempTagVR = (BYTE*)&pEle->nVR;
	BYTE *TagVR = WritePara(pEle,TempTagVR,pVR->nLenOfVrDes,false,Para_VR);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,TagVR,pVR->nLenOfVrDes,&nBt,0);
	delete []TagVR;


	char *cTsTypeName = new char[100];
	memset(cTsTypeName,0,100);
	unsigned long iValueLen;

	if (nTsType == TS_IMPLICIT_VR_LITTLE_ENDIAN)
	{	
		strcpy(	cTsTypeName, "1.2.840.10008.1.2");
	}
	else if (nTsType == TS_EXPLICIT_VR_LITTLE_ENDIAN)
	{
		strcpy(	cTsTypeName,"1.2.840.10008.1.2.1");
	}
	else if (nTsType == TS_EXPLICIT_VR_BIG_ENDIAN)
	{
		strcpy(	cTsTypeName,"1.2.840.10008.1.2.2");
	}
	else if (nTsType == TS_RLE_LOSSLESS)
	{
		strcpy(	cTsTypeName,"1.2.840.10008.1.2.5");
	}
	else if (nTsType == TS_JPEG_BASELINE_1)
	{
		strcpy(	cTsTypeName,"1.2.840.10008.1.2.4.50");
	}
	else if (nTsType == TS_JPEG_EXTENDED_2_4)
	{
		strcpy(	cTsTypeName,"1.2.840.10008.1.2.4.51");
	}
	else if (nTsType == TS_JPEG_LOSSLESS_NONHIER_14)
	{
		strcpy(	cTsTypeName,"1.2.840.10008.1.2.4.57");
	}
	else if (nTsType == TS_JPEG_LOSSLESS_NONHIER_14B)
	{
		strcpy(	cTsTypeName,"1.2.840.10008.1.2.4.70");
	}
	else if (nTsType == TS_JPEG2000_LOSSLESS_ONLY)
	{
		strcpy(	cTsTypeName,"1.2.840.10008.1.2.4.90");
	}
	else if (nTsType == TS_JPEG2000)
	{
		strcpy(	cTsTypeName,"1.2.840.10008.1.2.4.91");
	}
	else
	{
		strcpy(	cTsTypeName, "1.2.840.10008.1.2");
	}

	iValueLen = strlen(cTsTypeName);
	if (iValueLen%2 != 0)
	{
		iValueLen += 1;
	}

	//3��LEN
	BYTE *TempTagLen = (BYTE *) &iValueLen;
	BYTE *TagLen = WritePara(pEle,TempTagLen,pVR->nLenOfLenDes,false,Para_Len);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,TagLen,pVR->nLenOfLenDes,&nBt,0);
	delete []TagLen;

	//ֵ��
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,cTsTypeName,iValueLen,&nBt,0);
	delete []cTsTypeName;

	return Ret_Success;
}

int CHsBaseFile::SaveSquenceEle(pHsElement pEle,HANDLE &hf,bool bBigEndia,bool bConvert)
{
	if (pEle == NULL)
	{
		return 0;
	}

	DWORD nBt=0;

	//1��дTagNumber
	BYTE *TempTagNum = (BYTE*) &pEle->nTag;
	BYTE *TagNum = WritePara(pEle,TempTagNum,4,bBigEndia,Para_TagNum);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,TagNum,4,&nBt,0);
	delete []TagNum;

	//2��VR
	pDcmVR pVR = CDcmVR::Find(pEle->nVR);
	if (m_BaseFileInfo.nTsType != TS_IMPLICIT_VR_LITTLE_ENDIAN)
	{
		BYTE *TempTagVR = (BYTE*)&pEle->nVR;
		BYTE *TagVR = WritePara(pEle,TempTagVR,pVR->nLenOfVrDes,bBigEndia,Para_VR);
		::SetFilePointer(hf,0,NULL,FILE_END);
		::WriteFile(hf,TagVR,pVR->nLenOfVrDes,&nBt,0);
		delete []TagVR;
	}

	//3��LEN = 0xFFFFFFFF
	pEle->nLen = 0xFFFFFFFF;
	BYTE *TempTagLen = (BYTE *) &pEle->nLen;
	BYTE *TagLen = WritePara(pEle,TempTagLen,pVR->nLenOfLenDes,bBigEndia,Para_Len);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,TagLen,pVR->nLenOfLenDes,&nBt,0);
	delete []TagLen;

	//4��ֵ��:
	int n = pEle->pChildEleV.size();

	for (int i=0; i<n; i++)
	{
		pHsElement pTempEle = pEle->pChildEleV[i];
		if (pTempEle->nVR == VR_SQ)
		{
			this->SaveSquenceEle(pTempEle,hf,bBigEndia,bConvert);
		}
		else if (pTempEle->nTag == TAG_ITEM)
		{
			this->SaveItemEle(pTempEle,hf,bBigEndia,bConvert);
		}
		else
		{
			this->SaveNormalEle(pTempEle,hf,bBigEndia,bConvert);
		}	
	}	

	//5��SQ������
	unsigned long nEndTagNum= 0xFFFEE0DD;
	BYTE *TempSQEndTag = (BYTE*) &nEndTagNum;
	BYTE *SQEndTagNum = WritePara(pEle,TempSQEndTag,4,bBigEndia,Para_TagNum);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,SQEndTagNum,4,&nBt,0);
	delete []SQEndTagNum;

	unsigned long nSqEnd = 0;
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,&nSqEnd,4,&nBt,0);	

	return Ret_Success;
}

int CHsBaseFile::SaveItemEle(pHsElement pEle,HANDLE &hf,bool bBigEndia,bool bConvert)
{	
	if (pEle == NULL)
	{
		return 0;
	}

	DWORD nBt=0;

	//1��дTagNumber
	BYTE *TempTagNum = (BYTE*) &pEle->nTag;
	BYTE *TagNum = WritePara(pEle,TempTagNum,4,bBigEndia,Para_TagNum);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,TagNum,4,&nBt,0);
	delete []TagNum;

	//2��LEN = 0xFFFFFFFF
	pEle->nLen = 0xFFFFFFFF;
	BYTE *TempTagLen = (BYTE *) &pEle->nLen;
	BYTE *TagLen = WritePara(pEle,TempTagLen,4,bBigEndia,Para_Len);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,TagLen,4,&nBt,0);
	delete []TagLen;

	//3��ֵ��:
	int n = pEle->pChildEleV.size();

	for (int i=0; i<n; i++)
	{
		pHsElement pTempEle = pEle->pChildEleV[i];
		if (pTempEle->nVR == VR_SQ)
		{
			this->SaveSquenceEle(pTempEle,hf,bBigEndia,bConvert);
		}
		else if (pTempEle->nTag == TAG_ITEM)
		{
			this->SaveItemEle(pTempEle,hf,bBigEndia,bConvert);
		}
		else
		{
			this->SaveNormalEle(pTempEle,hf,bBigEndia,bConvert);
		}	
	}	

	//4��ITEM������
	unsigned long nEndTagNum= 0xFFFEE00D;
	BYTE *TempItemEndTag = (BYTE*) &nEndTagNum;
	BYTE *ItemEndTagNum = WritePara(pEle,TempItemEndTag,4,bBigEndia,Para_TagNum);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,ItemEndTagNum,4,&nBt,0);
	delete []ItemEndTagNum;

	unsigned long nItemEnd = 0;
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,&nItemEnd,4,&nBt,0);

	return Ret_Success;
}

int CHsBaseFile::SaveNormalEle(pHsElement pEle,HANDLE &hf,bool bBigEndia,bool bConvert)
{
	if (pEle == NULL)
	{
		return 0;
	}

	DWORD nBt=0;

	//1��дTagNumber
	BYTE *TempTagNum = (BYTE*) &pEle->nTag;
	BYTE *TagNum = WritePara(pEle,TempTagNum,4,bBigEndia,Para_TagNum);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,TagNum,4,&nBt,0);
	delete []TagNum;

	//2��VR
	pDcmVR pVR = CDcmVR::Find(pEle->nVR);
	if (m_BaseFileInfo.nTsType != TS_IMPLICIT_VR_LITTLE_ENDIAN)
	{
		if (pEle->nVR == VR_00)
			pEle->nVR = VR_UN;
		BYTE *TempTagVR = (BYTE*)&pEle->nVR;
		BYTE *TagVR = WritePara(pEle,TempTagVR,pVR->nLenOfVrDes,false,Para_VR);
		::SetFilePointer(hf,0,NULL,FILE_END);
		::WriteFile(hf,TagVR,pVR->nLenOfVrDes,&nBt,0);
		delete []TagVR;
	}

	//3��LEN
	BYTE *TempTagLen = (BYTE *) &pEle->nLen;
	int nLenOfLenDes;
	if (m_BaseFileInfo.nTsType == TS_IMPLICIT_VR_LITTLE_ENDIAN)
		nLenOfLenDes = 4;
	else
		nLenOfLenDes = pVR->nLenOfLenDes;	  
	BYTE *TagLen = WritePara(pEle,TempTagLen,nLenOfLenDes,bBigEndia,Para_Len);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,TagLen,nLenOfLenDes,&nBt,0);
	delete []TagLen;

	//4��ֵ��:
	//if (pEle->bNewTag == TRUE)
	//{
		if (pEle->pValue != NULL)
		{
			BYTE *TempTagValue = pEle->pValue;
			BYTE *TagValue = WriteValue(pEle,TempTagValue,pEle->nLen,false);
			::SetFilePointer(hf,0,NULL,FILE_END);
			::WriteFile(hf,TagValue,pEle->nLen,&nBt,0);
			delete []TagValue;
		}
		else
		{
			return Ret_Success;
		}
	/*}
	else
	{
		BYTE *TempTagValue = new BYTE[pEle->nLen];
		memcpy(TempTagValue,&(m_buf[pEle->nTagPos+pEle->nOffset]),pEle->nLen);		
		BYTE *TagValue = WriteValue(pEle,TempTagValue,pEle->nLen,bConvert);
		::SetFilePointer(hf,0,NULL,FILE_END);
		::WriteFile(hf,TagValue,pEle->nLen,&nBt,0);
		delete []TempTagValue;
		delete []TagValue;
	}*/

	return Ret_Success;
}

int CHsBaseFile::SavePixelDataEle(pHsElement pPixelEle,HANDLE &hf,bool bBigEndia,bool bConvert)
{
	if (pPixelEle == NULL)
		return Ret_InvalidElement;
	int nRet;

	DWORD nBt =0;

	//1��дTagNumber
	BYTE *TempTagNum = (BYTE*) &pPixelEle->nTag;
	BYTE *TagNum = WritePara(pPixelEle,TempTagNum,4,bBigEndia,Para_TagNum);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,TagNum,4,&nBt,0);
	delete []TagNum;

	//2��VR
	pDcmVR pVR = CDcmVR::Find(pPixelEle->nVR);
	if (m_BaseFileInfo.nTsType != TS_IMPLICIT_VR_LITTLE_ENDIAN)
	{
		if (pPixelEle->nVR == VR_00)
			pPixelEle->nVR = VR_OW;
		BYTE *TempTagVR = (BYTE*)&pPixelEle->nVR;
		BYTE *TagVR = WritePara(pPixelEle,TempTagVR,pVR->nLenOfVrDes,false,Para_VR);
		::SetFilePointer(hf,0,NULL,FILE_END);
		::WriteFile(hf,TagVR,pVR->nLenOfVrDes,&nBt,0);
		delete []TagVR;
	}

	//3��LEN
	BYTE *TempTagLen = (BYTE *) &pPixelEle->nLen;
	BYTE *TagLen = WritePara(pPixelEle,TempTagLen,pVR->nLenOfLenDes,bBigEndia,Para_Len);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,TagLen,pVR->nLenOfLenDes,&nBt,0);
	delete []TagLen;

	// ֵ��
	pHsElement p;	
	ImageInfo ImgInfo;

	//nSamplePerPixel
	p = Hs_FindFirstEle(pPixelEle,TAG_SAMPLES_PER_PIXEL,true);
	nRet = Hs_GetLongValue(p,ImgInfo.nSamplePerPixel,0);
	if(ImgInfo.nSamplePerPixel==0)
		ImgInfo.nSamplePerPixel = 1;

	//nBitsAllocated
	p = Hs_FindFirstEle(pPixelEle,TAG_BITS_ALLOCATED,true);
	nRet = Hs_GetLongValue(p,ImgInfo.nBitsAllocated,0);

	FILE *m_File;
	QByteArray ba = m_BaseFileInfo.sFileName.toLatin1();
	errno_t err = fopen_s(&m_File,ba.data(),"rb");

	fseek(m_File,0,SEEK_END);
	int iFileEndPos = ftell(m_File);
	int iPerByte = iFileEndPos - pPixelEle->nTagPos-pPixelEle->nOffset;
	BYTE *pPixel = new BYTE[iPerByte];


	fseek(m_File,pPixelEle->nTagPos+pPixelEle->nOffset,SEEK_SET); 
	size_t nReadCount = fread(pPixel, iPerByte, 1,m_File);
	BYTE *pFinalPixel = WriteImage(ImgInfo,pPixel,iPerByte,bConvert);
	::SetFilePointer(hf,0,NULL,FILE_END);
	::WriteFile(hf,pFinalPixel,iPerByte,&nBt,0);
	delete []pFinalPixel;

	fclose(m_File);
	delete []pPixel;

	return Ret_Success;
}
/*
����TagNum��С�˴洢��ʽ��
1.����CPUΪС�ˣ�����DICOM�ļ�ΪС�ˣ��Ƚ�4���ֽ�ȫ���ߵ����ٷֱ�ǰ�����ֽڣ��������ֽڵߵ�;
2.����CPUΪС�ˣ�����DICOM�ļ�Ϊ��ˣ���4���ֽ�ȫ���ߵ�;
3.����CPUΪ��ˣ�����DICOM�ļ�ΪС�ˣ��ֱ�ǰ�����ֽڣ��������ֽڵߵ�;
4.����CPUΪ��ˣ�����DICOM�ļ�Ϊ��ˣ������κε���;

����TagVR�����ص㣺
������Ϊ�ĸ��ֽ�ʱ�����ִ�С�ˣ�ȫΪ"UL0000"����;
*/

BYTE *CHsBaseFile::WritePara(pHsElement pEle,BYTE *pData,int iDataSize,bool bBigEndia,int iPareType)
{
	BYTE *pResult = new BYTE[iDataSize];

	if (iPareType == Para_TagNum)
	{
		if (m_BaseFileInfo.bCpuBigEndia == false)
		{
			if (bBigEndia == false)
			{
				BYTE temp[4];
				temp[0] = pData[3]; temp[1] = pData[2]; temp[2] = pData[1]; temp[3] = pData[0];
				pResult[0] = temp[1]; pResult[1] = temp[0]; pResult[2] = temp[3]; pResult[3] = temp[2];
			}
			else
			{
				if (pEle->nTag!=0x00020000UL && pEle->nTag!=0x00020102UL && pEle->nTag!=0x00020001UL && pEle->nTag!=0x00020002UL && pEle->nTag!=0x00020003UL && 
					pEle->nTag!=0x00020010UL && pEle->nTag!=0x00020012UL && pEle->nTag!=0x00020013UL && pEle->nTag!=0x00020016UL && pEle->nTag!=0x00020100UL )
				{
					pResult[0] = pData[3]; pResult[1] = pData[2]; pResult[2] = pData[1]; pResult[3] = pData[0];
				}
				else
				{
					BYTE temp[4];
					temp[0] = pData[3]; temp[1] = pData[2]; temp[2] = pData[1]; temp[3] = pData[0];
					pResult[0] = temp[1]; pResult[1] = temp[0]; pResult[2] = temp[3]; pResult[3] = temp[2];
				}
			}
		}
		else
		{
			if (bBigEndia == false)
			{
				pResult[0] = pData[3]; pResult[1] = pData[2]; pResult[2] = pData[1]; pResult[3] = pData[0];
			}
			else
			{
				if (pEle->nTag!=0x00020000UL && pEle->nTag!=0x00020102UL && pEle->nTag!=0x00020001UL && pEle->nTag!=0x00020002UL && pEle->nTag!=0x00020003UL && 
					pEle->nTag!=0x00020010UL && pEle->nTag!=0x00020012UL && pEle->nTag!=0x00020013UL && pEle->nTag!=0x00020016UL && pEle->nTag!=0x00020100UL )
				{
					pResult[0] = pData[3]; pResult[1] = pData[2]; pResult[2] = pData[1]; pResult[3] = pData[0];
				}
				else
				{
					pResult = pData;
				}
			}
		}
	}
	else if (iPareType == Para_VR)
	{
		BYTE temp[4];
		memset(temp,0,4);
		if (m_BaseFileInfo.bCpuBigEndia == false)
		{
			temp[0] = pData[1]; temp[1] = pData[0] ;
		}
		else
		{
			temp[0] = pData[2]; temp[1] = pData[3];
		}
		if (iDataSize == 2)
		{
			pResult[0] = temp[0]; pResult[1] = temp[1];
		}
		else
		{
			pResult[0] = temp[0]; pResult[1] = temp[1];pResult[2] = temp[2]; pResult[3] = temp[3];
		}
	}
	else if (iPareType == Para_Len)
	{
		BYTE temp[4];
		memset(temp,0,4);
		if (bBigEndia == false)
		{
			if (m_BaseFileInfo.bCpuBigEndia == false)
			{
				temp[0] = pData[0]; temp[1] = pData[1]; temp[2] = pData[2]; temp[3] = pData[3];
			}				
			else
			{
				temp[0] = pData[3]; temp[1] = pData[2]; temp[2] = pData[1]; temp[3] = pData[0];			
			}	
			if(iDataSize == 2)
			{
				pResult[0] = temp[0]; pResult[1] = temp[1];
			}
			else
			{
				pResult[0] = temp[0]; pResult[1] = temp[1];pResult[2] = temp[2]; pResult[3] = temp[3];
			}
		}
		else
		{
			if (m_BaseFileInfo.bCpuBigEndia == false)
			{
				temp[0] = pData[3]; temp[1] = pData[2]; temp[2] = pData[1]; temp[3] = pData[0];
			}
			else
			{
				temp[0] = pData[0]; temp[1] = pData[1]; temp[2] = pData[2]; temp[3] = pData[3];
			}
			if (iDataSize == 2)
			{
				if (pEle->nTag!=0x00020000UL && pEle->nTag!=0x00020102UL && pEle->nTag!=0x00020001UL && pEle->nTag!=0x00020002UL && pEle->nTag!=0x00020003UL && 
					pEle->nTag!=0x00020010UL && pEle->nTag!=0x00020012UL && pEle->nTag!=0x00020013UL && pEle->nTag!=0x00020016UL && pEle->nTag!=0x00020100UL )
				{
					pResult[0] = temp[2]; pResult[1] = temp[3];
				}
				else
				{
					pResult[0] = temp[3]; pResult[1] = temp[2];
				}
			}
			else
			{
				if (pEle->nTag!=0x00020000UL && pEle->nTag!=0x00020102UL && pEle->nTag!=0x00020001UL && pEle->nTag!=0x00020002UL && pEle->nTag!=0x00020003UL && 
					pEle->nTag!=0x00020010UL && pEle->nTag!=0x00020012UL && pEle->nTag!=0x00020013UL && pEle->nTag!=0x00020016UL && pEle->nTag!=0x00020100UL )
				{
					pResult[0] = temp[0]; pResult[1] = temp[1];pResult[2] = temp[2]; pResult[3] = temp[3];
				}
				else
				{
					pResult[0] = temp[3]; pResult[1] = temp[2]; pResult[2] = temp[1]; pResult[3] =temp[0]; 
				}

			}		
		}
	}
	return pResult;
}

BYTE *CHsBaseFile::WriteValue(pHsElement pEle,BYTE *pData, int iDataSize,bool bConvert)
{
	BYTE *pResult = new BYTE[iDataSize];	
	if (bConvert != false)
	{
		if (pEle->nTag==0x00020000UL || pEle->nTag==0x00020102UL || pEle->nTag==0x00020001UL || pEle->nTag==0x00020002UL || pEle->nTag==0x00020003UL || 
			pEle->nTag==0x00020010UL || pEle->nTag==0x00020012UL || pEle->nTag==0x00020013UL || pEle->nTag==0x00020016UL || pEle->nTag==0x00020100UL )
		{	
			memcpy(pResult,pData,iDataSize);
		}
		else
		{
			if (pEle->nVR == VR_SL  || pEle->nVR == VR_UL || pEle->nVR == VR_FL || pEle->nVR == VR_OF)
			{
				int iFunState;                            
				unsigned long nValueCount = Hs_GetValueCount(pEle,iFunState);
				for (int i = 0; i<nValueCount;i++)
				{
					for (int j = 0; j<4;j++)
					{
						pResult[i*4 +j] = pData[(i+1)*4-1-j];
					}
				}
			}
			else if(pEle->nVR == VR_SS||pEle->nVR == VR_US||pEle->nVR == VR_OW)
			{
				int iFunState;                            
				unsigned long nValueCount = Hs_GetValueCount(pEle,iFunState);
				for (int i = 0; i<nValueCount;i++)
				{
					for (int j = 0; j<2;j++)
					{
						pResult[i*2 +j] = pData[(i+1)*2-1-j];
					}
				}
			}
			else if (pEle->nVR ==VR_FD)
			{
				int iFunState;                            
				unsigned long nValueCount = Hs_GetValueCount(pEle,iFunState);
				for (int i = 0; i<nValueCount;i++)
				{
					for (int j = 0; j<8;j++)
					{
						pResult[i*8 +j] = pData[(i+1)*8-1-j];
					}
				}
			}
			//else if (pEle->nVR == VR_AT)
			//{
			//	pResult[0] = pData[1]; pResult[1] = pData[0]; pResult[2] = pData[3]; pResult[3] = pData[2]; 
			//}
			else
			{
				memcpy(pResult,pData,iDataSize);
			}
		}
	}		
	else
	{
		memcpy(pResult,pData,iDataSize);
	}
	//TRACE("\r\n %d %d %d %d",pResult[0],pResult[1],pResult[2],pResult[3]);
	return pResult;
}

BYTE *CHsBaseFile::WriteImage(ImageInfo ImgInfo,BYTE *pImageData,int iImageByte,bool bConvert)
{
	BYTE *pResult = new BYTE[iImageByte];

	int	iPerPixelByte = ImgInfo.nBitsAllocated/ImgInfo.nSamplePerPixel/8;


	if (bConvert != false)
	{
		if (iPerPixelByte == 1)
		{
			memcpy(pResult,pImageData,iImageByte);
		}
		else if (iPerPixelByte == 2)
		{
			int iPixelNum = iImageByte/iPerPixelByte;

			for (int i=0; i<iPixelNum; i++)
			{
				pResult[i*iPerPixelByte] = pImageData[i*iPerPixelByte+1];pResult[i*iPerPixelByte+1] = pImageData[i*iPerPixelByte];
			}
		}
		else if (iPerPixelByte == 4)
		{
			int iPixelNum = iImageByte/iPerPixelByte;

			for (int i=0; i<iPixelNum; i++)
			{
				pResult[i*iPerPixelByte] = pImageData[i*iPerPixelByte+3];pResult[i+iPerPixelByte+1] = pImageData[i*iPerPixelByte+2];
				pResult[i*iPerPixelByte+2] = pImageData[i*iPerPixelByte+1];pResult[i+iPerPixelByte+3] = pImageData[i*iPerPixelByte];
			}
		}
	}
	else
	{
		memcpy(pResult,pImageData,iImageByte);
	}

	return pResult;
}


int CHsBaseFile::Hs_DeleteValues(pHsElement pEle,int nIndex)
{
	if (pEle == NULL)
		return Ret_InvalidPara;
	if(nIndex<0)
		return Ret_OutOfValueCount;

	int nRet = 0;
	unsigned long nValueCount = Hs_GetValueCount(pEle,nRet);

	if (nValueCount == 0)
	{
		return Ret_OutOfValueCount;
	}

	//if (nIndex+1-nValueCount>0)
	//	nIndex = nValueCount-1;

	if (nValueCount == 1&& nIndex==0)
	{
		if (pEle->pValue)
		{
			delete pEle->pValue;
			pEle->pValue = NULL;
		}
		pEle->pValue = new BYTE[0];
		pEle->nLen = 0;
	}
	else
	{
		BYTE *pNewValue;
		if (pEle->nVR ==VR_UL || pEle->nVR == VR_FL || pEle->nVR == VR_FD || pEle->nVR == VR_SS || pEle->nVR == VR_SL || pEle->nVR == VR_US || pEle->nVR == VR_AT)
		{
			int iTypeSize = CDcmVR::Find(pEle->nVR)->nValueLeng;
			pNewValue = new BYTE[(nValueCount-1)*iTypeSize];

			//if (pEle->pValue == NULL)//δ�޸�
			//	memcpy(pNewValue,&(m_buf[pEle->nTagPos+pEle->nOffset]),(nIndex)*iTypeSize);
			//else//���޸�
			memcpy(pNewValue,pEle->pValue,nIndex*iTypeSize);

			pNewValue += nIndex*iTypeSize;

			//if (pEle->pValue == NULL)//δ�޸�
			//	memcpy(pNewValue,&(m_buf[pEle->nTagPos+pEle->nOffset+(nIndex)*iTypeSize + iTypeSize]),pEle->nLen  - (nIndex+1)*iTypeSize);
			//else//���޸�
			memcpy(pNewValue,pEle->pValue+(nIndex)*iTypeSize + iTypeSize,pEle->nLen  - (nIndex+1)*iTypeSize);

			pEle->nLen = pEle->nLen - iTypeSize;
			pNewValue -= nIndex*iTypeSize;

			if (pEle->pValue!=NULL)
			{
				delete pEle->pValue;
				pEle->pValue = NULL;
			}	
			pEle->pValue = new BYTE[pEle->nLen];
			memcpy(pEle->pValue,pNewValue,pEle->nLen);
			delete []pNewValue;
		}
		else
		{	
			QVector<QString> OldValues;
			QVector<int> ValueLen;
			for (int i = 0; i<nValueCount; i++)
			{
				QString Value;
				Hs_GetStringValue(pEle,Value,i);
				OldValues.push_back(Value);
				int Len = Value.length();
				ValueLen.push_back(Len);
			}		

			int iPreLen = 0;

			for (int i = 0; i<nIndex;i++)
			{
				iPreLen = iPreLen + ValueLen[i] + 1;
			}

			pNewValue = new BYTE[pEle->nLen - ValueLen[nIndex]];

			if (nIndex == 0)
			{				
				//if (pEle->pValue == NULL)//δ�޸�
				//	memcpy(pNewValue,&(m_buf[pEle->nTagPos+pEle->nOffset+ValueLen[nIndex]+1]),pEle->nLen-ValueLen[nIndex]-1);
				//else//���޸�
				memcpy(pNewValue,pEle->pValue+ValueLen[nIndex]+1,pEle->nLen-ValueLen[nIndex]-1);
			}
			else
			{
				//if (pEle->pValue == NULL)//δ�޸�
				//	memcpy(pNewValue,&(m_buf[pEle->nTagPos+pEle->nOffset]),iPreLen-1);
				//else//���޸�
				memcpy(pNewValue,pEle->pValue,iPreLen-1);

				if (nIndex < nValueCount-1)				
				{
					pNewValue += iPreLen-1;

					//if (pEle->pValue == NULL)//δ�޸�
					//	memcpy(pNewValue,&(m_buf[pEle->nTagPos+pEle->nOffset+iPreLen+ValueLen[nIndex]+1]),pEle->nLen - iPreLen - ValueLen[nIndex]-1);
					//else//���޸�
					memcpy(pNewValue,pEle->pValue+iPreLen+ValueLen[nIndex]+1,pEle->nLen - iPreLen - ValueLen[nIndex]-1);
				}				
			}
			pEle->nLen = pEle->nLen - ValueLen[nIndex] -1;

			if (pEle->pValue!=NULL)
			{
				delete pEle->pValue;
				pEle->pValue = NULL;
			}

			if (pEle->nLen%2 != 0)
				pEle->nLen = pEle->nLen + 1;

			pEle->pValue = new BYTE[pEle->nLen];
			memset(pEle->pValue,0x00,pEle->nLen);
			memcpy(pEle->pValue,pNewValue,pEle->nLen);
			delete []pNewValue;			
		}				
	}

	pEle->bNewTag = true;

	return 0;
}

unsigned long CHsBaseFile::Hs_GetSequenceItemEle( pHsElement pEle,QVector<pHsElement> &ItemEleV )
{
	if (m_pMainEle == NULL || pEle == NULL)
		return 0;

	if(pEle->nVR != VR_SQ)
		return 0;

	int n = int(pEle->pChildEleV.size());
	for (int i=0; i<n; i++)
	{
		if (pEle->pChildEleV[i]->nTag == TAG_ITEM)
		{
			ItemEleV.push_back(pEle->pChildEleV[i]);
		}
	}

	return unsigned long(ItemEleV.size());
}



bool CHsBaseFile::Hs_GetLutDataItem( pHsElement pLutItemEle,LutData &lut )
{//����һ������Lut�������ITEM���Ұ�Lut����lut
	if (m_pMainEle == NULL || pLutItemEle == NULL)
		return false;

	int nRet = 0;
	//Lut Descriptor�е���ҪԱ,û˭������
	pHsElement pLutDes = Hs_GetChildEle(pLutItemEle,TAG_LUT_DESCRIPTOR);
	nRet = Hs_GetLongValue(pLutDes,lut.nLutLen,0);//Lut�೤? �����ٸ�Data?
	if(nRet)
		return false;

	nRet = Hs_GetLongValue(pLutDes,lut.nMinValue,1);//��Сֵ
	if(nRet)
		return false;

	nRet = Hs_GetLongValue(pLutDes,lut.nBitsPerData,2);//Lut��ÿ��Data����ռ��λ
	if(nRet)
		return false;

	//�����Lut������
	pHsElement pExplan = Hs_GetChildEle(pLutItemEle,TAG_LUT_EXPLANATION);
	nRet = Hs_GetStringValue(pExplan,lut.sName,0);
	if(lut.sName.isEmpty())
	{
		char cName[200] = {'\0'};
		sprintf(cName,"Lut:%d",lut.iLutID);
		lut.sName = cName;
	}
	else
	{
		char cName[200] = {'\0'};
		QByteArray ba = lut.sName.toLatin1();
		sprintf(cName,"Lut:%s",ba.data());
		lut.sName = cName;
	}

	//Lut��������
	lut.nBytePerData = lut.nBitsPerData/8 + (lut.nBitsPerData%8 > 0 ? 1:0);

	unsigned long nSize = lut.nLutLen * lut.nBytePerData ;//Ԥ��Ӧ�г���
	if(nSize==0)
		return false;

	pHsElement pLutDataEle = Hs_GetChildEle(pLutItemEle,TAG_LUT_DATA);
	unsigned long nSizeGot = 0;
	lut.pLutData = Hs_GetByteValue(pLutDataEle,nSizeGot,nRet);

	if(nRet)
		return false;

	if(nSizeGot!=nSize)//Ԥ�Ƴ�����ʵ�ʳ��Ȳ�����
		return false;

	//����Ǵ����Ҫÿ���ڵ������ֽ���Ҫ����
	if (pLutDataEle->bBigEndian)
	{
		int nTime = nSizeGot/2;
		for (int e=0;e<nTime;e++)
		{
			std::swap(lut.pLutData[2*e],lut.pLutData[2*e+1]);
		}
	}

	lut.nMaxValue = lut.nMinValue + lut.nLutLen - 1;

	return true;
}


bool CHsBaseFile::Hs_GetWcLutItem( pHsElement pLutItemEle,QVector<LutData*> &LutV )
{//ĳTag:pLutItemEle�º���������Tag��TAG_WINDOW_CENTER  TAG_WINDOW_WIDTH 0x00281055������ʵ�γ���һ��Wl���У���Ϊ������tag�¶����Զ�ֵ

	if (m_pMainEle == NULL)
		return false;

	pHsElement pWinCenter = Hs_GetChildEle(pLutItemEle,TAG_WINDOW_CENTER);
	pHsElement pWinWidth = Hs_GetChildEle(pLutItemEle,TAG_WINDOW_WIDTH);
	pHsElement pWinExplan = Hs_GetChildEle(pLutItemEle,TAG_WINDOW_CENTER_WIDTH_EXPLANATION);

	int nRet = 0;

	if (pWinCenter)
	{
		int i = -1;
		while(1)
		{
			i++;
			LutData *pNewWcLut = new LutData;
			pNewWcLut->bModality = false;
			pNewWcLut->bWc = true;
			pNewWcLut->iLutID = i;
			nRet = Hs_GetDoubleValue(pWinCenter,pNewWcLut->nC,i);
			if(nRet)
			{
				delete pNewWcLut;
				break;
			}

			nRet = Hs_GetDoubleValue(pWinWidth,pNewWcLut->nW,i);
			if(nRet)
			{
				delete pNewWcLut;
				break;
			}

			nRet = Hs_GetStringValue(pWinExplan,pNewWcLut->sName,i);
			if(pNewWcLut->sName.isEmpty())
			{
				char cName[100];
				sprintf(cName,"WL:%d",i+1);
				pNewWcLut->sName = cName;
			}
			else
			{
				char cName[200] = {'\0'};
				QByteArray ba = pNewWcLut->sName.toLatin1();
				sprintf(cName,"WL:%s",ba.data());
				pNewWcLut->sName = QString(QLatin1String(cName));
			}

			LutV.push_back(pNewWcLut);
		}
	}
//	�˺������������Ӧ�õ�CHsFile::GetLuts;
	return true;
}

int CHsBaseFile::Hs_GetFrameCount(pHsElement pPixEle)
{
	long nFrame = 0;
	pHsElement p = Hs_FindFirstEle(pPixEle,TAG_NUMBER_OF_FRAMES,true);
	if(p)//����Ƕ�֡�����������Tag,��֡�Ŀ���û��
	{
		Hs_GetLongValue(p,nFrame,0);
	}
	else
	{
		nFrame = 1;
	}

	m_BaseFileInfo.nFrame = nFrame;

	return nFrame;
}

pHsElement CHsBaseFile::GetItemFromPerFramFunGpSeq( pHsElement pFrmFunGpSeqEle,int iFrame )
{
	if(pFrmFunGpSeqEle == NULL || m_pMainEle == NULL)
		return NULL;

	QVector<pHsElement> ItemEleV;
	int nItem = Hs_GetSequenceItemEle(pFrmFunGpSeqEle,ItemEleV);
	if(nItem==0)
		return NULL;

	if(iFrame>=0 && iFrame<nItem)
		return ItemEleV[iFrame];

	return NULL;
	//for (int i=0;i<nItem;i++)
	//{
	//	pHsElement pItemEle = ItemEleV[i];
	//	pHsElement pFrmConSeqEle = Hs_GetChildEle(pItemEle,0x00209111,-1);
	//	if(pFrmConSeqEle==NULL)
	//		return NULL;

	//	pHsElement tItemEle = Hs_GetChildEle(pFrmConSeqEle,TAG_ITEM,-1);
	//	if(tItemEle==NULL)
	//		return NULL;

	//	pHsElement pInStackFrameNumEle = Hs_GetChildEle(tItemEle,0x00209057,-1);
	//	if(pInStackFrameNumEle==NULL)
	//		return NULL;

	//	long tFrame = -1;
	//	if(Hs_GetLongValue(pInStackFrameNumEle,tFrame,0) != Ret_Success)
	//		return NULL;

	//	if(tFrame-1 == iFrame)
	//	{
	//		return pItemEle;
	//	}
	//}

	return NULL;
}

bool CHsBaseFile::GetImageLocParaMulityFrame( int iFrame,ImageInfo &info )
{
	pHsElement pFunGpSeqEle = Hs_FindFirstEle(NULL,0x52009230,true);
	if(pFunGpSeqEle==NULL)
		return false;

	pHsElement pItemEle = GetItemFromPerFramFunGpSeq(pFunGpSeqEle,iFrame);

	//Pixel Spacing 
	pHsElement pPixMeasureSeqEle = Hs_GetChildEle(pItemEle,0x00289110,-1);
	if(pPixMeasureSeqEle==NULL)
		return false;

	pHsElement tItemEle0 = Hs_GetChildEle(pPixMeasureSeqEle,TAG_ITEM,-1);
	if(tItemEle0 == NULL)
		return false;

	pHsElement pPixSpacingEle = Hs_GetChildEle(tItemEle0,TAG_PIXEL_SPACING,true);

	if(Hs_GetDoubleValue(pPixSpacingEle,info.ImgLocPara.fPixSpacingY,0)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pPixSpacingEle,info.ImgLocPara.fPixSpacingX,1)!=Ret_Success) 
		return false;

	// TAG_IMAGE_POSITION_PATIENT 0x00200032
	pHsElement pPlanePosSeqEle = Hs_GetChildEle(pItemEle,0x00209113,-1);
	if(pPlanePosSeqEle==NULL)
		return false;

	pHsElement tItemEle1 = Hs_GetChildEle(pPlanePosSeqEle,TAG_ITEM,-1);
	if(tItemEle1 == NULL)
		return false;

	pHsElement pPosEle = Hs_GetChildEle(tItemEle1,TAG_IMAGE_POSITION_PATIENT,-1);

	if(Hs_GetDoubleValue(pPosEle,info.ImgLocPara.fOriLeftTopPixX,0)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pPosEle,info.ImgLocPara.fOriLeftTopPixY,1)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pPosEle,info.ImgLocPara.fOriLeftTopPixZ,2)!=Ret_Success) 
		return false;

	info.ImgLocPara.fLeftTopPixX = info.ImgLocPara.fOriLeftTopPixX;
	info.ImgLocPara.fLeftTopPixY = info.ImgLocPara.fOriLeftTopPixY;
	info.ImgLocPara.fLeftTopPixZ = info.ImgLocPara.fOriLeftTopPixZ;

	// TAG_IMAGE_ORIENTATION_PATIENT 0x00200037UL
	pHsElement pPlaneOriSeqEle = Hs_GetChildEle(pItemEle,0x00209116,-1);
	if(pPlaneOriSeqEle==NULL)
		return false;

	pHsElement tItemEle2 = Hs_GetChildEle(pPlaneOriSeqEle,TAG_ITEM,-1);
	if(tItemEle2==NULL)
		return false;
	pHsElement pOriEle = Hs_GetChildEle(tItemEle2,TAG_IMAGE_ORIENTATION_PATIENT,-1);

	if(Hs_GetDoubleValue(pOriEle,info.ImgLocPara.fOriFirstRowCosX,0)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pOriEle,info.ImgLocPara.fOriFirstRowCosY,1)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pOriEle,info.ImgLocPara.fOriFirstRowCosZ,2)!=Ret_Success) 
		return false;

	if(Hs_GetDoubleValue(pOriEle,info.ImgLocPara.fOriFirstColCosX,3)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pOriEle,info.ImgLocPara.fOriFirstColCosY,4)!=Ret_Success) 
		return false;
	if(Hs_GetDoubleValue(pOriEle,info.ImgLocPara.fOriFirstColCosZ,5)!=Ret_Success) 
		return false;

	info.ImgLocPara.fFirstRowCosX = info.ImgLocPara.fOriFirstRowCosX;
	info.ImgLocPara.fFirstRowCosY = info.ImgLocPara.fOriFirstRowCosY;
	info.ImgLocPara.fFirstRowCosZ = info.ImgLocPara.fOriFirstRowCosZ;

	info.ImgLocPara.fFirstColCosX = info.ImgLocPara.fOriFirstColCosX;
	info.ImgLocPara.fFirstColCosY = info.ImgLocPara.fOriFirstColCosY;
	info.ImgLocPara.fFirstColCosZ = info.ImgLocPara.fOriFirstColCosZ;

	info.ImgLocPara.bValide = true;

	info.ImgLocPara.fRowmm = info.ImgLocPara.nRow * info.ImgLocPara.fPixSpacingY;
	info.ImgLocPara.fColmm = info.ImgLocPara.nCol * info.ImgLocPara.fPixSpacingX;

	if (info.fPixelSpaceX < 0.000001)
		info.fPixelSpaceX = info.ImgLocPara.fPixSpacingX;
	if (info.fPixelSpaceY < 0.000001)
		info.fPixelSpaceY = info.ImgLocPara.fPixSpacingY;

	/// Image SliceThick
	pHsElement pSliceThickEle = Hs_GetChildEle(tItemEle0,TAG_SLICE_THICKNESS,true);

	if (Hs_GetDoubleValue(pSliceThickEle,info.ImgLocPara.fSliceThickness,0) != Ret_Success)
	{
		pHsElement pPhilpPriEle = Hs_GetChildEle(pItemEle,0x2005140f,-1);//�����ֶ�֡˽��tag
		if(pPhilpPriEle==NULL)
			return false;

		pHsElement tPriItemEle = Hs_GetChildEle(pPhilpPriEle,TAG_ITEM,-1);
		if (tPriItemEle == NULL)
			return false;

		pSliceThickEle = Hs_GetChildEle(tPriItemEle,TAG_SLICE_THICKNESS,true);

		if (Hs_GetDoubleValue(pSliceThickEle,info.ImgLocPara.fSliceThickness,0) != Ret_Success)
			return false;
	}
	//��λ��Ҳ��
	//Hs_GetDoubleValueA(TAG_SLICE_LOCATION,info.ImgLocPara.fSliceLoction);

	return true;
}

int CHsBaseFile::Hs_CopyRootTagTo(CHsBaseFile* pDstDs, unsigned long nTag)// ,pHsElement* pDstParentEle)
{
	if(nTag == 0 || pDstDs == NULL)
		return Ret_InvalidPara;

	if(nTag == TAG_ITEM)
		return Ret_UnSupportPara;

	pHsElement pSrcEle = Hs_FindFirstEle(NULL,nTag,true);
	if(pSrcEle==NULL)
		return Ret_Success;

	if(pSrcEle->nVR == VR_SQ || pSrcEle->pChildEleV.empty()==false)
		return Ret_UnSupportPara;

	int nRet = 0;
	//����Ŀ�ĵ��Ƿ��Ѵ������Tag---���޸�Ŀ¼����
	pHsElement pDstEle = NULL;

	pDstEle = pDstDs->Hs_FindFirstEle(NULL,nTag,true);
	if(pDstEle == NULL)
		pDstEle = pDstDs->Hs_InsertElement(NULL,true,pSrcEle->nTag,CDcmTag::Find(pSrcEle->nTag)->nVR,pSrcEle->bSquence,0,nRet);

	if(pDstEle == NULL)
		return nRet;

	unsigned long  nByte = 0;
	BYTE* pByte = Hs_GetByteValue(pSrcEle,nByte,nRet);
	if(pByte)
	{
		pDstDs->Hs_SetByteValue(pDstEle,pByte,nByte);
		delete []pByte;
	}

	return Ret_Success;

}
int CHsBaseFile::Hs_SetByteValue( pHsElement pEle,BYTE *pValue,int nCount)
{
	if (pEle == NULL )
		return Ret_InvalidPara;

	if(pEle->pValue)
	{
		delete [](pEle->pValue);
		pEle->pValue = NULL;
		pEle->nLen = 0;
	}

	if (pValue && nCount > 0)
	{
		pEle->pValue = new BYTE[nCount];
		memcpy(pEle->pValue,pValue,nCount);
		pEle->nLen = nCount;
	}

	pEle->bNewTag = TRUE;
	return Ret_Success;
}

int CHsBaseFile::Hs_InitFile(QString StorageClassUID, QString StorageInstancUID, int nTsType)
{
	if (m_pMainEle == NULL)
	{
		m_pMainEle = new HsElement;
	}

	int nRet;

	//
	pHsElement pInsertEle = Hs_InsertElement(NULL,false,TAG_FILE_META_INFORMATION_VERSION,VR_OB,false,0,nRet);
	if (pInsertEle == NULL)
		return Ret_InvalidPara;
	Hs_SetShortValue(pInsertEle,01,true,0);
	Hs_SetShortValue(pInsertEle,00,true,1);

	//
	pInsertEle = Hs_InsertElement(NULL,false,TAG_MEDIA_STORAGE_SOP_CLASS_UID,CDcmTag::Find(TAG_MEDIA_STORAGE_SOP_CLASS_UID)->nVR,false,0,nRet);
	if (pInsertEle == NULL)
		return Ret_InvalidPara;
	Hs_SetStringValue(pInsertEle,StorageClassUID,false,0);

	//
	pInsertEle = Hs_InsertElement(NULL,false,TAG_MEDIA_STORAGE_SOP_INSTANCE_UID,CDcmTag::Find(TAG_MEDIA_STORAGE_SOP_INSTANCE_UID)->nVR,false,0,nRet);
	if (pInsertEle == NULL)
		return Ret_InvalidPara;
	Hs_SetStringValue(pInsertEle,StorageInstancUID,false,0);

	//
	pInsertEle = Hs_InsertElement(NULL,false,TAG_TRANSFER_SYNTAX_UID,CDcmTag::Find(TAG_TRANSFER_SYNTAX_UID)->nVR,false,0,nRet);
	if (pInsertEle == NULL)
		return Ret_InvalidPara;

	QString sTsTypeName;
	if (nTsType == TS_IMPLICIT_VR_LITTLE_ENDIAN)
	{	
		sTsTypeName= "1.2.840.10008.1.2";
	}
	else if (nTsType == TS_EXPLICIT_VR_LITTLE_ENDIAN)
	{
		sTsTypeName= "1.2.840.10008.1.2.1";
	}
	else if (nTsType == TS_EXPLICIT_VR_BIG_ENDIAN)
	{
		sTsTypeName= "1.2.840.10008.1.2.2";
	}
	else if (nTsType == TS_RLE_LOSSLESS)
	{
		sTsTypeName= "1.2.840.10008.1.2.5";
	}
	else if (nTsType == TS_JPEG_BASELINE_1)
	{
		sTsTypeName= "1.2.840.10008.1.2.4.50";
	}
	else if (nTsType == TS_JPEG_EXTENDED_2_4)
	{
		sTsTypeName= "1.2.840.10008.1.2.4.51";
	}
	else if (nTsType == TS_JPEG_LOSSLESS_NONHIER_14)
	{
		sTsTypeName= "1.2.840.10008.1.2.4.57";
	}
	else if (nTsType == TS_JPEG_LOSSLESS_NONHIER_14B)
	{
		sTsTypeName= "1.2.840.10008.1.2.4.70";
	}
	else if (nTsType == TS_JPEG2000_LOSSLESS_ONLY)
	{
		sTsTypeName= "1.2.840.10008.1.2.4.90";
	}
	else if (nTsType == TS_JPEG2000)
	{
		sTsTypeName="1.2.840.10008.1.2.4.91";
	}
	else
	{
		sTsTypeName="1.2.840.10008.1.2";
	}
	Hs_SetStringValue(pInsertEle,sTsTypeName,false,0);


	//
	pInsertEle = Hs_InsertElement(NULL,false,TAG_IMPLEMENTATION_CLASS_UID,VR_UI,false,0,nRet);
	if (pInsertEle == NULL)
		return Ret_InvalidPara;
	QString sImplementationUID;
	sImplementationUID = "1.2.840.49110116101.99104";
	Hs_SetStringValue(pInsertEle,sImplementationUID,false,0);

	//
	pInsertEle = Hs_InsertElement(NULL,false,TAG_IMPLEMENTATION_VERSION_NAME,VR_SH,false,0,nRet);
	if (pInsertEle == NULL)
		return Ret_InvalidPara;
	QString sImplementationVersion;
	sImplementationVersion = "1";
	Hs_SetStringValue(pInsertEle,sImplementationVersion,false,0);

	return 0;
}

int CHsBaseFile::CalcInformationLength( int nTsType )
{
	int nRet;

	pHsElement pTsType = Hs_FindFirstEle(NULL,TAG_TRANSFER_SYNTAX_UID,true);
	if (pTsType == NULL)
	{
		pTsType = Hs_InsertElement(NULL,false,TAG_TRANSFER_SYNTAX_UID,VR_UI,false,0,nRet);
		if (pTsType == NULL)
			return Ret_InvalidPara;
	}

	QString sTsTypeName;
	if (nTsType == TS_IMPLICIT_VR_LITTLE_ENDIAN)
	{	
		sTsTypeName= "1.2.840.10008.1.2";
	}
	else if (nTsType == TS_EXPLICIT_VR_LITTLE_ENDIAN)
	{
		sTsTypeName= "1.2.840.10008.1.2.1";
	}
	else if (nTsType == TS_EXPLICIT_VR_BIG_ENDIAN)
	{
		sTsTypeName= "1.2.840.10008.1.2.2";
	}
	else if (nTsType == TS_RLE_LOSSLESS)
	{
		sTsTypeName= "1.2.840.10008.1.2.5";
	}
	else if (nTsType == TS_JPEG_BASELINE_1)
	{
		sTsTypeName= "1.2.840.10008.1.2.4.50";
	}
	else if (nTsType == TS_JPEG_EXTENDED_2_4)
	{
		sTsTypeName= "1.2.840.10008.1.2.4.51";
	}
	else if (nTsType == TS_JPEG_LOSSLESS_NONHIER_14)
	{
		sTsTypeName= "1.2.840.10008.1.2.4.57";
	}
	else if (nTsType == TS_JPEG_LOSSLESS_NONHIER_14B)
	{
		sTsTypeName= "1.2.840.10008.1.2.4.70";
	}
	else if (nTsType == TS_JPEG2000_LOSSLESS_ONLY)
	{
		sTsTypeName= "1.2.840.10008.1.2.4.90";
	}
	else if (nTsType == TS_JPEG2000)
	{
		sTsTypeName="1.2.840.10008.1.2.4.91";
	}
	else
	{
		sTsTypeName="1.2.840.10008.1.2";
	}
	Hs_SetStringValue(pTsType,sTsTypeName,true,0);

	pHsElement pInsertEle = Hs_InsertElement(NULL,false,0x00020000,VR_UL,false,0,nRet);
	if (pInsertEle == NULL)
		return Ret_InvalidPara;	

	int n = m_pMainEle->pChildEleV.size();
	int nLength = 0;
	if (n>1)
	{	
		for (int i=0; i<n;i++)
		{
			if (m_pMainEle->pChildEleV[i]->nTag/65536 == 2 && m_pMainEle->pChildEleV[i]->nTag != 0x00020000)
			{
				nLength = nLength + m_pMainEle->pChildEleV[i]->nOffset + m_pMainEle->pChildEleV[i]->nLen;
			}
		}
	}
	else
	{
		nLength = 0;
	}

	Hs_SetLongValue(pInsertEle,nLength,true,0);

	return 0;
}

BYTE* CHsBaseFile::GetByteFromBuffer( pHsElement pEle, unsigned long &nValueLen, int &nRet )
{
	if(pEle == NULL)
	{
		nRet = Ret_InvalidPara;
		return NULL;
	}

	if(pEle->nLen==0)
	{
		nRet = Ret_NoValue;
		return NULL;
	}

	BYTE *pByte = new BYTE[pEle->nLen];
	memcpy(pByte,&(m_buf[pEle->nTagPos+pEle->nOffset]),pEle->nLen);

	nValueLen = pEle->nLen;
	nRet = Ret_Success;

	return pByte;
}

int CHsBaseFile::Hs_GetStringValueA( unsigned long uTag,QString &sRet, bool bRoot,int nValueIndex )
{
	pHsElement p = NULL;
	if(bRoot)
		p = Hs_FindFirstEle(NULL,uTag,true);
	else
		p = Hs_FindFirstEle(NULL,uTag,false);

	if (p == NULL)
		return Ret_TagNotFound;

	return Hs_GetStringValue(p,sRet,nValueIndex);
}

int CHsBaseFile::Hs_GetDateValueA( unsigned long uTag,HsDateTime &DateValue,bool bRoot, int nValueIndex )
{
	pHsElement p = NULL;
	if(bRoot)
		p = Hs_FindFirstEle(NULL,uTag,true);
	else
		p = Hs_FindFirstEle(NULL,uTag,false);

	if (p == NULL)
		return Ret_TagNotFound;

	return Hs_GetDateValue(p,DateValue,nValueIndex);
}

int CHsBaseFile::Hs_GetTimeValueA( unsigned long uTag, HsDateTime &TimeValue,bool bRoot, int nValueIndex)
{
	pHsElement p = NULL;
	if(bRoot)
		p = Hs_FindFirstEle(NULL,uTag,true);
	else
		p = Hs_FindFirstEle(NULL,uTag,false);

	if (p == NULL)
		return Ret_TagNotFound;

	return Hs_GetTimeValue(p,TimeValue,nValueIndex);
}

int CHsBaseFile::Hs_GetDateTimeValueA( unsigned long uTag, HsDateTime &DateTimeValue, bool bRoot/*=true*/, int nValueIndex/*=0*/ )
{
	pHsElement p = NULL;
	if(bRoot)
		p = Hs_FindFirstEle(NULL,uTag,true);
	else
		p = Hs_FindFirstEle(NULL,uTag,false);

	if (p == NULL)
		return Ret_TagNotFound;

	return Hs_GetDateTimeValue(p,DateTimeValue,nValueIndex);
}

int CHsBaseFile::Hs_GetAgeValueA( unsigned long uTag,int &nAge,char &cAgeType,bool bRoot)
{
	pHsElement p = NULL;
	if(bRoot)
		p = Hs_FindFirstEle(NULL,uTag,true);
	else
		p = Hs_FindFirstEle(NULL,uTag,false);

	if (p == NULL)
		return Ret_TagNotFound;

	return Hs_GetAgeValue(p,nAge,cAgeType);
}

int CHsBaseFile::Hs_GetLongValueA( unsigned long uTag, long &nValue,bool bRoot,int nValueIndex)
{
	pHsElement p = NULL;
	if(bRoot)
		p = Hs_FindFirstEle(NULL,uTag,true);
	else
		p = Hs_FindFirstEle(NULL,uTag,false);

	if (p == NULL)
		return Ret_TagNotFound;

	return Hs_GetLongValue(p,nValue,nValueIndex);
}

BYTE* CHsBaseFile::Hs_GetByteValueA( unsigned long uTag, unsigned long &nValueLen, int &nRet, bool bRoot/*=true*/ )
{
	pHsElement p = NULL;
	if(bRoot)
		p = Hs_FindFirstEle(NULL,uTag,true);
	else
		p = Hs_FindFirstEle(NULL,uTag,false);

	if (p == NULL)
	{
		nRet = Ret_TagNotFound;
		return NULL;
	}

	return Hs_GetByteValue(p,nValueLen,nRet);
}

int CHsBaseFile::Hs_GetDoubleValueA( unsigned long uTag, double &fVal,bool bRoot,int nValueIndex )
{
	pHsElement p = NULL;
	if(bRoot)
		p = Hs_FindFirstEle(NULL,uTag,true);
	else
		p = Hs_FindFirstEle(NULL,uTag,false);

	if (p == NULL)
		return Ret_TagNotFound;

	return Hs_GetDoubleValue(p,fVal,nValueIndex);
}

int CHsBaseFile::Hs_GetValueCountA( unsigned long uTag,int &nRet , bool bRoot)
{
	pHsElement p = NULL;
	if(bRoot)
		p = Hs_FindFirstEle(NULL,uTag,true);
	else
		p = Hs_FindFirstEle(NULL,uTag,false);

	if (p == NULL)
		return Ret_TagNotFound;

	return Hs_GetValueCount(p,nRet);
}

QString CHsBaseFile::Hs_GetConvertValueA( unsigned long uTag,int nValueIndex, int& nRet,bool bRoot)
{
	pHsElement p = NULL;
	if(bRoot)
		p = Hs_FindFirstEle(NULL,uTag,true);
	else
		p = Hs_FindFirstEle(NULL,uTag,false);

	if (p == NULL)
	{
		nRet = Ret_TagNotFound;
		return "";
	}

	return Hs_GetConvertValue(p,nValueIndex,nRet);
}

void CHsBaseFile::IsXRay()//�ǲ����շ�ͼ��
{
	if(m_pMainEle == NULL)
		return;

	QString sModality = m_sModality;
	if (sModality.compare("CT", Qt::CaseInsensitive) == 0 || //��ЩModality��ӹ���ɣ���Ϊ������ͼ�񣬲����շ�ͼ��
		sModality.compare("MR", Qt::CaseInsensitive) == 0 ||
		sModality.compare("PET",Qt::CaseInsensitive) == 0 ||
		sModality.compare("PT", Qt::CaseInsensitive) == 0 ||
		sModality.compare("MG",Qt::CaseInsensitive) == 0 ||
		sModality.compare("RF", Qt::CaseInsensitive) == 0)
	{
		m_bXRay = false;
		return;
	}

	//����֣��һ��Ժ��DSA��CTɨ�裬����Modality=="XA"����ͼ��ʵ������CTͼ�񣬶���ÿ��Ҳ��512K��ͼ�������ܴ�����ͼ�޶࣬�˴�ר�Ŵ���
	if (sModality.compare("XA", Qt::CaseInsensitive) == 0 || sModality.compare("DSA", Qt::CaseInsensitive) == 0)
	{
		//�в�����Ϊ������ͼ��
		pHsElement pThickEle = Hs_FindFirstEle(NULL,TAG_SLICE_THICKNESS,true);
		if(pThickEle)
		{
			m_bXRay = false;
			return;
		}

		//��ɫDSAͼ��Ҳ��Ϊ������ͼ�񣬲����շ�ͼ��
		QString sImgType = "";
		if(Hs_GetStringValueA(TAG_PHOTOMETRIC_INTERPRETATION,sImgType) == 0)
		{
			if (sImgType.indexOf("RGB", 0) >= 0 || sImgType.indexOf("Palette", 0) >= 0)
			{
				m_bXRay = false;
				return;
			}
		}
		
	}

	m_bXRay = true;
}

//void CHsFile::CopyElement( pHsElement pSrc,pHsElement pDst )
//{
//	if(pSrc == NULL || pDst == NULL)
//		return;
//
//	*pDst = *pSrc;
//
//	int n = pSrc->pChildEleV.size();
//	for (int i=0;i<n;i++)
//	{
//		pHsElement pNewChildEle = new HsElement;
//
//		CopyElement(pSrc->pChildEleV[i],pNewChildEle);
//		pDst->pChildEleV.push_back(pNewChildEle);
//		pNewChildEle->pParentEle = pDst;
//	}
//}
int CHsBaseFile::Hs_OffsetWndLevel( CHsBaseImg &Img )
{
   /* int ndv = CAppConfig::m_DevCfgV.size();
    if (ndv)
    {
        string sManu = "";
        Hs_GetStringValueA(0x00080070,sManu,true,0);

        string sSerial = "";
        Hs_GetStringValueA(0x00181000,sSerial,true,0);

        string sStaion = "";
        Hs_GetStringValueA(0x00081010,sStaion,true,0);

        for (int d=0;d<ndv;d++)
        {
            if (sManu.compare(CAppConfig::m_DevCfgV[d].sManuFacturer) == 0 &&
                sSerial.compare(CAppConfig::m_DevCfgV[d].sSerialNumber) == 0 &&
                sStaion.compare(CAppConfig::m_DevCfgV[d].sStationName) == 0 )
            {
                Img.m_ImgState.CurWc.x += CAppConfig::m_DevCfgV[d].w;
                Img.m_ImgState.CurWc.y  += CAppConfig::m_DevCfgV[d].l;
                if(CAppConfig::m_DevCfgV[d].bModalityLine == false)
                {
                    Img.m_ImgState.bUseSlope = false;
                }
                if (CAppConfig::m_DevCfgV[d].bModalityLUT == false)
                {
                    int nLut = Img.m_pLutV.size();
                    for (int L=0;L<nLut;L++)
                    {
                        if (Img.m_pLutV[L]->bModality)
                        {
                            delete Img.m_pLutV[L];
                            Img.m_pLutV.erase(Img.m_pLutV.begin() + L);
                            break;
                        }
                    }
                }
                break;
            }
        }
    }*/

    return 0;
}

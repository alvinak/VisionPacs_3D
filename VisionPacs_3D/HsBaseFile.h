#pragma once
//////////////////////////////////////////////////////////////////////////
//����Dicom�ļ�
//������Ҫ���ɾ���һ��m_pMainEle�����ڲ�ͨ��ChildV�����ļ�Ele�Ĳ�Σ�ÿ��ELe��ͨ��pPreEle��pNextEle����List�ṹ
//ֻҪ�������һ�㣬�Ϳ������������Լ�ϲ���Ļ�ȡEle�ĺ�����
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DcmVR.h"
#include "DcmTag.h"
#include "HsBaseImg.h"

template <class TP>	//���ڳ������ֵ���з������أ���������
int CorrectPixelValue(TP **pPixData,unsigned long nRow,unsigned long nCol,const TP nMinPixValue,const TP nMaxPixValue,TP nOffset)
{
	if(pPixData == NULL)
		return Ret_InvalidPara;

	for (unsigned long r = 0;r<nRow; r++)
	{
		for (unsigned long c=0;c<nCol; c++)
		{
			if(pPixData[r][c] > nMaxPixValue)
				pPixData[r][c] -= nOffset;

			if(pPixData[r][c] < nMinPixValue)
				pPixData[r][c] += nOffset;
		}
	}

	return 0;
}

template <class T>
int SetNumberValue(CHsFile* pDs, pHsElement pEle, T nValue,bool bCover,int nValueIndex)
{
	if(pEle == NULL || pDs == NULL)
		return Ret_InvalidPara;

	if (pEle->nVR == VR_FD || pEle->nVR == VR_DS)
		return pDs->Hs_SetDoubleValue(pEle,(double)(nValue*1.00),bCover,nValueIndex);

	if (pEle->nVR == VR_OF || pEle->nVR == VR_FL)
		return pDs->Hs_SetFloatValue(pEle,(float)(nValue*1.00),bCover,nValueIndex);

	if (pEle->nVR == VR_SS || pEle->nVR == VR_US || pEle->nVR == VR_OW || pEle->nVR == VR_OB)
		return pDs->Hs_SetShortValue(pEle,(short)nValue,bCover,nValueIndex);

	if (pEle->nVR == VR_AT || pEle->nVR == VR_IS || pEle->nVR == VR_SL || pEle->nVR == VR_UL )
		return pDs->Hs_SetLongValue(pEle,(long)nValue,bCover,nValueIndex);

	qDebug("\r\nSetTagValue:�޷������Tag���ͣ�����������������������������������������������������������");

	return Ret_InvalidPara;
}
class CHsBaseFile
{
public:
	CHsBaseFile(void);
	~CHsBaseFile(void);

	//Member:
private:
	FILE *m_fp;
	unsigned char* m_buf;

	long m_nBufLen;//m_buf��С
	long m_nDefaultReadSize;//һ���ļ���һ�ζ�ȡ�����ֽ�?---������һ�ν��ļ�ȫ������,,ռ���ڴ�̫��

	pHsElement m_pPreEle;//Ϊ�˱��Ԫ�ص�list��ʽ.
public:
	pHsElement m_pMainEle;//��������Dicom�ļ�����һ����Ԫ��.

	HsBaseFileInfo m_BaseFileInfo;//���ڱ��ļ�����Ĺؼ���Ϣ

	bool m_bXRay;//�ǲ��ǰ����շ�ͼ��չʾ

	//Method:
private:
	// �Ҵ����﷨
	int GetTsType();

	// ��ȡDcm�ļ���һ��Tag��λ��
	bool GetFirstTagPos(void);

	// ��nStartPos����4���ֽ���ΪTag
	int GetTagNumber(unsigned long nStartPos,unsigned long &nTag,bool &bBigEndia);

	// �õ�nTag������
	unsigned long GetTagVR(unsigned long nPos,unsigned long nTag ,bool &bShowField,int &nRet);

	// ��ȡpParentEle����������Element
	int GetChildElement(unsigned long &nPos/*��Ԫ��ֵ��ĵ�һ���ֽ�*/,pHsElement pParentEle/*��Ԫ��ָ��*/);

	// ��ȡ��Item���͵�Ԫ��
	pHsElement GetNormalElement(unsigned long &nStartPos,unsigned long nTag,unsigned long nVR,bool bBigEndia,bool bVrFiledShow,int &nRet);

	// ����һ������Ԫ��
	pHsElement GetSequenceElement(unsigned long &nStartPos,unsigned long nTag,bool bBigEndia,int &nRet);

	// ����һ��ITEMԪ��
	pHsElement GetItemElement(unsigned long &nStartPos/*һ��ItemԪ�صĵ�һ���ֽ�*/,unsigned long nTag/*TAG_ITEM*/,bool bBigEndia/*�����﷨�Ǵ�С*/,int &nRet);

	// �������νṹ
	void BuildTreeRelation(pHsElement pCurEle,pHsElement pParentEle,pHsElement pPreEle);

	//�ͷ�m_buf
	int FreeBuffer(void);

	//����ַ�����ȡָ����һ��
	int DivString(char* pString,const char* pDelChar,int nID, QString &sValue);

	//LodaFileʱ����һ�������Ԫ��.����ɾ���ǲ�����,��Ҫ��ָ�����������Ԫ�ص�ָ�봦��һ��
	pHsElement DestroyEle(pHsElement pEle);

	//��Ԫ�ؽ���List�ṹ
	pHsElement BuildListRelation(pHsElement pEle);

    int ShowElement(pHsElement p,int nLevel);

	//�����ҵ�Element���Leadtools��DS�ж�Ӧ��ELe
	pDICOMELEMENT GetLtEleByMyEle(pHsElement pMyEle, LDicomDS *pDS);

	//����Overlay������,�������ֵ��Сֵ----��Щ����������Ҫѭ��ÿ�����ص�,���Էŵ�һ��.����ѭ������
	int SeparatePixdataAndOverlayByRow(CHsBaseImg &Img,unsigned long iRow,unsigned long nCol,long &nMin,long &nMax);

	//����Element
	void CopyEle(HsElement* pFrom,HsElement* pTo);


	//hmy 
	int  ValueToEle(void *pData,pHsElement pEle, int iTypeSize,bool bCover, int nValueIndex);

	BYTE *WritePara(pHsElement pEle,BYTE *pData,int iDataSize/*���������ĳ��ȣ������ֵ��ѯ*/,bool bBigEndia,int iPareType/*��������*/);

	BYTE *WriteValue(pHsElement pEle,BYTE *pData, int iDataSize,bool bConvert/*��С���Ƿ�ת��*/);

	BYTE *WriteImage(ImageInfo ImgInfo,BYTE *pImageData,int iImageByte,bool bConvert);

	int SaveTsType(pHsElement pEle, int nTsType,HANDLE &hf);

	int SaveItemEle(pHsElement pEle,HANDLE &hf,bool bBigEndia,bool bConvert);

	int SaveSquenceEle(pHsElement pEle,HANDLE &hf,bool bBigEndia,bool bConvert);

	int SaveNormalEle(pHsElement pEle,HANDLE &hf,bool bBigEndia,bool bConvert);

	int SavePixelDataEle(pHsElement pPixelEle,HANDLE &hf,bool bBigEndia,bool bConvert);

	//Ϊ��λ��������--��ȡ��λ���������-----һ��С����,Ϊ�˷���
	bool GetImageLocPara(ImageInfo &info);

	//��ȡ��λ�����޲��ϵڶ������--�еĶ�֡ͼ��Ὣ��Щ��Ϣ���д洢��һ�������£�����������ȥ��
	bool GetImageLocParaMulityFrame(int iFrame,ImageInfo &info);

	//С���ߺ�����ר�Ż�ȡ0x52009230(Per-frame Functional Groups Sequence)��ĳ֡��Item
	pHsElement GetItemFromPerFramFunGpSeq(pHsElement pFrmFunGpSeqEle,int iFrame);

	int CalcInformationLength( int nTsType );

	BYTE* GetByteFromBuffer(pHsElement pEle,	unsigned long &nValueLen, int &nRet);

	//����Element��С����
	//void CopyElement(pHsElement pSrc,pHsElement pDst);

	//�ж��ǲ����շ�ͼ��
	void IsXRay();
public:
	// ����һ���ļ�
	int Hs_LoadFile(const char *cFileName,bool bFullLoad=false);

	// �ͷ�CHsBaseFile����
	int Hs_CloseFile();

	// ���ҵ�һ��Tag=nTag��Ԫ��,pSiblingEle==NULL:��ʾ�ڵ�һ������.��������pSiblingEleͬ���в��ҵ�һ��nTag
	pHsElement Hs_FindFirstEle(pHsElement pSiblingEle,unsigned long nTag,bool bAsTree);

	// ��pEle��ʼ����,������pElen��Tag��ȵ�Ԫ��.bTree==true:ֻ��ͬ������,������Ϊlist���������ļ�
	pHsElement Hs_FindNextEle(pHsElement pEle,bool bTree);

	// ����pEle��ĳ����Ԫ��
	pHsElement Hs_GetChildEle(pHsElement pEle,unsigned long nTag=0,int nIndex=0);

	// ����SQ�͵�pEle������ֱ���¼�Item
	unsigned long Hs_GetSequenceItemEle(pHsElement pEle,QVector<pHsElement> &ItemEleV);

	// ��pEle���ϲ�����pELe��nTag��ͬ��Ԫ��.bTree==true:ֻ��pEleͬ���в���,������list��ʽ���������ļ�
	pHsElement Hs_FindPreEle(pHsElement pEle,bool bTree);

	// ȡ����С����Tag��ֵ;(VR_DS   VR_FD   VR_FL)
	int Hs_GetDoubleValue(pHsElement pEle,	double &fVal,int nValueIndex=0);
	// ֱ��ȡ����С����Tag��ֵ;(VR_DS   VR_FD   VR_FL)
	int Hs_GetDoubleValueA(unsigned long uTag,	double &fVal,bool bRoot=true,int nValueIndex=0);

	//���ַ�����ʽ����Tag��ֵ
	int Hs_GetStringValue(pHsElement pEle, QString &sRet, int nValueIndex = 0);
	//ֱ�ӻ�ȡ��Ŀ¼�����Tag��ֵ
	int Hs_GetStringValueA(unsigned long uTag, QString &sRet, bool bRoot = true, int nValueIndex = 0);

	// ȡ����
	int Hs_GetDateValue(pHsElement pEle,	HsDateTime &DateValue,	int nValueIndex=0);
	//ֱ��ȡ����
	int Hs_GetDateValueA(unsigned long uTag,HsDateTime &DateValue,bool bRoot=true,	int nValueIndex=0);

	// ȡʱ��
	int Hs_GetTimeValue(pHsElement pEle,	HsDateTime &TimeValue,	int nValueIndex=0);
	// ֱ��ȡʱ��
	int Hs_GetTimeValueA(unsigned long uTag,	HsDateTime &TimeValue,bool bRoot=true,	int nValueIndex=0);

	// ȡ����ʱ��
	int Hs_GetDateTimeValue(pHsElement pEle,	HsDateTime &DateTimeValue,	int nValueIndex=0);
	// ֱ��ȡ����ʱ��
	int Hs_GetDateTimeValueA(unsigned long uTag,	HsDateTime &DateTimeValue, bool bRoot=true, int nValueIndex=0);

	// ��ȡ����
	int Hs_GetAgeValue(pHsElement pEle,int &nAge,char &cAgeType);
	// ֱ�ӻ�ȡ����
	int Hs_GetAgeValueA(unsigned long uTag,int &nAge,char &cAgeType,bool bRoot=true);

	//��ȡ����Tagֵ
	int Hs_GetLongValue(pHsElement pEle,	long &nValue,int nValueIndex=0);
	//ֱ�ӻ�ȡ����Tagֵ
	int Hs_GetLongValueA(unsigned long uTag, long &nValue,bool bRoot=true,int nValueIndex=0);

	//ֱ�ӿ���ĳTag��ֵ------��Ҫdelete []���ص�ָ��
	BYTE* Hs_GetByteValue(pHsElement pEle,	unsigned long &nValueLen, int &nRet);
	//ֱ�ӿ���ĳTag��ֵ------��Ҫdelete []���ص�ָ��
	BYTE* Hs_GetByteValueA(unsigned long uTag,	unsigned long &nValueLen, int &nRet, bool bRoot=true);

	//����ĳTag����ֵ�ĸ���
	int Hs_GetValueCount(pHsElement pEle,int &nRet);
	//ֱ�ӷ���ĳTag����ֵ�ĸ���
	int Hs_GetValueCountA(unsigned long uTag,int &nRet, bool bRoot=true);

	//���ַ�����ʽ����Tagֵ
	QString Hs_GetConvertValue(pHsElement pEle ,int nValueIndex, int& nRet);
	//ֱ�����ַ�����ʽ����Tagֵ
	QString Hs_GetConvertValueA(unsigned long uTag, int nValueIndex, int& nRet, bool bRoot = true);

	// ��ȡĳTag_Pixel_Data����Ϣ
	int Hs_GetImageInfo(pHsElement pPixEle, ImageInfo& ImgInfo,int iFrame);

	// ��ȡĳTag_Pixel_Data�ĵ�iFrame֡�����ֽ�,iFrame=-1.��ʾȫ����ȡ
	virtual int Hs_GetImage(pHsElement pPixEle,CHsBaseImg &Img, int iFrame);

	//��ȡ����Lut
	int Hs_GetLuts(pHsElement pPixelEle,CHsBaseImg &Img);

	//����
	virtual int Hs_CopyTo( CHsBaseFile &NewFile);

	//��ȡLut��һ��С������ͨ��
	bool Hs_GetLutDataItem( pHsElement pLutItemEle,LutData &lut );
	bool Hs_GetWcLutItem(pHsElement pLutItemEle, QVector<LutData*> &LutV);

	//ר�Ÿ��������ȡ֡����
	int Hs_GetFrameCount(pHsElement pPixEle);

    //ƫ�ƴ���λ
    int Hs_OffsetWndLevel(CHsBaseImg &Img);

	//�༭DICOM********************************************************************************
	//��ʼ��һ��Dcm�ļ�
	int Hs_InitFile( QString StorageClassUID, QString StorageInstancUID,int nTsType );

	//������Ele
	pHsElement Hs_InsertElement(pHsElement pNeighbor,bool bChild, unsigned long nTag,unsigned long nVR,bool bSquence/*������˽��Tagʱ��Tag�Ƿ�ΪSQ*/,int nIndex/*������˽��Itemʱ,Item��SQ�µ�λ������*/,int &nRet);

	//ɾ��Ele
	int Hs_DeleteElement(pHsElement pEle);

	//ɾ��ֵ�����ض���ֵ,nIndex��0��ʼ
	int Hs_DeleteValues(pHsElement pEle,int nIndex);

	//�޸�Tag ����ֵ ����nValueIndex��0��ʼ
	int Hs_SetLongValue(pHsElement pEle, long nValue, bool bCover,int  nValueIndex);

	//�޸�Tag ������ֵ ����nValueIndex��0��ʼ
	int Hs_SetShortValue(pHsElement pEle, short nValue, bool bCover,int  nValueIndex);

	//�޸�Tag ˫���ȸ�����ֵ ����nValueIndex��0��ʼ
	int Hs_SetDoubleValue(pHsElement pEle, double nValue,bool bCover,int nValueIndex);

	//�޸�Tag �����ȸ�����ֵ ����nValueIndex��0��ʼ
	int Hs_SetFloatValue(pHsElement pEle, float nValue,bool bCover,int nValueIndex);

	//�޸�Tag �ַ�ֵ ����nValueIndex��0��ʼ
	int Hs_SetStringValue(pHsElement pEle, QString nValue,bool bCover,int nValueIndex);

	//�޸�Tag ����ֵ ����nValueIndex��0��ʼ
	int Hs_SetDataValue(pHsElement pEle, HsDateTime nValue,bool bCover,int nValueIndex);

	//�޸�Tag ʱ��ֵ ����nValueIndex��0��ʼ
	int Hs_SetTimeValue(pHsElement pEle, HsDateTime nValue,bool bCover,int nValueIndex);

	//�޸�Tag ����ʱ��ֵ ����nValueIndex��0��ʼ
	int Hs_SetDataTimeValue(pHsElement pEle, HsDateTime nValue,bool bCover,int nValueIndex);

	//�޸�Tag ����ֵ ����nValueIndex��0��ʼ
	int Hs_SetAgeValue(pHsElement pEle,int nAge,char cAgeType,bool bCover,int nValueIndex);

	//����BYTEֵ��
	int Hs_SetByteValue( pHsElement pEle,BYTE *pValue,int nCount);

	//�����޸ģ��γ��µ�DICOM
	int Hs_SaveFile(const char *cFileName,int nTsType=TS_EXPLICIT_VR_LITTLE_ENDIAN);

public:
	//���ļ�֮�临��Tag(���޸�Ŀ¼�µ�Tag)
	int Hs_CopyRootTagTo(CHsBaseFile* pDstDs, unsigned long nTag);//,pHsElement* pDstParentEle);

	QString m_sModality;//ֱ�Ӷ�������ס���ˣ�ʡ���ظ�
};

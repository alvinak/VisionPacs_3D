#pragma once
#include "HsBaseImg.h"

class CHsFile;

class CHsImage :
	public CHsBaseImg
{
public:
	CHsImage();
	~CHsImage();

private:
	CHsFile *m_pDs;

	//��ʼ�����Ϣ
	int InitInfoItem(CHsFile* pDs, INFOITEM &infoItem);
	//���±仯��Ϣ
	int RefreshInfoItem(CHsFile* pDs, INFOITEM &infoItem);
	//��ȡ��ʽ���ַ���������0=��ȷ����1=�ַ�����ʽ����2=������ʽ����3=С����ʽ��
	int GetFormatType(QString &sFormat);

protected:
	//�ڴ�����ռ�е�����
	RECT m_WndRc;
public:
	void SetDs(CHsFile *pDS);
	CHsFile *GetDs();

	//��ȡ��ǰͼ���С.
	SIZE Hs_GetImgSize(bool bDisplaySize = false);

	//�����ڴ����ϵ���ʾ����
	void SetWndRc(RECT rc);

	//��ȡ�ڴ����ϵ���ʾ����
	RECT GetWndRc();

	//��ԭ
	int Hs_Restore();

	//����,bChangeValue=true��Ϊw��c�Ǳ仯��.����w��c�Ǿ���ֵ
	int Hs_WinLevel(long w, long c, bool bChangeValue = false, long *pRetW = NULL, long *pRetC = NULL);

	int Hs_Reload(int islicenum, bool bApplyCurImgState = true);

	//�������תͼ������
	POINT ConvertWndToImg(RECT ImgRcOnWnd, long nImgW, long nImgH, POINT &pt);

	int Hs_ApplyCurImgState();

	//��QT�ķ�ʽ��ͼ
	int Hs_QtDrawImg(QImage &qImage, RECT &rc);

	//��ΪQT��ʵͼ��ߵ����轫ԭʼ���ݷ�ת
	int Hs_FilpOridata();

	//��ýǱ���Ϣ
	MODINFO m_CornorInfo;
	void Hs_GetCornerInfo(bool bFrist);

	//���
	float m_fSilceThick;
};


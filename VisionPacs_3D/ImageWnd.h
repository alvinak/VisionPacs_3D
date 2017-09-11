#ifndef IMAGEWND_H
#define IMAGEWND_H

#include <QWidget>


class CHsImage;
class CHsNormalMprMaker;
class WorkZone;
class OperateMprLines;

namespace Ui {
class ImageWnd;
}

class ImageWnd : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWnd(QWidget *parent = 0);
    ~ImageWnd();

private:
    Ui::ImageWnd *ui;

	//�۽����
	bool m_bFocused; 

	//����ͼ���ڴ�������ʾ��rc
	RECT CalDisplayRect(CHsImage*pImg);//RECT DlgRc,

	int m_nInteractionState;//��������״̬

	//������ؼ���
	QPoint m_PrePoint;//�ƶ������š�WC��,��Ҫ����ƶ�����Ч��

	HSRECT m_fImgRc;//��ȷ��ͼ������

	//ȷ������ͼ������
	int m_nImgWndType;

	//��ͨMPR�и���
	CHsNormalMprMaker *m_pNormalMaker;

	//������ͼ������
	int m_nImgNum;

	//��ʼ��������Ϣ
	void RefreshCornorInfoWidget();
	bool m_bInitCorInfo;
	
	//������Ϣ����
	typedef struct _ROWITEM
	{
		QString sValue;//���ֵ֮��
		QString sType;
		_ROWITEM()
		{
			sValue = "";
			sType = "Normal";
		}
	}ROWITEM;

	typedef struct _QEDITITEM
	{
		QString sName;
		QString sPos;
		QLineEdit *qEdit;
		QLabel *qPreLabel;
		QLabel *qNextLabel;
		_QEDITITEM()
		{
			sName = "";
			sPos = "";
			qEdit = NULL;
			qPreLabel = NULL;
			qNextLabel = NULL;
		}
	}QEDITITEM;

	vector<QEDITITEM> m_vCornorEdit;
	int ArrangeCorinfo(CORNORINFO corInfo,map<int,ROWITEM> &mapRow);
	void InitNormalCorInfo();
	void InitPosCorInfo();
	void ReSetPosCorInfo(QSize deltaSize);

	//��ǩ���ϣ��������λ��
	map<QLabel*,QString> m_mapInfoLabel;
	map<QLineEdit *, QString> m_mapInfoEdit;

	//��ǰͼ������
	int m_nCurImgIndex;

	//ͼ��
	CHsImage *m_pImg;

	//������
	OperateMprLines *m_pOperateLines;
protected:
	QPixmap *m_pPixmap;
	QImage *m_pQImage;

	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent* size);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	
public:
	
	unsigned int m_nLeftButtonInteractionStyle;
	unsigned int m_nRightButtonInteractionStyle;

public:
	//���ñ������Ƿ�Ϊ�۽�����
	void SetFocuse(bool bFocused){ m_bFocused = bFocused; } 

	//��ñ������Ƿ�Ϊ�۽�����
	bool IsFocused(){ return m_bFocused; }	

	//�жϱ��������Ƿ���ͼ����ʾ
	bool IsImgEmpty();

	//���ñ���������ʾͼ��
	int SetImage(CHsImage *pImg);

	void ConvertCoord(long *x1, long *y1, long *x2, long *y2, bool bFromHwdToImg);

	//���ò���
	void SetOperate(QString operate);

	//�������תͼ������
	POINT ConvertWndToImg(RECT ImgRcOnWnd, long nImgW, long nImgH, QPoint &pt);

	long m_nCurW, m_nCurC;

	void ***m_p3DArray;
	vtkSmartPointer<vtkImageData> m_p3DImgData;
	vector<CHsImage*> m_vOriImg;

	void GetImgNumAndWndType(QString sWndName, int nOriImgType);

	//���㱾������ʾͼ��
	int CalcAndShowNormalImg(QString sWndName, int nOriImgType, int iImgIndex,int iSlice);

	void SetCurImageIndex(int nIndex) { m_nCurImgIndex = nIndex; }

	//��ò�����
	OperateMprLines *GetOperateLine() { return m_pOperateLines; }

	//���ͼ��
	CHsImage *GetImage() { return m_pImg; }

	//����MPRģʽ
	void SetMprMode(QString sModeName);


signals:
	void SendImageNum(int);
	void ImageIndexChange(int);
	void ImageThickChange(double);

private slots:
	void OnEditTextChanged(const QString &sText);
	void OnEditFinished();
	void OnMprLinesShow(bool isShow);
	
	
};

#endif // IMAGEWND_H

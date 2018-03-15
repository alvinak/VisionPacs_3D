#ifndef IMAGEWND_H
#define IMAGEWND_H

#include <QWidget>


class CHsImage;
class CHmyMprMaker;
class WorkZone;
class OperateMprLines;
class Hmy3DVector;
class HmyPlane3D;
class HmyLine3D;
class HmyImageData3D;
class CResliceControl;

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
	CHmyMprMaker *m_pMprMaker;

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
	void ReSetPosCorInfoPos(QSize deltaSize);
	void RefreshPosCorInfo();

	//��ǩ���ϣ��������λ��
	map<QLabel*,QString> m_mapInfoLabel;
	map<QLineEdit *, QString> m_mapInfoEdit;

	//����
	int m_nCurImgIndex;

	//ͼ��
	HmyLine3D *m_pLine1;
	HmyLine3D *m_pLine2;
	CHsImage *m_pImg;

	//������
	OperateMprLines *m_pOperateLines;

	//�и��������
	CResliceControl *m_pResliceControl;

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

	//��ʼ������
	void InitImageDataPara(void ***pImgArray, vtkSmartPointer<vtkImageData> p3Ddata, vector<CHsImage*> vOriImg, HmyImageData3D *pHmyImgData);
	void SetResliceControl(CResliceControl *pResliceContrl);
	void SetImageWndType(QString sWndName);

	//������ʾͼ��
	int FirstCalAndShowImage();

	//������������λ�õ�Line����
	void SetOrthogonalIndex(int nIndex);

	//���ͼ��
	int CalcAndShowImg(QString sWndName, int nSliceNum);
	void CalcAndShowImg();

	//��ò�����
	OperateMprLines *GetOperateLine() { return m_pOperateLines; }

	//���ͼ��
	CHsImage *GetImage() { return m_pImg; }

	//����MPRģʽ
	void SetMprMode(QString sModeName);

	//MPR���淽��
	HmyImageData3D *m_pHmyImageData;

	//�����ֱ���ڱ�ͼ������ϵ�ϵĿռ䷽��
	void GetLinesDirection(QPoint pt1, QPoint pt2, HmyLine3D &line);
	
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

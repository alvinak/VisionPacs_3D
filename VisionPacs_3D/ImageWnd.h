#ifndef IMAGEWND_H
#define IMAGEWND_H

#include <QWidget>

class CHsImage;

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

protected:
	QPixmap *m_pPixmap;
	QImage *m_pQImage;

	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
public:
	CHsImage *m_pImg;
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
	void setOperate(QString operate);
	//�������תͼ������
	POINT ConvertWndToImg(RECT ImgRcOnWnd, long nImgW, long nImgH, QPoint &pt);

	long m_nCurW, m_nCurC;
};

#endif // IMAGEWND_H

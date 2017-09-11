#pragma once
class OperateMprLines : public QObject
{
	Q_OBJECT
public:
	OperateMprLines(QObject *parent,int nImgWndType);
	~OperateMprLines();

private:
	

private:
	int m_nWndType;	//�����ڵ�����
	bool m_bShow;	//�Ƿ���ʾ
	bool m_bActiveOblique; //�Ƿ�����б��
	bool m_bInitLines; //�Ƿ��ʼ����

	int m_nNearFactor; //����ϵ��
	bool m_bMousePressed;//����Ƿ���

	QPoint m_prePt;
	
	MoveObject m_moveObject;//�ƶ�����

private:
	//ÿ���ɷֵ�λ��
	MprLine *m_pLineH;
	MprPoint *m_ptHStartRotate;
	MprPoint *m_ptHEndRotate;
	MprPoint *m_ptHStartInter;
	MprPoint *m_ptHEndInter;
	MprLine *m_pLineHSliceTop;
	MprLine *m_pLineHSliceBottom;
	MprTriAngle *m_pTriHStartTop;
	MprTriAngle *m_pTriHStartBottom;
	MprTriAngle *m_pTriHEndTop;
	MprTriAngle *m_pTriHEndBottom;

	MprLine *m_pLineV;
	MprPoint *m_ptVStartRotate;
	MprPoint *m_ptVEndRotate;
	MprPoint *m_ptVStartInter;
	MprPoint *m_ptVEndInter;
	MprLine *m_pLineVSliceTop;
	MprLine *m_pLineVSliceBottom;
	MprTriAngle *m_pTriVStartTop;
	MprTriAngle *m_pTriVStartBottom;
	MprTriAngle *m_pTriVEndTop;
	MprTriAngle *m_pTriVEndBottom;

	MprPoint *m_ptCenter;

	int m_iPointWidth;
	float m_fRotateLineRate;
	int m_nTriAngleWidth;//�����εײ���ȵ�һ��.��ֵ���������������εײ����
	int m_nTriAngleDis;//�����εײ������ߵľ���
private:
	//rc����
	RECT m_rcImgOnWnd;
	long m_nImgW;
	long m_nImgH;

	//��ɫ����
	QColor m_clrLineH;
	QColor m_clrLineV;
	QColor m_clrCenter;

	//���
	int m_nSlicePos;

	//ͼ��ֱ���
	double m_fSpacingX;
	double m_fSpacingY;
public:
	bool RefreshMprLinesPara(RECT rcImg, int nImgWidth, int nImgHeigh, double fspacingX, double fspacingY);
	void OnMprLinesMousePress(QMouseEvent *event);
	bool OnMprLinesMouseMove(QMouseEvent *event);
	void OnMprLinesMouseRelease(QMouseEvent *event);
	void OnMprLinesPaint(QPainter *painter);

public:
	void SetMprLineShow(bool isShow) { m_bShow = isShow; }
	void ActiveOblique() { m_bActiveOblique = true; }
	void DisactiveOblique() { m_bActiveOblique = false; }
	bool IsMprLineShow() { return m_bShow; }
	void OutputLineInfo();
	
	//��������
	void SetManiLinePos(MoveObject object, MprLine *pLine);

	//���ò��
	void SetSliceLinePos(double nSliceThick);
private:
	//��֪���ߣ�����������ת�������
	void UpdateRotatePoint(MprLine *pMainLine, MprPoint *ptStartRotate, MprPoint *ptEndRotate);

	//���ݴ������������ߵ���ɫ
	void GetLinesColor();

	//ͼ�񴰿�����ת��
	bool ConvertImgToWnd(MprPoint &pt);

	//����������ת��Ϊͼ������
	bool ConvertWndToImg(MprPoint &pt);

	//��֪�߶�p1-p2,���ڲ���p1��p2���[�߶γ���*f������1.00>f>0.00]������һ���߶�ΪnHeight�ġ��ײ����ΪnWidth*2��������(��4��),��ͷ����,bUseLeft:��Ҫp1-p2�����ϣ�������������
	bool GetTrianglePoint(QPoint &p1, QPoint &p2, int iDis, UINT nHeight, UINT nWidth, MprTriAngle &T1, MprTriAngle &T2, bool bUseLeft);

	//��֪�߶�p1-p2���󾭹��߶���һ��PubLicPt�ġ��Ҵ�ֱ�ڴ��߶εġ����Ҿ�����߶�nLen�����㣺newPt1��newPt2
	bool GetVerticalLine(QPoint &p1, QPoint &p2, QPoint &PublicPt, double nLen, QPoint &newPt1, QPoint &newPt2);

	//GetVerticalLine�����׺�����ֻ��p1-p2���������ĵ㣿�����Ҳ�ĵ㣿,bUseLeft:��Ҫp1-p2�����ϣ�������������
	QPoint SelectPtSideLine(QPoint &p1, QPoint &p2, QPoint &newPt1, QPoint &newPt2, bool bUseLeft);

	//��֪�߶�p1-p2,����p1����ΪnLen��������	
	bool GetPtByLength(QPoint&pCenterPoint,QPoint &p1, QPoint &p2, double nLen, QPoint &RetPtL, QPoint &RetPtR);

	//��֪�߶�A��B���󽻵�
	bool GetInterPoint(FPOINT pA0, FPOINT pA1, FPOINT pB0, FPOINT pB1, FPOINT &retPt);
	bool GetInterPoint(QPoint pA0, QPoint pA1, QPoint pB0, QPoint pB1, QPoint &retPt);

	//���λ�ü���ֱ��
	bool GetPointNearLines(QPoint pt);
	//���λ�ü���������
	bool GetPointNearTriAngle(QPoint pt);
	//���λ�ü����м��
	bool GetPointNearPoint(QPoint pt);

	//�ж��Ƿ�Ϊ����
	bool DistanceToLines(MprLine* pLine, QPoint pt);
	bool DistanceToTriangle(MprTriAngle *pTriangle, QPoint pt);
	bool DistanceToPoint(MprPoint *ptMpr,QPoint pt, int nDistance);
	float Dist2(QPoint pt1, QPoint pt2);

	//�ƶ���ֱ��
	void MoveMainLines(QPoint deltaPt);

	//�ƶ����ĵ�
	void MoveCenterPt(QPoint pt);
	//��ֱ֪�ߣ����ֱ����һ�����ֱ��ƽ��ֱ��
	void GetParalleLine(MprLine *pMainLine, MprPoint *pt, MprPoint &pParalleLinePt1, MprPoint &pParalleLinePt2);

	//�ƶ������
	void MoveSliceTriangle(QPoint deltaPt);
	void CalSliceLine(MprPoint *pStart, MprPoint *pEnd, MprLine *pTopSlice, MprTriAngle *pTriST, MprTriAngle *pTriET, MprLine *pBottomSlice, MprTriAngle *pTriSB, MprTriAngle *pTriEB);
	void CalDistanceOfParalleLine(QPoint pPt, MprLine *pLine, int &fdistance);
	//�ƶ���ת��
	void MoveRotateLine(QPoint pt);

	//��֪�����ڲ��������㣬������ֱ������ε���������
	bool GetLineAcrossRc(RECT rc, QPoint p1, QPoint p2, QPoint &retP1, QPoint &retP2);

	//���������
	bool ReCalInterPoint(MprLine *pManiLine, MprPoint *pStartInter, MprPoint *pEndInter);

	//��֪�߶�p1-p2,����p1����ΪnLen�ġ����ҿ���p2��һ����	
	bool GetInterPtByLength(QPoint &p1, QPoint &p2, double nLen,QPoint &RetPt);

	//�жϴ˵��Ƿ���rcȥ����
	bool IsPointInRc(RECT rc, QPoint pt);

	//����������Ĵ�������תΪͼ������
	void AllCovertWndToImg();

signals:
	void MprLinesInfoOutput(MprLinesInfo);
};



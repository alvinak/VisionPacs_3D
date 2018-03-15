#pragma once

class CResliceControl;
class HmyLine3D;
class HmyPlane3D;

class OperateMprLines : public QObject
{
	Q_OBJECT
public:
	OperateMprLines(QObject *parent,int nImgWndType);
	~OperateMprLines();	

private:
	int m_nWndType;	//�����ڵ�����
	bool m_bShow;	//�Ƿ���ʾ
	bool m_bActiveOblique; //�Ƿ�����б��
	bool m_bInitLines; //�Ƿ��ʼ����

	int m_nNearFactor; //����ϵ��
	bool m_bMousePressed;//����Ƿ���

	QPoint m_prePt;
		
	MoveObject m_moveObject;//�ƶ�����

	CResliceControl *m_pResliceControl;

private:
	//ÿ���ɷֵ�λ��
	MprLine *m_pLineAxis1;
	MprPoint *m_ptA1StartRotate;
	MprPoint *m_ptA1EndRotate;
	MprPoint *m_ptA1StartInter;
	MprPoint *m_ptA1EndInter;
	MprLine *m_pLineA1SliceTop;
	MprLine *m_pLineA1SliceBottom;

	MprLine *m_pLineAxis2;
	MprPoint *m_ptA2StartRotate;
	MprPoint *m_ptA2EndRotate;
	MprPoint *m_ptA2StartInter;
	MprPoint *m_ptA2EndInter;
	MprLine *m_pLineA2SliceTop;
	MprLine *m_pLineA2SliceBottom;

	MprPoint m_ptPreCenter;
	MprPoint *m_ptCenter;

	int m_iPointWidth;
	float m_fRotateLineRate;

private:
	//rc����
	RECT m_rcImgOnWnd;
	long m_nImgW;
	long m_nImgH;
	double m_dSpacingX;
	double m_dSpacingY;

	//��ɫ����
	QColor m_clrLineAxis1;
	QColor m_clrLineAxis2;
	QColor m_clrCenter;

	//���
	int m_nHWndSlicePos;
	int m_nVWndSlicePos;
	int m_nHImgSlicePos;
	int m_nVImgSlicePos;

public:
	bool RefreshMprLinesPara(RECT rcImg, int nImgWidth, int nImgHeigh, double dSpacingX, double dSpacingY);
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

	//�����µ����ĵ㣬�������ߵ�λ��
	void OnCenterPointChanged();
	
	//��������
	void SetManiLinePos(MoveObject object, MprLine *pLine);

	//���ò��
	void SetSliceLinePos(double nSliceThickm,int nIndex);//0��Axis1, 1:Axis2
	
	//����control
	void SetResliceControl(CResliceControl *control) { m_pResliceControl = control; }
	CResliceControl *GetResliceControl() { return m_pResliceControl; }

private:
	//��֪���ߣ�����������ת�������
	void UpdateRotatePoint(MprLine *pMainLine, MprPoint *ptStartRotate, MprPoint *ptEndRotate);

	//���ݴ������������ߵ���ɫ
	void GetLinesColor();

	//ͼ�񴰿�����ת��
	bool ConvertImgToWnd(MprPoint &pt);

	//����������ת��Ϊͼ������
	bool ConvertWndToImg(MprPoint &pt);

	//��֪�߶�p1-p2���󾭹��߶���һ��PubLicPt�ġ��Ҵ�ֱ�ڴ��߶εġ����Ҿ�����߶�nLen�����㣺newPt1��newPt2
	bool GetVerticalLine(QPoint &p1, QPoint &p2, QPoint &PublicPt, double nLen, QPoint &newPt1, QPoint &newPt2);

	//��֪�߶�A��B���󽻵�
	bool GetInterPoint(FPOINT pA0, FPOINT pA1, FPOINT pB0, FPOINT pB1, FPOINT &retPt);
	bool GetInterPoint(QPoint pA0, QPoint pA1, QPoint pB0, QPoint pB1, QPoint &retPt);

	//���λ�ü���ֱ��
	bool GetPointNearLines(QPoint pt);
	//���λ�ü����м��
	bool GetPointNearPoint(QPoint pt);

	//�ж��Ƿ�Ϊ����
	bool DistanceToLines(MprLine* pLine, QPoint pt);
	bool DistanceToPoint(MprPoint *ptMpr,QPoint pt, int nDistance);
	float Dist2(QPoint pt1, QPoint pt2);

	//�ƶ���ֱ��
	bool MoveMainLines(QPoint deltaPt);

	//�ƶ����ĵ�
	bool MoveCenterPt(QPoint pt);
	//��ֱ֪�ߣ����ֱ����һ�����ֱ��ƽ��ֱ��
	void GetParalleLine(MprLine *pMainLine, MprPoint *pt, MprPoint &pParalleLinePt1, MprPoint &pParalleLinePt2);

	//�ƶ������
	bool MoveSliceLines(QPoint deltaPt);
	void CalSliceLine(MprPoint *pStart, MprPoint *pEnd, MprLine *pTopSlice, MprLine *pBottomSlice);
	void CalDistanceOfParalleLine(QPoint pPt, MprLine *pLine, int &fdistance);
	void CalImageDistance(QPoint pPt, MprLine *pLine, int &fdistance);
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

	//�������������
	int GetAxis1();
	int GetAxis2();	

	//���ƽ��ˮƽ&��ֱ����
	int GetPlaneAxis1();
	int GetPlaneAxis2();

	//�ƶ�����ʱ��control��Ӱ��
	void MoveResliceControlCenter();

	//������ת
	double RotateAxisByPoint(QPoint pt, MoveObject moveObject);
	void RotateAxisByAngle(double angle, MoveObject moveObject);

	void RotateVectorAboutVector(double vectorToBeRotated[3],
		double axis[3], // vector about which we rotate
		double angle, // angle in radians
		double output[3]);

	//��������ƽ��Ľ���
	HmyLine3D CalInterSectingLine(HmyPlane3D *target, HmyPlane3D *reference);


signals:
	void MprLinesInfoChange(MprLinesInfo info);
};



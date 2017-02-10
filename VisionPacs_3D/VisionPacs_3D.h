#ifndef VISIONPACS_3D_H
#define VISIONPACS_3D_H

#include <QtWidgets/QMainWindow>
#include "ui_VisionPacs_3D.h"
#include "WaitDlg.h"

class CHsFile;
class CHsImage;

//����MAP
typedef struct _DuoQiVector
{
	vector<CHsImage *> imgV;
	int nInstanceID;
	int nFrameID;
	bool bLoaded;
	_DuoQiVector()
	{
		nInstanceID = nFrameID = 0;
		bLoaded = false;
	}
}DuoQiVector;

typedef map<int, DuoQiVector> PRIODLIST;

class VisionPacs_3D : public QMainWindow
{
	Q_OBJECT

public:
	VisionPacs_3D(QWidget *parent = 0);
	~VisionPacs_3D();

private:
	Ui::VisionPacs_3D *ui;

	//�ļ�������Ϣ
	QString m_sFilePath;
	QString m_sSeriesUID;

	//������������ʾ������UID
	QString m_sOnShowSeriesUID;

	//�����������ļ���Ϣ��ͼ����Ϣ
	QVector<CHsFile*>  m_vAllDcmFile;
	QVector<CHsImage*> m_vAllImage;

	//����MAP
	PRIODLIST m_mPriodList;
	int m_iPeriodNum;

	int m_iCurPeriod;

	//��ʾ���ȴ���
	WaitDlg *m_pWaitDlg;
	void ShowWaitDlg();

signals:
	void SetWaitProgress(int);

private slots:
	void on_actionOpen_File_triggered();
	void on_actionTest_triggered();
	void StartProcessImage();

public:
	//��ָ���ļ�����ɸѡ��ָ��seriesUID��ͼ������
	void ExtractImage(QString sFilePath, QString sSeriesUID, vector<string> &vImagePathList);
	bool ReadImgListFromIni(QString sFilePath, QString sSeriesUID, vector<string> &vImagePathList);

	//��ȡͼ����Ϣ
	bool ReadAllImageInfo(vector<string> vImagePathList);

	//ѡ���ض��ķ��ڽ��д����޷���ҲҪ����
	bool ChoosePeriod(int iPeriod);

};

#endif // VISIONPACS_3D_H

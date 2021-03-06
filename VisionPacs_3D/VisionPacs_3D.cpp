#include "stdafx.h"
#include "VisionPacs_3D.h"
#include "HsFile.h"
#include "HsImage.h"
#include "AppConfig.h"


VisionPacs_3D::VisionPacs_3D(QWidget *parent)
	: QMainWindow(parent),
	ui(new Ui::VisionPacs_3D)
	, m_sOnShowSeriesUID("")
	, m_iPeriodNum(1)
	, m_iCurPeriod(1)
	, m_pWaitDlg(NULL)
	, m_sFilePath("")
	, m_sSeriesUID("")
	, m_pVolumePropertywidget(NULL)
{
	ui->setupUi(this);
	setStyleSheet("#MainWidget{background-color:#323232;}");
	setWindowState(Qt::WindowMaximized);

	//设置窗体标题栏隐藏
	setWindowFlags(Qt::FramelessWindowHint);
	//可获取鼠标跟踪效果
	setMouseTracking(true);
	//界面管理
	InitUIConfig();
	//读取配置参数
	CAppConfig configApp;
	bool bRet = configApp.GetBaseConfig();
	if (bRet == false)
	{
		QMessageBox::information(NULL, QString("错误"), QString("配置文件未打开"));
		close();
	}
}

VisionPacs_3D::~VisionPacs_3D()	
{
	delete ui;

	for (int i=0; i<m_vAllDcmFile.size(); i++)
	{
		delete m_vAllDcmFile[i];
	}
	m_vAllDcmFile.clear();

	for (int i=0; i<m_vAllImage.size();i++)
	{
		delete m_vAllImage[i];
	}
	m_vAllImage.clear();

	if (m_pWaitDlg)
	{
		delete m_pWaitDlg;
		m_pWaitDlg = NULL;
	}

	delete m_pImgLeftBG;
	delete m_pImgRightBG;	
	delete m_pVrLeftBG;
	delete m_pVrRightBG;
	delete m_pMprType;

	if (m_pVolumePropertywidget)
	{
		delete m_pVolumePropertywidget;
		m_pVolumePropertywidget = NULL;
	}
}

void VisionPacs_3D::on_actionOpen_File_triggered()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Open Dicom"), ".", tr("Dicom File(*.dcm)"));

	CHsFile *hf = new CHsFile;

	hf->Hs_LoadFile(path.toLatin1().data());

	pHsElement pPixEle = hf->Hs_FindFirstEle(NULL, TAG_PIXEL_DATA, true);//找主图

	if (pPixEle == NULL)
		return;

	CHsImage *pImg = new CHsImage;
	int nFrame = hf->Hs_GetFrameCount(pPixEle);
	pImg->m_ImgInfo.nFrame = nFrame;
	hf->Hs_GetImageInfo(pPixEle, pImg->m_ImgInfo, 0);
	pImg->Hs_InitImgState();
	pImg->SetDs(hf);
}

void VisionPacs_3D::on_actionTest_triggered()
{
	QString sFilePath = "E:\\TestData\\Heart1";
	QString sSeriesUID = "1.2.840.113619.2.55.3.163580517.749.1227660948.182";

	m_sFilePath = sFilePath;
	m_sSeriesUID = sSeriesUID;

	ShowWaitDlg();
}


void VisionPacs_3D::StartProcessImage()
{
	//开始处理图像
	vector<string> vImagePathList;
	ExtractImage(m_sFilePath, m_sSeriesUID, vImagePathList);

	int nRet = ReadAllImageInfo(vImagePathList);
	if (nRet != Ret_Success)
		return;

	m_iPeriodNum = m_mPriodList.size();
	ChoosePeriod(0);

	ui->Workzone->LoadImageData();
}

void VisionPacs_3D::ReceiveProcessEnd()
{
	//渲染方案控件初始化
	if (m_pVolumePropertywidget)
	{
		ui->Workzone->InitVolumePropertyWidget(m_pVolumePropertywidget);
	}	
}

void VisionPacs_3D::Btn_VrOperateClick(int nButtonID)
{
	QString sOperateName = "";
	switch (nButtonID)
	{
	case 0:
		sOperateName = "VR_location";
		break;
	case 1:
		sOperateName = "VR_rotate";
		break;
	case 2:
		sOperateName = "VR_zoom";
		break;
	case 3:
		sOperateName = "VR_pan";
		break;
	}

	ui->Workzone->VrOperteChange(sOperateName);
}

void VisionPacs_3D::Btn_ImgOperateClick(int nButtonID)
{
	QString sOperateName = "";
	switch (nButtonID)
	{
	case 0:
		sOperateName = "Img_location";
		break;
	case 1:
		sOperateName = "Img_browser";
		break;
	case 2:
		sOperateName = "Img_wl";
		break;
	case 3:
		sOperateName = "Img_zoom";
		break;
	case 4:
		sOperateName = "Img_pan";
		break;
	}
	ui->Workzone->ImgOperteChange(sOperateName);
}

void VisionPacs_3D::Btn_CloseWndClick()
{
	close();
}

void VisionPacs_3D::Btn_MinimizeWndClick()
{
	showMinimized();
}

void VisionPacs_3D::ShowWaitDlg()
{
	//弹出进度条窗口
	if (m_pWaitDlg == NULL)
	{
		m_pWaitDlg = new WaitDlg(this);
		m_pWaitDlg->setModal(true);
		m_pWaitDlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
		QObject::connect(this, SIGNAL(SetWaitProgress(int)), m_pWaitDlg, SLOT(SetProgress(int)));
		QObject::connect(ui->Workzone, SIGNAL(SetWaitProgress(int)), m_pWaitDlg, SLOT(SetProgress(int)));
		QObject::connect(m_pWaitDlg, SIGNAL(StartProcess()), this, SLOT(StartProcessImage()), Qt::QueuedConnection);	
		QObject::connect(m_pWaitDlg, SIGNAL(ProcessEnd()), this, SLOT(ReceiveProcessEnd()), Qt::QueuedConnection);
		m_pWaitDlg->show();
	}
}

void VisionPacs_3D::InitUIConfig()
{
	//操作按钮管理
	m_pVrLeftBG = new QButtonGroup();
	m_pVrLeftBG->setExclusive(true);
	m_pVrLeftBG->addButton(ui->VR_location, 0);
	m_pVrLeftBG->addButton(ui->VR_rotate, 1);
	connect(m_pVrLeftBG, SIGNAL(buttonClicked(int)), this, SLOT(Btn_VrOperateClick(int)));
	m_pVrRightBG = new QButtonGroup();
	m_pVrRightBG->setExclusive(true);
	m_pVrRightBG->addButton(ui->VR_zoom, 2);
	m_pVrRightBG->addButton(ui->VR_pan, 3);
	connect(m_pVrRightBG, SIGNAL(buttonClicked(int)), this, SLOT(Btn_VrOperateClick(int)));
	m_pImgLeftBG = new QButtonGroup();
	m_pImgLeftBG->setExclusive(true);
	m_pImgLeftBG->addButton(ui->Img_location, 0);
	m_pImgLeftBG->addButton(ui->Img_browser, 1);
	connect(m_pImgLeftBG, SIGNAL(buttonClicked(int)), this, SLOT(Btn_ImgOperateClick(int)));
	m_pImgRightBG = new QButtonGroup();
	m_pImgRightBG->setExclusive(true);
	m_pImgRightBG->addButton(ui->Img_wl, 2);
	m_pImgRightBG->addButton(ui->Img_zoom, 3);
	m_pImgRightBG->addButton(ui->Img_pan, 4);
	connect(m_pImgRightBG, SIGNAL(buttonClicked(int)), this, SLOT(Btn_ImgOperateClick(int)));

	m_pMprType = new QButtonGroup();
	m_pMprType->setExclusive(true);
	m_pMprType->addButton(ui->Btn_MprLinesShow, 0);
	m_pMprType->addButton(ui->Btn_ActiveMprOblique, 1);
	connect(m_pMprType, SIGNAL(buttonClicked(int)), ui->Workzone, SLOT(Btn_SetMprState(int)));

	//VRmode下拉框设置
	QVector<QString> vModeNames;
	vModeNames.push_back("Bone");
	vModeNames.push_back("Heart");
	vModeNames.push_back("Kidney");
	vModeNames.push_back("MIP");

	ui->VRModeCBox->setIconSize(QSize(45, 45));
	QListWidget *pVRModeListWidget = new QListWidget(this);
	pVRModeListWidget->setIconSize(QSize(45, 45));
	ui->VRModeCBox->setModel(pVRModeListWidget->model());
	ui->VRModeCBox->setView(pVRModeListWidget);

	for (int i = 0; i < vModeNames.size(); i++)
	{
		QListWidgetItem  *item = new QListWidgetItem(pVRModeListWidget);
		QString sIconPath = QString("Image/%1.png").arg(vModeNames[i]);
		item->setIcon(QIcon(sIconPath));
		item->setText(vModeNames[i]);
		pVRModeListWidget->addItem(item);
	}

	if (m_pVolumePropertywidget == NULL)
	{
		m_pVolumePropertywidget = new ctkVTKVolumePropertyWidget;
		ui->RenderVLayout->addWidget(m_pVolumePropertywidget);
	}

	//主按钮配置
	connect(ui->CloseButton, SIGNAL(clicked()), this, SLOT(Btn_CloseWndClick()));
	connect(ui->MinimizeButton, SIGNAL(clicked()), this, SLOT(Btn_MinimizeWndClick()));

}

void VisionPacs_3D::ExtractImage(QString sFilePath, QString sSeriesUID, vector<string> &vImagePathList)
{
	emit SetWaitProgress(0);

	if (m_sOnShowSeriesUID.compare(sSeriesUID) == 0)
		return;
	else
		ui->Workzone->ClearImg(true);

	vImagePathList.clear();
	bool bSeriesIniGo = false;

	//获得对应UID文件列表，分两种方式
	bSeriesIniGo = ReadImgListFromIni(sFilePath,sSeriesUID,vImagePathList);

	bool b = true;
	if (bSeriesIniGo == false)
	{
		gdcm::Directory d;
		d.Load(sFilePath.toLatin1().data(), true);
		const gdcm::Directory::FilenamesType ftAllFilenames = d.GetFilenames();
		const size_t nfiles = ftAllFilenames.size();

		//此过滤器摘出选定SeriesUID图像
		gdcm::SmartPointer<gdcm::Scanner> spSeries = new gdcm::Scanner;
		const gdcm::Tag t1(0x0020, 0x000e);
		spSeries->AddTag(t1);

		b = spSeries->Scan(ftAllFilenames);
		vImagePathList = spSeries->GetAllFilenamesFromTagToValue(t1, sSeriesUID.toLatin1().data());//指定序列图像路径

		if (vImagePathList.size() != 0)
		{
			CHsFile *hf = new CHsFile;

			hf->Hs_LoadFile(vImagePathList[0].c_str());

			if (hf->m_sModality.compare("CT") != 0 && hf->m_sModality.compare("MR") != 0)
			{
				delete hf;
				QMessageBox::information(NULL, QString("错误"), QString("不支持CT/MR以外图像"));
				close();
			}

			delete hf;
		}
	}
	emit SetWaitProgress(10);

	//过滤图像SOP类型和类型
	gdcm::SmartPointer<gdcm::Scanner> spFilterScanner = new gdcm::Scanner;//此过滤器为图像类型过滤器
	const gdcm::Tag t2(0x0002, 0x0002);//SOP class
	const gdcm::Tag t3(0x0008, 0x0008);//Image Type;

	spFilterScanner->AddTag(t2);
	spFilterScanner->AddTag(t3);
	b = spFilterScanner->Scan(vImagePathList);

	gdcm::Scanner::ValuesType  vtSopClass = spFilterScanner->GetValues(t2);
	gdcm::Scanner::ValuesType  vtImageType = spFilterScanner->GetValues(t3);

	if (vtSopClass.size() != 1)
	{
		set<string>::iterator set_iter;
		for (set_iter = vtSopClass.begin(); set_iter != vtSopClass.end(); set_iter++)
		{
			string sSopClass = *set_iter;
			if (sSopClass.compare("1.2.840.10008.5.1.4.1.1.66") != 0)
			{
				gdcm::Directory::FilenamesType ftTemp = spFilterScanner->GetAllFilenamesFromTagToValue(t2, sSopClass.c_str());
				vImagePathList = ftTemp;
			}
		}
	}

	if (vtImageType.size() != 1)
	{
		set<string>::iterator set_iter;
		int iFileNum = 0;
		for (set_iter = vtImageType.begin(); set_iter != vtImageType.end(); set_iter++)
		{
			string sImageType = *set_iter;
			gdcm::Directory::FilenamesType ftTemp = spFilterScanner->GetAllFilenamesFromTagToValue(t3, sImageType.c_str());
			int iTempNum = ftTemp.size();
			if (iTempNum > iFileNum)
			{
				vImagePathList = ftTemp;
				iFileNum = iTempNum;
			}
		}
	}
	emit SetWaitProgress(15);
	return;
}

bool VisionPacs_3D::ReadImgListFromIni(QString sFilePath, QString sSeriesUID, vector<string> &vImagePathList)
{
	QString sIniPath = QString("%1\\seriesinfo\\%2.ini").arg(sFilePath).arg(sSeriesUID);
	QFile fSeriesIni(sIniPath);

	if (fSeriesIni.exists() != true)
		return false;

	HANDLE hf = ::CreateFile(sIniPath.toLatin1().data(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		0);

	if (hf == INVALID_HANDLE_VALUE || hf == NULL)
		return false;

	//读全部字符
	DWORD wSize = ::GetFileSize(hf, NULL);
	if (wSize <= 0)
	{
		::CloseHandle(hf);
		return false;
	}

	char *pInfo = new char[wSize + 100];
	memset(pInfo, 0, wSize);

	DWORD wRead = 0;
	::SetFilePointer(hf, 0, NULL, FILE_BEGIN);

	::ReadFile(hf, pInfo, wSize, &wRead, NULL);

	::CloseHandle(hf);

	//截取需要字符
	QVector<int> iPosV;//"filename="的位置

	int i = 0;
	while (i + 8 < wSize)
	{
		if (pInfo[i] == 'f'	&&
			pInfo[i + 1] == 'i' &&
			pInfo[i + 2] == 'l'	&&
			pInfo[i + 3] == 'e'	&&
			pInfo[i + 4] == 'n'	&&
			pInfo[i + 5] == 'a'	&&
			pInfo[i + 6] == 'm'	&&
			pInfo[i + 7] == 'e' &&
			pInfo[i + 8] == '=')
		{
			iPosV.push_back(i + 9);
		}

		i++;
	}

	int nUID = int(iPosV.size());
	for (int s = 0; s < nUID; s++)
	{
		char ch[100]; memset(ch, 0, 100);

		int pos = iPosV[s];
		for (int c = 0; c < 100; c++)
		{
			if (pos + c >= wSize)
			{
				if (int(strlen(ch)) > 0)
				{
					string fi;
					fi = sFilePath.toStdString();
					fi += "\\";
					fi += ch;
					vImagePathList.push_back(fi);
				}
				break;
			}


			ch[c] = pInfo[pos + c];

			if (ch[c] == '\r' || ch[c] == '\n' || ch[c] == 's')
			{
				ch[c] = '\0';
				if (int(strlen(ch)) > 0)
				{
					string fi;
					fi = sFilePath.toStdString();
					fi += "\\";
					fi += ch;
					vImagePathList.push_back(fi);
				}

				break;
			}
		}
	}
	delete []pInfo;
	return true;
}

bool VisionPacs_3D::ReadAllImageInfo(vector<string> vImagePathList)
{
	for (int i=0; i<m_vAllDcmFile.size();i++)
		delete m_vAllDcmFile[i];
	m_vAllDcmFile.clear();

	for (int i=0; i<m_vAllImage.size();i++)
		delete m_vAllImage[i];
	m_vAllImage.clear();

	m_mPriodList.clear();

	int nRet = Ret_Success;
	double iStep = 20.00 / vImagePathList.size();
	for (int i=0; i<vImagePathList.size();i++)
	{
		CHsFile *pDs = new CHsFile;
		nRet = pDs->Hs_LoadFile(vImagePathList[i].c_str(),false);
		if (nRet != 0)
		{
			QMessageBox::about(this, "错误","载入有误");
			delete pDs;
			return nRet;
		}

		pHsElement pPixEle = pDs->Hs_FindFirstEle(NULL, TAG_PIXEL_DATA, true);
		if (pPixEle == NULL)
		{
			QMessageBox::about(this, "错误", "未发现图像数据");
			delete pDs;
			return Ret_NoPixelFound;
		}

		QString sModality = "";
		pDs->Hs_GetStringValueA(TAG_MODALITY, sModality, true, 0);
		if (sModality.compare("CT") != 0 && sModality.compare("MR") != 0)
		{
			QMessageBox::about(this, "错误", "不支持此类型图像");
			delete pDs;
			return Ret_UnSupportPara;
		}

		int nFrame = pDs->Hs_GetFrameCount(pPixEle);
		if (nFrame <= 0)
		{
			QMessageBox::about(this, "错误", "文件有误");
			delete pDs;
			return Ret_NoPixelFound;
		}
		m_vAllDcmFile.push_back(pDs);
		
		emit SetWaitProgress(15+i*iStep);

		for (int i = 0; i < nFrame; i++)
		{
			CHsImage *pImg = new CHsImage;
			pDs->Hs_GetImageInfo(pPixEle, pImg->m_ImgInfo, i);
			pImg->SetDs(pDs);
			pImg->m_bMpr = true;
			m_vAllImage.push_back(pImg);

			if (pImg->m_ImgInfo.sScanOptions.indexOf("HELICAL") != -1)
			{
				vector<CHsImage *> &v = m_mPriodList[pImg->m_ImgInfo.iAcquisitionNum].imgV;
				v.push_back(pImg);
				m_mPriodList[pImg->m_ImgInfo.iAcquisitionNum].nFrameID = i;
			}
			else
			{
				vector<CHsImage *> &v = m_mPriodList[1].imgV;
				v.push_back(pImg);
				m_mPriodList[1].nFrameID = i;
			}
		}
	}

	return nRet;
}

bool VisionPacs_3D::ChoosePeriod(int iPeriod)
{
	PRIODLIST::const_iterator iter = m_mPriodList.begin();
	int iMiniPriod = (*iter).first;

	m_iCurPeriod = iMiniPriod + iPeriod;

	ui->Workzone->ClearImg(true);

	ui->Workzone->SetImageVector(m_mPriodList[m_iCurPeriod].imgV);

	return true;
}

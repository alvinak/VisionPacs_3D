#ifndef VTK3DWND_H
#define VTK3DWND_H

#include <QWidget>
#include "stdafx.h"
#include "VtkVRInteractorStyle.h"

namespace Ui {
class Vtk3DWnd;
}

class Vtk3DWnd : public QWidget
{
    Q_OBJECT

public:
    explicit Vtk3DWnd(QWidget *parent = 0);
    ~Vtk3DWnd();

private:
    Ui::Vtk3DWnd *ui;

    void resizeEvent(QResizeEvent *);

    vtkSmartPointer<vtkRenderWindowInteractor>	 m_pIrener;

    vtkSmartPointer<vtkVolume>					 m_pVolume;	//volume�ƹ�ӳ���������Զ���
//    vtkSmartPointer<vtkSmartVolumeMapper>		 m_pSmartMapper;
    vtkSmartPointer<vtkSlicerGPURayCastVolumeMapper> m_pSlicerMapper;
    vtkSmartPointer<vtkOrientationMarkerWidget>  m_pMarkerWidget;
    vtkSmartPointer<vtkVolumeProperty>			 m_pVolProperty;//��������ָ��
    vtkSmartPointer<vtkAnnotatedCubeActor>       m_pCubeAcor;
    vtkSmartPointer<vtkImageResample>            m_pImgResample;

	vtkSmartPointer<VtkVRInteractorStyle>		 m_pStyle;

public:

    vtkSmartPointer<vtkRenderer>				 m_p3DRenderer;
    vtkSmartPointer<vtkPiecewiseFunction>	 	 m_pPieceFun;
    vtkSmartPointer<vtkColorTransferFunction>	 m_pClrTrans;

    void Setup3DVR();//���ù������ԣ���λ�����
    void Set3DVRmode(QString sMode);//͸���ȵ�����
    void SetupCamera(QString sOrientationName);//���ùۿ��Ƕ�

	void SetImageDate(vtkSmartPointer<vtkImageData> pImageData);

	void ReRender();

    double ReductionFactor;
    double FrameRate;

public:
	void ChangeOperate(QString sOperate);

};

#endif // VTK3DWND_H

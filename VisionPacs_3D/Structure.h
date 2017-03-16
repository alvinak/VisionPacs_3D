#pragma once

typedef QMap<unsigned long, QString > MAP_TAGNAME;

//image process return code
#define Ret_FileExist				-8	//����ļ�AddFileʱ,���ļ�����ӹ���
#define Ret_UnKnownCase				-7
#define Ret_FindOddLen				-6	//����ֵ����Ϊ���������,�������ɹ�
#define Ret_NoValue					-5
#define Ret_ReachPixelData			-4
#define Ret_ReachBufferEnd			-3	//��ǰBuffer����,������ĳtag���жϵ�����,��Ҫ��ȥ��һ��buffer
#define Ret_ReachFileEnd			-2
#define Ret_ReachParentEleEnd		-1

//Error Code define
#define Ret_Success					0 

#define Ret_InvalidPara				1 
#define Ret_LoadFileError			2
#define Ret_GetTagPosError			3
#define Ret_InvalidDicomFile		4
#define Ret_TagNotFound				5
#define Ret_NoPixelFound			6
#define Ret_ErrorInSaveAs			7
#define Ret_InvalidDicomVR			8
#define Ret_GetElementError			9
#define Ret_NoTransferSyntaxes		10
#define Ret_GetValueFailed			11
#define Ret_OutOfValueCount			12
#define Ret_InvalidBuf				13
#define Ret_GetImgError				14
#define Ret_AllocateMemFailed		15
#define Ret_GetLutDataError			16
#define Ret_LedtolsGetImgError		17
#define Ret_EmptyImage				18
#define Ret_FileNotFound			19
#define Ret_GetPixValueFailed		20
#define Ret_InvalidPixFormat		21
#define Ret_UnSupportPara			22//�Ϸ�,���ǻ�û��ɵĲ���
#define Ret_InvalidImgForSubtract	23//��Ӱ֮�Ƿ�ͼ��

#define Ret_CreateFileFail	        24	
#define Ret_InvalidElement			25
#define Ret_RepeatElement			26
#define Ret_NoObjectSelected		27
#define Ret_InvalidImgInfo			28
#define Ret_CreateAviErr			29


#define Ret_InvalidFuncID		101
#define Ret_InvalidScreenID		102
#define Ret_NoScreenWnd			103

#define Ret_AddPrintImgErr		104
#define Ret_InsertPrintGpErr	105
#define Ret_CreateLBitmapErr	106
#define Ret_CreateMemDcErr		107
#define Ret_CreateBitmapErr		108
#define Ret_SelectObjectErr		109
#define Ret_CreateAtlImageErr	110
#define Ret_GetAtlImageDcErr	111

#define Ret_AbstractUnsupported	300
#define Ret_AssociateRejected	301
#define Ret_PrintingNow			302
#define Ret_GetImgBoxUidEmpty	303

typedef struct _DcmTag
{
	unsigned long  nCode;     // Tag Code
	unsigned long  nMask;     // Mask (for multiple-elements specifies the same entry in the table)
	char		   pszName[100];  // Name
	unsigned long  nVR;       // Value Representation
	unsigned long  nMinVM;    // Minimum Value Multiplicity
	unsigned long   nMaxVM;    // Maximum Value Multiplicity
	unsigned long  nDivideVM; // Value that should divide the Value Multiplicity

	_DcmTag()
	{
		nCode = 0;
		nMask = 0;
		strcpy_s(pszName, 100, "UnKnown");
		nVR = 0;
		nMinVM = 0;
		nMaxVM = 0;
		nDivideVM = 0;
	}

} DcmTag, *pDcmTag;

typedef struct _DcmVR
{
	unsigned long  nCode;		// Code (VR_AE, VR_AS, ...)
	char pszName[256];			// Name ("Application Entity", "Age String", ...)
	char pszShortName[10];		//Name (VR_OF,VR_UI)

	unsigned long  nValueLeng;	// Data Value  Length
	unsigned long  nRestrict;	// Restriction applied to the length
	unsigned long  nUnitSize;	// The size for the smallest item

	unsigned long  nLenOfLenDes;//"ֵ��������"����ռ�ó���,��������ĳ���.һ��Tag����һ������˵������ֵ�ĳ��ȵ�,���˵���ĳ���Ҳ���ǹ̶���
	unsigned long nOffset;		//һ��Tag��ֵ����ʼλ��(�����Tag Number����,Ҳ����һ��Tagǰ�������ֵ��ܳ���).ĳЩ���Ͷ�����ʱ�����ֵ�ǹ̶���.
	int nVrType;				//��֪��,������Ҫ.����
	unsigned long nLenOfVrDes;	//VR������

} DcmVR, *pDcmVR;

typedef struct _HsElement
{
	_HsElement* pParentEle;			//��
	_HsElement* pPreEle;			//��
	_HsElement* pNextEle;			//��
	QVector<_HsElement*> pChildEleV;	//��

	unsigned long	nTag;	    	// Tag
	unsigned long	nVR;			// ����
	unsigned long	nLenDesc;		// ֵ���ȵ�����ռ�����ֽ�

	unsigned long	nValueCount;	//һ������Ԫ���а��������ݸ�����ֵ�Ķ�����
	unsigned long	nLen;			//����ռ���ֽ���

	unsigned long   nTagPos;		//һ������Ԫ�ص���ʼλ��,Ҳ������TagNumber��ĵ�һ���ֽ�
	unsigned long   nOffset;		//TagNumber��һ���ֽ��������ֽ�Ϊֵ���һ���ֽ�(ֵ����Ԫ���ڲ���λ��)

	bool			bBigEndian;		//���Tag�Ƿ��Ǵ���﷨.

	bool			bVirtualItem;	//�е�SQ��ֻ��һ��Item,���ǾͲ�д��,�������Item��Ҫ����һ��

	bool			bNewTag;       //�Ƿ�Ϊ�޸Ĺ��Ļ��½���TAG

	BYTE			*pValue;		//��Ϊ�޸Ĺ��Ļ��½���TAG���������ֵ

	bool			bSquence;		//�Ƿ�ΪSquence;

	_HsElement()//��ʼ��
	{
		Reset();
	}

	void Reset()
	{
		pParentEle = NULL;
		pPreEle = NULL;
		pNextEle = NULL;

		nTag = 0;
		nVR = 0;
		nLenDesc = 0;

		nValueCount = 0;
		nLen = 0;

		nOffset = 0;
		nTagPos = 0;
		bBigEndian = false;

		bVirtualItem = false;

		bNewTag = false;

		pValue = NULL;

		bSquence = false;

	}

	_HsElement &operator=(const _HsElement &info)
	{
		pParentEle = NULL;			//��
		pPreEle = NULL;			    //��
		pNextEle = NULL;			//��
		//vector<_HsElement*> pChildEleV;	//��

		nTag = info.nTag;
		nVR = info.nVR;
		nLenDesc = info.nLenDesc;

		nValueCount = info.nValueCount;
		nLen = info.nLen;

		nTagPos = info.nTagPos;
		nOffset = info.nOffset;
		bBigEndian = info.bBigEndian;

		bVirtualItem = info.bVirtualItem;

		bNewTag = info.bNewTag;

		if (info.pValue && info.nLen)
		{
			pValue = new BYTE[nLen];
			memcpy(pValue, info.pValue, nLen);
		}
		else
		{
			pValue = NULL;
		}


		bSquence = info.bSquence;

		return *this;
	}

	~_HsElement()
	{
		size_t nChild = pChildEleV.size();

		for (size_t i = 0; i<nChild; i++)
			delete pChildEleV[i];

		pChildEleV.clear();

		if (pValue)
			delete[]pValue;

		pValue = NULL;
	}

}HsElement, *pHsElement;

typedef struct _HsBaseFileInfo
{
	int					nTsType;		//�����﷨Transfer Syntrax
	unsigned long		nPixPos;		//����Ԫ�ؿ�ʼ��λ��
	unsigned long		nStartPos;		//��ǩ��������ʼλ��
	unsigned long       nEndPos;		//��ǩ�����Ľ���λ��
	unsigned long		nFullSize;		//�ļ����ֽڳ���
	unsigned long		nCurStart;		//���ļ����ȴ���32000000ʱ��Ҫ���Ƿֶζ�ȡ�ļ�����Ϊ��ǰ��ȡ�ε����
	unsigned long		nCurEnd;		//��Ϊ��ǰ��ȡ�ε��յ�
	int					nSpecialTs;		// �����﷨���´��󣬽���У��

	QString             sFileName;		// �ļ�·���� 
	long                nBitsAllocated;	//���ط���λ
	int					nPixelTagNumber;// һ���ļ��У��м���pixel-data�ı�ǩ
	int					nDataType;		//�������ͣ�1Ϊ��ͨ��2ΪPalette Color, 3��RGB	

	unsigned long		nReadLen;		// ��ȡ�εĳ���
	long				nLoadLen;		// ��һ�ζ�ȡ�ĳ���
	long				nOffsetWidth;	// �����Ԥ��ƫ����
	long				hnOffsetCenter;	// ��λ��Ԥ��ƫ����

	HsElement			pElement;		//��õ�ǰͼ���element

	int					nJPEG;			// �Ƿ��ѹ��ͼƬ
	int					nNewlength;		// ��ѹ�����ݲ��ֵ��ֽڳ���
	unsigned char *     pJPEGbuf;		// ��ѹ��������ָ��

	int					nSpecial;		// Ϊ������Ҫ�趨��1Ϊ����Ͽ��dsa��ͼ��tagֵ�������ԣ��ļ����м���ͼ����㲻�ԡ�

	unsigned long		nCurPos;		//��ǰ����Buffer�������δ���

	bool				bCpuBigEndia;	//CPU�����?

	int					nFrame;//	��ͼ��Ļ��Ǽ�֡

	_HsBaseFileInfo()//��ʼ��
	{
		Reset();
	}

	void Reset()
	{
		nTsType = -1;
		nPixPos = 0;
		nStartPos = 0;
		nEndPos = 0;
		nFullSize = 0;
		nCurStart = 0;
		nCurEnd = 0;
		nSpecialTs = 0;

		sFileName = "";
		nBitsAllocated = 0;
		nPixelTagNumber = 0;
		nDataType = 0;

		nReadLen = 0;
		nLoadLen = 0;
		nOffsetWidth = 0;
		hnOffsetCenter = 0;

		pElement.Reset();

		nJPEG = 0;
		nNewlength = 0;
		pJPEGbuf = 0;
		nSpecial = 0;

		nCurPos = 0;
		bCpuBigEndia = false;
		nFrame = 0;

	}

}HsBaseFileInfo; //�Ƚ���Ҫ�ĳ���


typedef struct _HSPOINT
{
	double x;
	double y;

	bool operator == (const _HSPOINT& other) const
	{
		if (fabs(x - other.x) < 0.000001 && fabs(y - other.y) < 0.000001)
			return true;

		return false;
	}

	bool operator != (const _HSPOINT& other) const
	{
		if (fabs(x - other.x) < 0.000001 && fabs(y - other.y) < 0.000001)
			return false;

		return true;
	}

}HSPOINT;

//����λ����Ҫ�Ĳ������ݶ�������
typedef struct _ImageLoc
{
	//�Ƿ���Ч.
	bool bValide;

	//From Tag:0x00280010
	long nRow;		//ͼ��߶�(��λ����)
	double fRowmm;	//ͼ��߶�(��λmm)

	//From Tag:0x00280011
	long nCol;		//ͼ����(��λ����)
	double fColmm;	//ͼ����(��λmm)

	////From Tag:0x00280030
	double fPixSpacingX;//�������ؼ��
	double fPixSpacingY;//�������ؼ��

	//���ڶ�λ������:�����ӽ�Ϊ��׼.�ӽ���ͷ=Z������,��������=X������,����ǰ�򱳺�=Y������
	//From Tag:0x00200032 TAG_IMAGE_POSITION_PATIENT
	double fOriLeftTopPixX;//ͼ�����Ͻǵ�һ�����ص�,��X�������
	double fOriLeftTopPixY;//ͼ�����Ͻǵ�һ�����ص�,��Y�������
	double fOriLeftTopPixZ;//ͼ�����Ͻǵ�һ�����ص�,��Z�������

	//From Tag:0x00200037 TAG_IMAGE_ORIENTATION_PATIENT
	double fOriFirstRowCosX;//��һ��������X���cosֵ
	double fOriFirstRowCosY;//��һ��������Y���cosֵ
	double fOriFirstRowCosZ;//��һ��������Z���cosֵ

	double fOriFirstColCosX;//��һ��������X���cosֵ
	double fOriFirstColCosY;//��һ��������Y���cosֵ
	double fOriFirstColCosZ;//��һ��������Z���cosֵ

	//
	double fLeftTopPixX;//ͼ�����Ͻǵ�һ�����ص�,��X�������
	double fLeftTopPixY;//ͼ�����Ͻǵ�һ�����ص�,��Y�������
	double fLeftTopPixZ;//ͼ�����Ͻǵ�һ�����ص�,��Z�������

	//From Tag:0x00200037 TAG_IMAGE_ORIENTATION_PATIENT
	double fFirstRowCosX;//��һ��������X���cosֵ
	double fFirstRowCosY;//��һ��������Y���cosֵ
	double fFirstRowCosZ;//��һ��������Z���cosֵ

	double fFirstColCosX;//��һ��������X���cosֵ
	double fFirstColCosY;//��һ��������Y���cosֵ
	double fFirstColCosZ;//��һ��������Z���cosֵ

	//From Tag:0x00180050 TAG_SLICE_THICKNESS
	double fSliceThickness;//���

	//From Tag:0x00201041 TAG_SLICE_LOCATION
	double fSliceLoction;//��λ��

	_ImageLoc()
	{
		bValide = false;
		nRow = 0;
		nCol = 0;
		fPixSpacingX = 0.00;
		fPixSpacingY = 0.00;

		fOriLeftTopPixX = 0.00;
		fOriLeftTopPixY = 0.00;
		fOriLeftTopPixZ = 0.00;

		fOriFirstRowCosX = 0.00;
		fOriFirstRowCosY = 0.00;
		fOriFirstRowCosZ = 0.00;

		fOriFirstColCosX = 0.00;
		fOriFirstColCosY = 0.00;
		fOriFirstColCosZ = 0.00;

		fLeftTopPixX = 0.00;
		fLeftTopPixY = 0.00;
		fLeftTopPixZ = 0.00;

		//From Tag:0x00200037 TAG_IMAGE_ORIENTATION_PATIENT
		fFirstRowCosX = 0.00;
		fFirstRowCosY = 0.00;
		fFirstRowCosZ = 0.00;

		fFirstColCosX = 0.00;
		fFirstColCosY = 0.00;
		fFirstColCosZ = 0.00;

		fSliceThickness = 0.00;
		fSliceLoction = 0.00;
	}

	_ImageLoc & operator = (const _ImageLoc &info)//���������
	{
		this->bValide = info.bValide;

		this->nRow = info.nRow;
		this->fRowmm = info.fRowmm;

		this->nCol = info.nCol;
		this->fColmm = info.fColmm;

		this->fPixSpacingX = info.fPixSpacingX;
		this->fPixSpacingY = info.fPixSpacingY;

		this->fOriLeftTopPixX = info.fOriLeftTopPixX;
		this->fOriLeftTopPixY = info.fOriLeftTopPixY;
		this->fOriLeftTopPixZ = info.fOriLeftTopPixZ;

		this->fOriFirstRowCosX = info.fOriFirstRowCosX;
		this->fOriFirstRowCosY = info.fOriFirstRowCosY;
		this->fOriFirstRowCosZ = info.fOriFirstRowCosZ;

		this->fOriFirstColCosX = info.fOriFirstColCosX;
		this->fOriFirstColCosY = info.fOriFirstColCosY;
		this->fOriFirstColCosZ = info.fOriFirstColCosZ;

		this->fLeftTopPixX = info.fLeftTopPixX;
		this->fLeftTopPixY = info.fLeftTopPixY;
		this->fLeftTopPixZ = info.fLeftTopPixZ;

		this->fFirstRowCosX = info.fFirstRowCosX;
		this->fFirstRowCosY = info.fFirstRowCosY;
		this->fFirstRowCosZ = info.fFirstRowCosZ;

		this->fFirstColCosX = info.fFirstColCosX;
		this->fFirstColCosY = info.fFirstColCosY;
		this->fFirstColCosZ = info.fFirstColCosZ;

		this->fSliceThickness = info.fSliceThickness;
		this->fSliceLoction = info.fSliceLoction;

		return *this;
	}

}ImageLoc;

typedef struct _ImageInfo
{
	QString		 sFileName;							//�����ļ���

	double       fWinCenter;						//��λ
	double       fWinWidth;							//����

	double       fRescaleSlope;						//���µ�����б��
	double       fRescaleIntercept;					//���µ����Ľؾ�

	long		 nLutDescriptor1;					//TAG_LUT_DESCRIPTOR
	long		 nLutDescriptor2;					//TAG_LUT_DESCRIPTOR
	long	     nUltrasound_Color_Data_Present;	//�������Ƿ��в�ɫ	
	long		 nWcLutLen;							//����λLut�ĳ���
	long		 iLutStart;							//�ҵ�Lut�����и��±�.�˴����µ�һ���±�
	long		 iLutEnd;							//iLutStart + nWcLutLen;

	int			 nOverLayType;						//0:��,1������,2��Ƕ��
	long         nOverlayBitsAllocated;             // ����ͼλ����
	long         iOverlayBitPosition;               // ����ͼλλ��
	long         nOverlayRows;                      // ����ͼ��
	long         nOverlayCols;						// ����ͼ��
	QString		 sOverlayType;						// ����ͼ����
	long		 nOverlayOrigin1;					// ����ͼԭ��1
	long		 nOverlayOrigin2;					// ����ͼԭ��2
	QString		 sOverlayMagnificationType;			// �Ŵ�����
	QString		 sOverlaySmoothingType;				// ƽ������
	QString		 sOverlayOrImageMagnification;		// �Ŵ���
	long		 nMagnifyToNumberOfColumns;			// �������Ŵ�
	QString		 sModality;
	pHsElement	 pEle;
	int          iFrame;							//��ǰͼ���ID(�ڼ�֡)
	QString		 sFrameLabelVector;					//֡��ǩ������ֵ������Ӧ�õ���֡������,Lhû�ù�
	double		 fImagePosition[3];					// ͼƬλ�õ���������lhû�ù�
	//////////////////////////////////////////////////////////////////////////
	int iCompress;			//0:��ѹ��.3-9.�������﷨���ᵽ��ѹ������
	long nSamplePerPixel;	//һ�����طֳɼ���(��:Sample���������)��ʾ?
	long nBitsAllocated;	//ÿ�η��������λ?
	long nBitStored;		//ÿ��ʵ��ʹ�ö���λ?
	long iHighBit;			//ÿ���ض������λ�ǵڼ�λ?
	long nBitsPerPixel;		//ÿ�������ܹ�ռ����λ? (nSamplePerPixel*nBitsAllocated)
	long iPlanarConfig;		//����RGBͼ����rgb,����rrrgggbbb��Tag(һ��Ϊ0:rgb,��Ϊ1����r1r2r3g1g2g3b1b2b3) TAG_PLANAR_CONFIGURATION

	QString sPhotometric;	//"MONOCHROME1","MONOCHROME2","PALETTE COLOR","RGB","YBR_FULL_422","YBR FULL","FFFFFFFF","-1"

	long nRows;				//��������(�߶�),���ֵ����תʱ�ǻᱻ�ı��
	long nCols;				//��������(���),���ֵ����תʱ�ǻᱻ�ı��

	long nOriRows;				//��������(�߶�),���ֵ��Զ���ᱻ�ı�,ֻ�����¼����ĸ�
	long nOriCols;				//��������(���),���ֵ��Զ���ᱻ�ı�,ֻ�����¼����Ŀ�

	double fPixelSpaceX;	//�������ؼ��(��λӢ��)
	double fPixelSpaceY;	//�������ؼ��(��λӢ��)

	QString sImageType;		//ͼ������
	long nPixelRepresentation;//Pixel_Representation���Ƿ��λ����

	long nSmallestPixelValue;//������Сֵ
	long nLargestPixelValue; //�������ֵ

	long nFrame;			//֡��

	//�ƶϵĽ��
	bool bInverse;			 //if(sPhotometric.find("MONOCHROME1") )>=0)	bInverse = true;
	bool bGrayImg;			//MONOCHROME1 MONOCHROME2=Gray

	double fWinLevelStep;	//wc�ķŴ���.�е�ͼ��LutLen�ܴ�,����Ч��������.�ô�ֵ�Ŵ�

	ImageLoc ImgLocPara;	//��λ����ز���

	bool bBigEndia;//�����

	bool bValid;			//��Ϣ�Ƿ���Ч


	//HMY
	long iAcquisitionNum;  //һ�������еĻ����ţ������ڷֱ����
	QString sScanOptions;	   //ɨ�跽ʽ������������

	double fDifusionBvalue;//��ɢ�����е�Bֵ


	_ImageInfo()
	{
		Reset();
		memset(&ImgLocPara, 0, sizeof(ImageLoc));
	}

	void Reset()
	{
		sFileName = "";
		fWinCenter = 0.0;
		fWinWidth = 0.0;
		fRescaleSlope = 1.00;
		fRescaleIntercept = 0.00;
		nLutDescriptor1 = 0;
		nLutDescriptor2 = 0;
		nUltrasound_Color_Data_Present = -1;
		nWcLutLen = 0;
		iLutStart = 0;
		iLutEnd = 0;

		nOverlayBitsAllocated = 0;        // ����ͼλ����
		iOverlayBitPosition = 0;        // ����ͼλλ��
		nOverlayRows = 0;        // ����ͼ��
		nOverlayCols = 0;        // ����ͼ��
		sOverlayType = "";
		nOverlayOrigin1 = 0;
		nOverlayOrigin2 = 0;
		sOverlayMagnificationType = "";		// �Ŵ�����
		sOverlaySmoothingType = "";		// ƽ������
		sOverlayOrImageMagnification = "";		// �Ŵ���
		nMagnifyToNumberOfColumns = 0;		// �������Ŵ�
		sModality = "";
		pEle = NULL;
		iFrame = -1;
		fImagePosition[0] = -10000.00;
		fImagePosition[1] = -10000.00;
		fImagePosition[2] = -10000.00;
		sFrameLabelVector = "";

		iCompress = 0;
		nSamplePerPixel = 0;
		nBitsAllocated = 0;
		nBitStored = 0;
		iHighBit = 0;
		nBitsPerPixel = 0;
		iPlanarConfig = 0;
		sPhotometric = "";
		nRows = 0;
		nCols = 0;
		nOriRows = 0;
		nOriCols = 0;
		fPixelSpaceX = 0.00;
		fPixelSpaceY = 0.00;
		sImageType = "";
		nPixelRepresentation = 0;
		nSmallestPixelValue = 0;
		nLargestPixelValue = 0;
		bInverse = false;
		nFrame = 0;
		bGrayImg = true;

		fWinLevelStep = 1.00;
		bBigEndia = false;
		bValid = false;

		iAcquisitionNum = 0;
		sScanOptions = "";

		fDifusionBvalue = 0.00;
	}

}ImageInfo;

typedef struct _ImgState
{//���������ͼ���ٴδ��ļ�����������ʱ��Ҫ��ʾ֮ǰ�����������,�˴�ΪӰ��ͼ��Ч���Ĳ���

	bool bImgStateFilled;	//m_ImgState������ֵ��

    RECT *pOriRect;//��ǰ��ԭʼ����,�������ԭʼ�����е�λ��.---�������к�,���ֵ�����¼
	long nCurOriPixRow;//m_pOriData�������к�ǰʣ��ߴ�
	long nCurOriPixCol;//m_pOriData�������к�ǰʣ��ߴ�
    POINT CurWc;	//��ǰ����λ
	unsigned long nCurBitsRow;//��Ҫ��ʾ��ͼ������
	unsigned long nCurBitsCol;//��Ҫ��ʾ��ͼ������
	bool bShowOverLay;
	bool bUseSlope;

	//��ǰ��ʾ�õõ�m_pDisplayData��������
	unsigned long nDispalyRow;
	unsigned long nDispalyCol;

	//λ��Ч��
	HSPOINT fCenterPt;//�������������:x=��Ļ���ĵ㵽ͼ����ʾrc��left�ľ���/ͼ����ʾ���.y=��Ļ���ĵ㵽ͼ����ʾrc��top/ͼ����ʾ�߶�,x=-100��ʾ�������ʾ��100��ʾ���Ҳ���ʾ

	//�Ŵ�Ч��
	double fZoomX;//���ͼ����г�����ʾ��CenterRc,ʵ����ʾʱռ�õõ�DisplayRc,����ֵ�ı������Ǵ�ֵDisplayRc��/CenterRc��,�߶Ȳ�����,���ȵı���һ��,û������

	bool bWholeImgSized;//��ǰ��ʾ�õ�m_pDisplayData��m_pOriData��Hs_Size�����ӹ��Ĳ���,��ôm_pDisplayData��������m_pOriData�ӹ�����?

	bool bToSubstract;//�Ƿ���Ҫ��Ӱ
	bool bSubstracted;//�Ƿ����Ӱ

	bool bInversed;	//�Ƿ�Ƭ

	QString sLeftPatientPos;//ͼ������ǲ��˵�ʲôλ��(AH Hp......),�����ͼ��ķ�ת���仯
	QString sTopPatientPos;
	QString sRightPatientPos;
	QString sBottomPatientPos;

	long nAngleRotated;//��ת�˶��ٶȣ�
	bool bMirror;//�Ƿ����Ҿ�����

	char cLft;//��ǰ��Left��ԭʼͼ����ĸ��ߣ�ȡֵ:LTRB
	char cTop;//��ǰ��Top��ԭʼͼ����ĸ��ߣ�ȡֵ:LTRB
	char cRgt;//��ǰ��Rgiht��ԭʼͼ����ĸ��ߣ�ȡֵ:LTRB
	char cBtm;//��ǰ��Btm��ԭʼͼ����ĸ��ߣ�ȡֵ:LTRB

	QString sCurLutName;//��ǰӦ�õ�Lut���ƣ�Reloadʱ��Ҫ���ָ�Ч����

	bool IsUserSubstractWc;//--��Ӱʱ�û�����wc��
	QVector<int> nSharpValueV;//�񻯴�����ÿ�ε��񻯶�

	_ImgState()
	{
		pOriRect = NULL;
		Reset();
	}

	_ImgState &operator = (const _ImgState &info)
	{
		this->bImgStateFilled = info.bImgStateFilled;

		this->nCurOriPixRow = info.nCurOriPixRow;
		this->nCurOriPixCol = info.nCurOriPixCol;

		if (info.pOriRect)
		{
            this->pOriRect = new RECT;
			*pOriRect = *(info.pOriRect);
		}
		else
		{
			if (this->pOriRect)
				delete this->pOriRect;

			this->pOriRect = NULL;
		}

		this->CurWc = info.CurWc;
		this->nCurBitsRow = info.nCurBitsRow;
		this->nCurBitsCol = info.nCurBitsCol;
		this->nDispalyRow = info.nDispalyRow;
		this->nDispalyCol = info.nDispalyCol;
		this->bShowOverLay = info.bShowOverLay;
		this->bUseSlope = info.bUseSlope;

		this->fCenterPt = info.fCenterPt;

		this->fZoomX = info.fZoomX;
		this->bWholeImgSized = info.bWholeImgSized;
		this->bToSubstract = info.bToSubstract;
		this->bSubstracted = info.bSubstracted;
		this->bInversed = info.bInversed;

		this->sLeftPatientPos = info.sLeftPatientPos;
		this->sTopPatientPos = info.sTopPatientPos;
		this->sRightPatientPos = info.sRightPatientPos;
		this->sBottomPatientPos = info.sBottomPatientPos;

		this->sCurLutName = info.sCurLutName;
		this->nAngleRotated = info.nAngleRotated;
		this->bMirror = info.bMirror;

		this->cBtm = info.cBtm;
		this->cLft = info.cLft;
		this->cRgt = info.cRgt;
		this->cTop = info.cTop;

		this->IsUserSubstractWc = info.IsUserSubstractWc;
		this->nSharpValueV = info.nSharpValueV;

		return *this;
	}

	void Reset()
	{
		if (pOriRect)
			delete pOriRect;
		pOriRect = NULL;

		bImgStateFilled = false;

        CurWc.x=0;
        CurWc.y=0;
		nCurBitsRow = nCurBitsCol = 0;
		bShowOverLay = bUseSlope = true;
		nDispalyRow = nDispalyCol = 0;

		fCenterPt.x = 0.5;
		fCenterPt.y = 0.5;
		fZoomX = 1.00;

		nCurOriPixRow = 0;
		nCurOriPixCol = 0;

		bWholeImgSized = true;
		bToSubstract = false;
		bSubstracted = false;
		bInversed = false;

		sLeftPatientPos = "";
		sTopPatientPos = "";
		sRightPatientPos = "";
		sBottomPatientPos = "";

		sCurLutName = "";
		bMirror = false;
		nAngleRotated = 0;

		cLft = 'L';
		cTop = 'T';
		cRgt = 'R';
		cBtm = 'B';

		IsUserSubstractWc = false;
		nSharpValueV.clear();

	}


	~_ImgState()
	{
		if (pOriRect)
			delete pOriRect;
		pOriRect = NULL;
	}

}ImgState;


typedef struct _HsDateTime
{
	unsigned long nYear; /* year */
	unsigned long nMonth; /* month */
	unsigned long nDay; /* day */
	unsigned long nHours; /* hours */
	unsigned long nMinutes; /* minutes */
	unsigned long nSeconds; /* seconds */
	unsigned long nFractions; /* fraction of a second */
	long nOffset; /* suffix */
} HsDateTime;

typedef struct _LutData
{
	bool	bModality;		//��Modality�Ļ���VOI��?Modality��ǿ���Զ���Modality Lut�²����д���λ
	QString	sName;			//����
	QString sLutType;		//C.11.1.1.2 (OD��HU��US)

	bool	bWc;			//�Ǵ���λ��?

	//���Ǵ���λ,
	double	nW;				//����Ǵ���
	double	nC;				//����Ǵ�λ

	//���������LutData
	long	nLutLen;		//LutData����(����)
	long	nMinValue;		//��Сֵ
	long	nMaxValue;		//���ֵ
	long	nBitsPerData;	//ÿ��LutDataռ��λ?

	BYTE*	pLutData;		//LutData
	unsigned short nBytePerData;//ÿ��Dataռ���ֽ�?

	bool	bUse;			//�Ƿ�����

	int iLutID;//��ʱ��Lutû�����ƣ��Ǿ�������������ļ��ڵ�Lut˳���������

	bool bFromPr;//���Lut�Ƿ�����PR�ļ�

	void Reset()
	{
		bModality = false;
		sName = "";
		sLutType = "";
		bWc = false;
		nW = 0.00;
		nC = 0.00;

		nLutLen = 0;
		nMinValue = 0;
		nMaxValue = 0;
		nBitsPerData = 0;
		pLutData = NULL;
		nBytePerData = 0;
		bUse = false;
		iLutID = 0;
		bFromPr = false;
		if (pLutData)
		{
			delete[]pLutData;
			pLutData = NULL;
		}
	}
	_LutData()
	{
		bModality = false;
		bWc = false;
		nW = 0.00;
		nC = 0.00;
		sName = "";
		sLutType = "";

		nLutLen = 0;
		nMinValue = 0;
		nMaxValue = 0;
		nBitsPerData = 0;
		pLutData = NULL;
		nBytePerData = 0;
		bUse = false;
		iLutID = 0;
		bFromPr = false;
	}

	_LutData &operator = (_LutData const &info)
	{
		if (info.pLutData)
		{
			long nByte = info.nLutLen*info.nBytePerData;
			this->pLutData = new BYTE[nByte];
			memcpy(this->pLutData, info.pLutData, nByte);
		}

		this->bModality = info.bModality;
		this->bWc = info.bWc;
		this->nW = info.nW;
		this->nC = info.nC;
		this->sName = info.sName;
		this->sLutType = info.sLutType;

		this->nLutLen = info.nLutLen;
		this->nMinValue = info.nMinValue;
		this->nMaxValue = info.nMaxValue;
		this->nBitsPerData = info.nBitsPerData;
		this->nBytePerData = info.nBytePerData;
		this->bUse = info.bUse;
		this->iLutID = info.iLutID;
		this->bFromPr = info.bFromPr;

		return *this;
	}

	~_LutData()
	{
		if (pLutData)
		{
			delete[]pLutData;
			pLutData = NULL;
		}
	}
}LutData;

typedef QMap<unsigned long, DcmTag> MAP_TAGPROPERTY;
typedef QMap<unsigned long, DcmVR> MAP_VRPROPERTY;
typedef QMap<QString, int> MAP_DCMFILE;

typedef enum ENUM_HS_TRANSFER_SYNTAXES
{
	TS_IMPLICIT_VR_LITTLE_ENDIAN,//1.2.840.10008.1.2
	TS_EXPLICIT_VR_LITTLE_ENDIAN,//1.2.840.10008.1.2.1
	TS_EXPLICIT_VR_BIG_ENDIAN,//1.2.840.10008.1.2.2
	TS_RLE_LOSSLESS,//1.2.840.10008.1.2.5
	TS_JPEG_BASELINE_1,//1.2.840.10008.1.2.4.50
	TS_JPEG_EXTENDED_2_4,//1.2.840.10008.1.2.4.51
	TS_JPEG_LOSSLESS_NONHIER_14,//1.2.840.10008.1.2.4.57
	TS_JPEG_LOSSLESS_NONHIER_14B,//1.2.840.10008.1.2.4.70
	TS_JPEG2000_LOSSLESS_ONLY,//1.2.840.10008.1.2.4.90
	TS_JPEG2000                   //1.2.840.10008.1.2.4.91  
}HS_TRANSFER_SYNTAXES;

typedef struct _MYDATA24
{
	BYTE pData[3];

	_MYDATA24()
	{
		memset(pData, 0, 3);
	}

	_MYDATA24 &operator = (_MYDATA24 const &info)
	{
		memcpy(this->pData, info.pData, 3);

		return *this;
	}
}MYDATA24;

#define OverLay_None	0	//ûoverlay
#define OverLay_Bits	1	//��������ת�ɶ����ƺ�,ÿһλ����һ��overlay���ݵ�����.
#define OverLay_Pixel	2	//Ƕ�뵽���ؿհ�λ��Overlay
#define OverLay_Byte	3	//һ���ֽڴ���һ��overlay--��û����

#define OverlayValue	9


//------------------------��ֵ��ʽ---------------------------------------------
#define HSIZE_NONE		0//�޲�ֵ
#define HSIZE_NORMAL	1//�������
#define HSIZE_RESAMPLE  2//˫���Բ�ֵ��
#define HSIZE_BICUBIC	3//���ξ��

#define Len_AfterPixelData	300 //�󲿷��ļ������غ�ͽ�����,���е��ļ�������������,����������غ��������ݵ����ֵ

typedef struct _HSRECT
{
	double left;
	double top;
	double right;
	double bottom;
}HSRECT;

//Img���Ҽ�����
typedef enum Enum_ImgInteractionStyleIds {
	LOCTION_INTERACTION,
	WINDOW_LEVEL_INTERACTION,
	PAN_INTERACTION,
	ZOOM_INTERACTION,
	BROWSER_INTERACTIOM
}ImgInteractionStyleIds;

//�������
#define IMGSTSTEM_LOCTION			   0
#define IMGSTSTEM_PICK				   5
#define IMGSTSTEM_PAN				   1
#define IMGSTSTEM_ZOOM				   2
#define IMGSTSTEM_WL				   3
#define IMGSTSTEM_BROWSER			   4


//MPR ͼ��λ
#define  ORIIMG_AXIAL					1
#define  ORIIMG_CORONAL					2
#define  ORIIMG_SAGITTAL				3

//#define  ORIIMG_AXIAL_R					0		//���λ��X���������ҵ���
//#define  ORIIMG_AXIAL_L					1		//���λ��X������������
//#define  ORIIMG_CORONAL_R				2		//��״λ��X���������ҵ���
//#define  ORIIMG_CORONAL_R				3		//��״λ��X������������
//#define  ORIIMG_SAGGITAL_A				4		//ʸ״λ��X��������ǰ����
//#define  ORIIMG_SAGGITAL_P				4		//ʸ״λ��X�������ɺ�ǰ


typedef struct _INFOITEM
{
	QString sItemName;//��Ŀ����	(1:0x.....ΪDicomTag,2:[Txt]Ϊ����,3:{�ֶ���}Ϊ�����ֶ�)
	QString sTag;//Tag
	QString sFormat;//��%s%d��֮��ĸ�ʽ���ַ���
	int iRow;//�ڵڼ���
	int iOrder;//����˳���

	QString sValue;//���ֵ֮��
	_INFOITEM()
	{
		sItemName = "";
		sTag = "";
		sFormat = "";
		iRow = 1;
		iOrder = 1;
		sValue = "";
	}
}INFOITEM;

typedef struct _CORNORINFO
{
	QString sPos;					//λ�ú�
	vector <INFOITEM> infoV;

	_CORNORINFO  &operator = (_CORNORINFO const &info)
	{
		sPos = info.sPos;

		this->infoV = info.infoV;

		return *this;
	}

}CORNORINFO;

typedef struct _MODINFO
{
	QString sModality;

	QString sFaceName;	//��������
	int nSize;		//�����С
	QColor clor;	//������ɫ;

	vector<CORNORINFO> coInfoV;

	_MODINFO()
	{
		sModality = "";
		sFaceName = "����";
		nSize = 10;

		clor = qRgb(255, 255, 255);
	}

	void AddCornerInfo(QString sPos, INFOITEM InfoItem)
	{
		int n = int(coInfoV.size());
		for (int i = 0; i < n; i++)
		{
			if (coInfoV[i].sPos == sPos)
			{
				coInfoV[i].infoV.push_back(InfoItem);
				return;
			}
		}

		CORNORINFO coInfo;
		coInfo.sPos = sPos;
		coInfo.infoV.push_back(InfoItem);

		coInfoV.push_back(coInfo);
	}

	_MODINFO  &operator = (_MODINFO const &info)
	{
		sModality = info.sModality;

		sFaceName = info.sFaceName;
		nSize = info.nSize;
		clor = info.clor;

		this->coInfoV = info.coInfoV;

		return *this;
	}

}MODINFO;

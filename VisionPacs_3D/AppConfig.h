#pragma once
class CAppConfig
{
public:
	CAppConfig();
	~CAppConfig();

	//ע����Ϣ
	static vector<MODINFO> m_ModInfoV;

	bool GetBaseConfig();

	//��m_ModInfoV��������
	static bool GetInfoSet(QString sModality, MODINFO& InfoSet);
};


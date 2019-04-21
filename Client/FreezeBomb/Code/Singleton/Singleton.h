#pragma once

//���Ŀ� SoundSystem�� �̱������� �����ؼ� 
//��ü �ϳ��� �����ϰ� ������ ���� - ����

template<class T>
class Singleton
{
private:
	static T* _instance;

public:
	static T* GetInstance()
	{
		if (_instance == nullptr)
			_instance = new T();
		return _instance;
	}

	static void DeleteInstance()
	{
		if (_instance) 
			delete _instance;		
		
	}
};


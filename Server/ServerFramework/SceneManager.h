#pragma once
#include "MyInclude.h"
#include "MyDefine.h"
#include "Network.h"
#include "Scene.h"

class SceneManager
{
private:
	Scene * pSceneList[MAX_SCENE];
	int preIndex;
public:
	SceneManager();
	~SceneManager();
public:
	void AddScene(int index, Scene* scene);
	void MoveScene(int index);
public:
	Scene * GetScene(){return pSceneList[preIndex];}

public:
	// ��������� ��������� ���� �ʵ��� ����.
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
};


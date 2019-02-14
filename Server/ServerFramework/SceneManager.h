#pragma once
#include "Scene.h"
#include "MyInclude.h"
#include "MyDefine.h"
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

private:
	// ��������� ��������� ���� �ʵ��� ����.
	SceneManager(const SceneManager&);
	SceneManager& operator=(const SceneManager&);
};


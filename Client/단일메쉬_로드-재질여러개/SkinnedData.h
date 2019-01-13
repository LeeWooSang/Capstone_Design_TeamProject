#pragma once



//Ű �������� �� �ð��� bone Transfromation�� �����Ѵ�.
struct Keyframe
{
	Keyframe();
	~Keyframe();

	float TimePos;

	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;


};


//BoneAnimation�� Ű������ list�� �����Ѵ�. 
//�� ���� ����� KeyFrames���� �ð��� �����Ѵ�.
//�ϳ��� �ִϸ��̼��� �ΰ��� Ű�������� ������ �־���Ѵ�.
struct BoneAnimation
{
	float GetStartTime() const;
	float GetEndTime() const;

	void Interpolate(float t, XMFLOAT4X4& matrix) const;

	std::vector<Keyframe> Keyframes;
};


struct AnimationClip
{
	float GetClipStartTime() const;
	float GetClipEndTime() const;

	void Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransform) const;

	std::vector<BoneAnimation> BoneAnimations;
};


class SkinnedData
{
public:

	UINT BoneCount() const;

	float GetClipStartTime(const std::string& clipName) const;
	float GetClipEndTime(const std::string& clipName) const;

	void Set(std::vector<int>& boneHierarchy, std::vector<XMFLOAT4X4>& boneOffsets, std::unordered_map<std::string, AnimationClip>& animations);

	// In a real project, you'd want to cache the result if there was a chance
	// that you were calling this several times with the same clipName at 
	// the same timePos.
	void GetFinalTransforms(const std::string& clipName, float timePos, std::vector<XMFLOAT4X4>& finalTransforms) const;


private:
	//������ ��ȣ, �������� ���, �ִϸ��̼� Ŭ�� ������()
	// Gives parentIndex of ith bone.

	std::vector<int> m_BoneHierarchy;

	std::vector<XMFLOAT4X4> m_BoneOffsets;

	std::unordered_map<std::string, AnimationClip> m_Animations;
};

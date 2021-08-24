#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <openvr.h>
#include <cstdint>
#include <stdexcept>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class OpenVRPart
{
	vr::IVRSystem* hmd;
	uint32_t rtWidth, rtHeight;

	OpenVRPart() : hmd(NULL), rtWidth(0), rtHeight(0)
	{
		if (!isHmdPresent())
		{
			throw std::runtime_error("Error : HMD not detected");
		}

		if (!vr::VR_IsRuntimeInstalled())
		{
			throw std::runtime_error("Error : OpenVR Runtime not detected");
		}

		initVR();

		if (!vr::VRCompositor())
		{
			throw std::runtime_error("Unable to initialize VR compositor!\n");
		}

		hmd->GetRecommendedRenderTargetSize(&rtWidth, &rtHeight);

		std::cout << "Initialized HMD with render target width : " << rtWidth << ", height: " << rtHeight << std::endl;
	}

	inline static bool isHmdPresent()
	{
		return vr::VR_IsHmdPresent();
	}

	void handleVRError(vr::EVRInitError err)
	{
		throw std::runtime_error(vr::VR_GetVRInitErrorAsEnglishDescription(err));
	}

	void initVR()
	{
		vr::EVRInitError err = vr::VRInitError_None;

		hmd = vr::VR_Init(&err, vr::VRApplication_Scene);

		if (err != vr::VRInitError_None)
		{
			handleVRError(err);
		}

		std::cout << GetTrackedDeviceString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String) << std::endl;
		std::cout << GetTrackedDeviceString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String) << std::endl;
	}

	void submitFramesToOpenVR(GLint leftEyeImg, GLint rightEyeImg, bool linear = false)
	{
		if (!hmd)
			throw std::runtime_error("Error submitting frames, HMD is NULL");

		vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
		vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);


		vr::Texture_t leftEyeImage = { (void*)leftEyeImg, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::Texture_t rightEyeImage = { (void*)rightEyeImg, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeImage);
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeImage);

		vr::VRCompositor()->PostPresentHandoff();
	}

	std::string GetTrackedDeviceString(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = NULL)
	{
		uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);

		if (unRequiredBufferLen == 0)
			return "";

		char* pchBuffer = new char[unRequiredBufferLen];
		unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
		std::string sResult = pchBuffer;
		delete[] pchBuffer;
		return sResult;
	}

	glm::mat4 GetEyeMatrix(vr::EVREye eye)
	{
		glm::mat4 result = ToGLM(hmd->GetEyeToHeadTransform(eye));
		return result;
	}

	glm::mat4 ToGLM(const vr::HmdMatrix34_t& m)
	{
		glm::mat4 result = glm::mat4(
			m.m[0][0], m.m[1][0], m.m[2][0], 0.0,
			m.m[0][1], m.m[1][1], m.m[2][1], 0.0,
			m.m[0][2], m.m[1][2], m.m[2][2], 0.0,
			m.m[0][3], m.m[1][3], m.m[2][3], 1.0f);

		return result;
	}
};

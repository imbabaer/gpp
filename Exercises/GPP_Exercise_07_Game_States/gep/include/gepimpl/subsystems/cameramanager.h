#pragma once

#include "gep/interfaces/cameraManager.h"

namespace gep
{
	class ICamera;

	class CameraManager : public ICameraManager
	{
	public:
		CameraManager();
		~CameraManager();

		virtual void initialize();

		virtual void destroy();

		virtual void update(float elapsedTime);

		virtual void setActiveCamera(Camera* camera);

		virtual Camera* getActiveCamera();
	protected:
	private:

		Camera* m_pActiveCam;

	};
}

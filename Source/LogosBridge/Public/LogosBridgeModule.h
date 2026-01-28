#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FLogosBridgeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void StartHttpServer();
	void StopHttpServer();
};

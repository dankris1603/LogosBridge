#include "LogosBridgeModule.h"
#include "HttpServerManager.h"

#define LOCTEXT_NAMESPACE "FLogosBridgeModule"

void FLogosBridgeModule::StartupModule() {
  // Inicialização do Portal
  UE_LOG(LogTemp, Log, TEXT("[LogosBridge] Inicializando Modulo..."));

  StartHttpServer();
}

void FLogosBridgeModule::ShutdownModule() {
  // Fechamento do Portal
  UE_LOG(LogTemp, Log, TEXT("[LogosBridge] Desligando Modulo..."));

  StopHttpServer();
}

void FLogosBridgeModule::StartHttpServer() {
  FHttpServerManager::Get().StartListener();
}

void FLogosBridgeModule::StopHttpServer() {
  FHttpServerManager::Get().StopListener();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLogosBridgeModule, LogosBridge)

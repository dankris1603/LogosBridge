#pragma once

#include "CoreMinimal.h"
#include "HttpPath.h"
#include "HttpServerModule.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"
#include "IHttpRouter.h"

/**
 * LogosBridge Http Server Manager
 * Responsável por gerenciar o ciclo de vida do servidor HTTP e rotas.
 */
class LOGOSBRIDGE_API FHttpServerManager {
public:
  static FHttpServerManager &Get();

  void StartListener();
  void StopListener();

private:
  FHttpServerManager();
  ~FHttpServerManager();

  TSharedPtr<IHttpRouter> HttpRouter;
  uint32 Port = 8080;

  // Handlers de Rota
  bool HandleHealthCheck(const FHttpServerRequest &Request,
                         const FHttpResultCallback &OnComplete);
  bool HandleQuery(const FHttpServerRequest &Request,
                   const FHttpResultCallback &OnComplete);
  bool HandleEdit(const FHttpServerRequest &Request,
                  const FHttpResultCallback &OnComplete);
  bool HandleRunScript(const FHttpServerRequest &Request,
                       const FHttpResultCallback &OnComplete);
  bool HandleCreateAsset(const FHttpServerRequest &Request,
                         const FHttpResultCallback &OnComplete);
  bool HandleDuplicateAsset(const FHttpServerRequest &Request,
                            const FHttpResultCallback &OnComplete);
  bool HandleListAssets(const FHttpServerRequest &Request,
                        const FHttpResultCallback &OnComplete);

  // Helpers
  void SendResponse(const FHttpResultCallback &OnComplete,
                    const FString &JsonString, int32 ResponseCode = 200);
};

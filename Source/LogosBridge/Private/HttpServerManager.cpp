#include "HttpServerManager.h"
#include "AssetActionHandler.h"
#include "Async/Async.h"
#include "BlueprintEditHandler.h"
#include "BlueprintQueryHandler.h"
#include "Engine/Engine.h"
#include "HttpPath.h"
#include "HttpServerModule.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"

FHttpServerManager &FHttpServerManager::Get() {
  static FHttpServerManager Instance;
  return Instance;
}

FHttpServerManager::FHttpServerManager() {}

FHttpServerManager::~FHttpServerManager() { StopListener(); }

void FHttpServerManager::StartListener() {
  if (HttpRouter.IsValid()) {
    return;
  }

  UE_LOG(LogTemp, Log,
         TEXT("[LogosBridge] Iniciando Servidor HTTP na porta %d..."), Port);

  // Garantir que o modulo esta carregado
  FHttpServerModule &HttpServerModule = FHttpServerModule::Get();
  HttpRouter = HttpServerModule.GetHttpRouter(Port);

  if (!HttpRouter.IsValid()) {
    UE_LOG(LogTemp, Error, TEXT("[LogosBridge] Falha ao obter HttpRouter!"));
    return;
  }

  // 1. Health Check
  HttpRouter->BindRoute(
      FHttpPath(TEXT("/api/health")), EHttpServerRequestVerbs::VERB_GET,
      FHttpRequestHandler::CreateLambda(
          [this](const FHttpServerRequest &Request,
                 const FHttpResultCallback &OnComplete) -> bool {
            return HandleHealthCheck(Request, OnComplete);
          }));

  // 2. Query (Leitura de Dados)
  HttpRouter->BindRoute(
      FHttpPath(TEXT("/api/query")), EHttpServerRequestVerbs::VERB_GET,
      FHttpRequestHandler::CreateLambda(
          [this](const FHttpServerRequest &Request,
                 const FHttpResultCallback &OnComplete) -> bool {
            return HandleQuery(Request, OnComplete);
          }));

  // 3. Edit (Modificacao)
  HttpRouter->BindRoute(
      FHttpPath(TEXT("/api/edit")), EHttpServerRequestVerbs::VERB_POST,
      FHttpRequestHandler::CreateLambda(
          [this](const FHttpServerRequest &Request,
                 const FHttpResultCallback &OnComplete) -> bool {
            return HandleEdit(Request, OnComplete);
          }));

  // 4. Run Script (Python)
  HttpRouter->BindRoute(
      FHttpPath(TEXT("/api/run_script")), EHttpServerRequestVerbs::VERB_POST,
      FHttpRequestHandler::CreateLambda(
          [this](const FHttpServerRequest &Request,
                 const FHttpResultCallback &OnComplete) -> bool {
            return HandleRunScript(Request, OnComplete);
          }));

  // 5. Create Asset
  HttpRouter->BindRoute(
      FHttpPath(TEXT("/api/create")), EHttpServerRequestVerbs::VERB_POST,
      FHttpRequestHandler::CreateLambda(
          [this](const FHttpServerRequest &Request,
                 const FHttpResultCallback &OnComplete) -> bool {
            return HandleCreateAsset(Request, OnComplete);
          }));

  // 6. Duplicate Asset
  HttpRouter->BindRoute(
      FHttpPath(TEXT("/api/duplicate")), EHttpServerRequestVerbs::VERB_POST,
      FHttpRequestHandler::CreateLambda(
          [this](const FHttpServerRequest &Request,
                 const FHttpResultCallback &OnComplete) -> bool {
            return HandleDuplicateAsset(Request, OnComplete);
          }));

  // 7. List Assets (Discovery)
  HttpRouter->BindRoute(
      FHttpPath(TEXT("/api/list")), EHttpServerRequestVerbs::VERB_GET,
      FHttpRequestHandler::CreateLambda(
          [this](const FHttpServerRequest &Request,
                 const FHttpResultCallback &OnComplete) -> bool {
            return HandleListAssets(Request, OnComplete);
          }));

  // MANDATORIO para iniciar a escuta nas portas configuradas
  HttpServerModule.StartAllListeners();

  UE_LOG(LogTemp, Log, TEXT("[LogosBridge] Servidor Online!"));
}

void FHttpServerManager::StopListener() {
  UE_LOG(LogTemp, Log, TEXT("[LogosBridge] Parando Listener..."));
  HttpRouter.Reset();
}

bool FHttpServerManager::HandleHealthCheck(
    const FHttpServerRequest &Request, const FHttpResultCallback &OnComplete) {
  FString ResponseJson =
      TEXT("{ \"status\": \"alive\", \"identity\": \"LogosBridge\" }");
  SendResponse(OnComplete, ResponseJson);
  return true;
}

bool FHttpServerManager::HandleQuery(const FHttpServerRequest &Request,
                                     const FHttpResultCallback &OnComplete) {
  FString AssetPath = Request.QueryParams.FindRef(TEXT("asset"));

  AsyncTask(ENamedThreads::GameThread, [this, OnComplete, AssetPath]() {
    FString JsonOutput;
    if (AssetPath.IsEmpty()) {
      JsonOutput = TEXT("{ \"error\": \"Param 'asset' is required\" }");
    } else {
      JsonOutput = FBlueprintQueryHandler::ProcessQuery(AssetPath);
    }
    SendResponse(OnComplete, JsonOutput);
  });

  return true;
}

bool FHttpServerManager::HandleEdit(const FHttpServerRequest &Request,
                                    const FHttpResultCallback &OnComplete) {
  FString BodyStr = "";
  const TArray<uint8> &Body = Request.Body;
  if (Body.Num() > 0) {
    BodyStr = FString(UTF8_TO_TCHAR((const char *)Body.GetData()));
  }

  AsyncTask(ENamedThreads::GameThread, [this, OnComplete, BodyStr]() {
    FString JsonOutput = FBlueprintEditHandler::ProcessEdit(BodyStr);
    SendResponse(OnComplete, JsonOutput);
  });

  return true;
}

bool FHttpServerManager::HandleRunScript(
    const FHttpServerRequest &Request, const FHttpResultCallback &OnComplete) {
  FString ScriptContent = "";
  const TArray<uint8> &Body = Request.Body;
  if (Body.Num() > 0) {
    ScriptContent = FString(UTF8_TO_TCHAR((const char *)Body.GetData()));
  }

  AsyncTask(ENamedThreads::GameThread, [this, OnComplete, ScriptContent]() {
    // Abordagem Segura: Salvar em temp e executar arquivo via console command
    FString TempScriptPath =
        FPaths::ProjectSavedDir() / TEXT("TempLogosScript.py");
    FFileHelper::SaveStringToFile(ScriptContent, *TempScriptPath);

    FString Command = FString::Printf(TEXT("py \"%s\""), *TempScriptPath);

    if (GEngine) {
      GEngine->Exec(NULL, *Command);
    }

    FString JsonOutput =
        TEXT("{ \"status\": \"script_executed\", \"path\": \"") +
        TempScriptPath + TEXT("\" }");
    SendResponse(OnComplete, JsonOutput);
  });

  return true;
}

bool FHttpServerManager::HandleCreateAsset(
    const FHttpServerRequest &Request, const FHttpResultCallback &OnComplete) {
  FString BodyStr = "";
  const TArray<uint8> &Body = Request.Body;
  if (Body.Num() > 0) {
    BodyStr = FString(UTF8_TO_TCHAR((const char *)Body.GetData()));
  }

  AsyncTask(ENamedThreads::GameThread, [this, OnComplete, BodyStr]() {
    FString JsonOutput = FAssetActionHandler::ProcessCreate(BodyStr);
    SendResponse(OnComplete, JsonOutput);
  });

  return true;
}

bool FHttpServerManager::HandleDuplicateAsset(
    const FHttpServerRequest &Request, const FHttpResultCallback &OnComplete) {
  FString BodyStr = "";
  const TArray<uint8> &Body = Request.Body;
  if (Body.Num() > 0) {
    BodyStr = FString(UTF8_TO_TCHAR((const char *)Body.GetData()));
  }

  AsyncTask(ENamedThreads::GameThread, [this, OnComplete, BodyStr]() {
    FString JsonOutput = FAssetActionHandler::ProcessDuplicate(BodyStr);
    SendResponse(OnComplete, JsonOutput);
  });

  return true;
}

bool FHttpServerManager::HandleListAssets(
    const FHttpServerRequest &Request, const FHttpResultCallback &OnComplete) {
  FString Path = Request.QueryParams.FindRef(TEXT("path"));

  AsyncTask(ENamedThreads::GameThread, [this, OnComplete, Path]() {
    FString JsonOutput;
    if (Path.IsEmpty()) {
      JsonOutput = TEXT("{ \"error\": \"Param 'path' is required\" }");
    } else {
      JsonOutput = FBlueprintQueryHandler::ProcessList(Path);
    }
    SendResponse(OnComplete, JsonOutput);
  });

  return true;
}

void FHttpServerManager::SendResponse(const FHttpResultCallback &OnComplete,
                                      const FString &JsonString,
                                      int32 ResponseCode) {
  TUniquePtr<FHttpServerResponse> Response =
      FHttpServerResponse::Create(JsonString, TEXT("application/json"));
  Response->Code = (EHttpServerResponseCodes)ResponseCode;
  OnComplete(MoveTemp(Response));
}

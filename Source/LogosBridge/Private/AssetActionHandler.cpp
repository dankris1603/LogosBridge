#include "AssetActionHandler.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Factories/DataAssetFactory.h"
#include "Factories/Factory.h"
#include "IAssetTools.h"
#include "Serialization/JsonSerializer.h"

FString FAssetActionHandler::ProcessCreate(const FString &JsonPayload) {
  TSharedPtr<FJsonObject> JsonObj;
  TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonPayload);

  if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid()) {
    return TEXT("{ \"error\": \"Invalid JSON\" }");
  }

  FString AssetPath = JsonObj->GetStringField(TEXT("path"));
  FString ClassName = JsonObj->GetStringField(TEXT("class"));

  if (AssetPath.IsEmpty() || ClassName.IsEmpty()) {
    return TEXT("{ \"error\": \"Missing fields: path or class\" }");
  }

  // 1. Obter AssetTools
  IAssetTools &AssetTools =
      FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

  // 2. Determinar Classe e Factory
  UClass *AssetClass = FindObject<UClass>(nullptr, *ClassName);
  if (!AssetClass) {
    // Tentar carregar a classe caso não esteja na memória
    AssetClass = StaticLoadClass(UObject::StaticClass(), nullptr, *ClassName);
  }

  if (!AssetClass) {
    UE_LOG(LogTemp, Error, TEXT("[LogosBridge] Classe '%s' nao encontrada!"),
           *ClassName);
    return FString::Printf(TEXT("{ \"error\": \"Class '%s' not found\" }"),
                           *ClassName);
  }

  // 3. Criar o Asset
  FString PackagePath, AssetName;
  AssetPath.Split(TEXT("/"), &PackagePath, &AssetName, ESearchCase::IgnoreCase,
                  ESearchDir::FromEnd);

  UObject *NewAsset = nullptr;

  // Custom factory selection for common types
  UFactory *Factory = nullptr;
  if (AssetClass->IsChildOf(UDataAsset::StaticClass())) {
    UDataAssetFactory *DAF = NewObject<UDataAssetFactory>();
    DAF->DataAssetClass = AssetClass;
    Factory = DAF;
  }

  UE_LOG(LogTemp, Log,
         TEXT("[LogosBridge] Tentando criar asset: %s em %s (Classe: %s, "
              "Factory: %s)"),
         *AssetName, *PackagePath, *AssetClass->GetName(),
         Factory ? *Factory->GetName() : TEXT("Nenhum"));

  NewAsset =
      AssetTools.CreateAsset(AssetName, PackagePath, AssetClass, Factory);

  if (NewAsset) {
    UE_LOG(LogTemp, Log, TEXT("[LogosBridge] Asset criado com sucesso: %s"),
           *NewAsset->GetPathName());
    return FString::Printf(
        TEXT("{ \"status\": \"success\", \"path\": \"%s\" }"),
        *NewAsset->GetPathName());
  } else {
    UE_LOG(LogTemp, Error,
           TEXT("[LogosBridge] AssetTools falhou ao criar o asset!"));
  }

  return TEXT("{ \"error\": \"Failed to create asset\" }");
}

FString FAssetActionHandler::ProcessDuplicate(const FString &JsonPayload) {
  TSharedPtr<FJsonObject> JsonObj;
  TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonPayload);

  if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid()) {
    return TEXT("{ \"error\": \"Invalid JSON\" }");
  }

  FString SourcePath = JsonObj->GetStringField(TEXT("source"));
  FString DestPath = JsonObj->GetStringField(TEXT("destination"));

  if (SourcePath.IsEmpty() || DestPath.IsEmpty()) {
    return TEXT("{ \"error\": \"Missing fields: source or destination\" }");
  }

  // 1. Carregar Source
  UObject *SourceObj =
      StaticLoadObject(UObject::StaticClass(), nullptr, *SourcePath);
  if (!SourceObj) {
    return TEXT("{ \"error\": \"Source asset not found\" }");
  }

  // 2. Preparar Destino
  FString DestPkgPath, DestName;
  DestPath.Split(TEXT("/"), &DestPkgPath, &DestName, ESearchCase::IgnoreCase,
                 ESearchDir::FromEnd);

  // 3. Duplicar
  IAssetTools &AssetTools =
      FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

  UObject *NewObj = AssetTools.DuplicateAsset(DestName, DestPkgPath, SourceObj);

  if (NewObj) {
    return FString::Printf(
        TEXT("{ \"status\": \"success\", \"path\": \"%s\" }"),
        *NewObj->GetPathName());
  }

  return TEXT("{ \"error\": \"Failed to duplicate asset\" }");
}

#include "BlueprintQueryHandler.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/UnrealType.h"

FString FBlueprintQueryHandler::ProcessQuery(const FString &AssetPath) {
  // 1. Tentar carregar o Asset
  // Nota: StaticLoadObject pode ser lento, idealmente usariamos AssetRegistry
  // para buscas leves
  UObject *LoadedObject =
      StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);

  if (!LoadedObject) {
    return TEXT("{ \"error\": \"Asset not found or failed to load\" }");
  }

  // 2. Se for um Blueprint, queremos olhar o GeneratedClass (CDO) para ver
  // valores padrao
  UObject *ObjectToInspect = LoadedObject;
  if (UBlueprint *Blueprint = Cast<UBlueprint>(LoadedObject)) {
    if (Blueprint->GeneratedClass) {
      ObjectToInspect = Blueprint->GeneratedClass->GetDefaultObject();
    }
  }

  // 3. Serializar
  TSharedPtr<FJsonObject> JsonObj = SerializeObject(ObjectToInspect);

  // Adicionar metadados
  JsonObj->SetStringField("name", LoadedObject->GetName());
  JsonObj->SetStringField("class", LoadedObject->GetClass()->GetName());

  // 4. Converter para String
  FString OutputString;
  TSharedRef<TJsonWriter<>> Writer =
      TJsonWriterFactory<>::Create(&OutputString);
  FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

  return OutputString;
}

FString FBlueprintQueryHandler::ProcessList(const FString &Path) {
  FAssetRegistryModule &AssetRegistryModule =
      FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

  TArray<FAssetData> AssetDataList;
  AssetRegistryModule.Get().GetAssetsByPath(FName(*Path), AssetDataList, true);

  TArray<TSharedPtr<FJsonValue>> AssetsArray;
  for (const FAssetData &AssetData : AssetDataList) {
    TSharedPtr<FJsonObject> AssetObj = MakeShareable(new FJsonObject());
    AssetObj->SetStringField("name", AssetData.AssetName.ToString());
    AssetObj->SetStringField("path", AssetData.PackageName.ToString());
    AssetObj->SetStringField("class", AssetData.AssetClassPath.ToString());
    AssetsArray.Add(MakeShareable(new FJsonValueObject(AssetObj)));
  }

  TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());
  RootObj->SetArrayField("assets", AssetsArray);
  RootObj->SetNumberField("count", AssetsArray.Num());

  FString OutputString;
  TSharedRef<TJsonWriter<>> Writer =
      TJsonWriterFactory<>::Create(&OutputString);
  FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer);

  return OutputString;
}

TSharedPtr<FJsonObject>
FBlueprintQueryHandler::SerializeObject(UObject *Object) {
  TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());

  if (!Object)
    return Result;

  // Iterar sobre propriedades
  for (TFieldIterator<FProperty> PropIt(Object->GetClass()); PropIt; ++PropIt) {
    FProperty *Prop = *PropIt;
    FString PropName = Prop->GetName();

    // Pular propriedades internas/editor-only complexas por enquanto para
    // simplificar
    if (Prop->HasAnyPropertyFlags(CPF_Transient | CPF_EditorOnly)) {
      continue;
    }

    // Ler valores básicos
    // Float
    if (FFloatProperty *FloatProp = CastField<FFloatProperty>(Prop)) {
      float Val = FloatProp->GetPropertyValue_InContainer(Object);
      Result->SetNumberField(PropName, Val);
    }
    // Int
    else if (FIntProperty *IntProp = CastField<FIntProperty>(Prop)) {
      int32 Val = IntProp->GetPropertyValue_InContainer(Object);
      Result->SetNumberField(PropName, Val);
    }
    // Bool
    else if (FBoolProperty *BoolProp = CastField<FBoolProperty>(Prop)) {
      bool Val = BoolProp->GetPropertyValue_InContainer(Object);
      Result->SetBoolField(PropName, Val);
    }
    // String / Name / Text
    else if (FStrProperty *StrProp = CastField<FStrProperty>(Prop)) {
      FString Val = StrProp->GetPropertyValue_InContainer(Object);
      Result->SetStringField(PropName, Val);
    } else if (FNameProperty *NameProp = CastField<FNameProperty>(Prop)) {
      FName Val = NameProp->GetPropertyValue_InContainer(Object);
      Result->SetStringField(PropName, Val.ToString());
    }
    // Object Reference (apenas nome)
    else if (FObjectProperty *ObjProp = CastField<FObjectProperty>(Prop)) {
      UObject *SubObj = ObjProp->GetPropertyValue_InContainer(Object);
      if (SubObj) {
        Result->SetStringField(PropName, SubObj->GetPathName());
      } else {
        Result->SetField(PropName, MakeShareable(new FJsonValueNull()));
      }
    }
  }

  // 1.2 Iterar sobre funções (Methods)
  TArray<TSharedPtr<FJsonValue>> FunctionsArray;
  for (TFieldIterator<UFunction> FuncIt(Object->GetClass(),
                                        EFieldIterationFlags::IncludeSuper);
       FuncIt; ++FuncIt) {
    UFunction *Func = *FuncIt;
    if (Func) {
      FunctionsArray.Add(MakeShareable(new FJsonValueString(Func->GetName())));
    }
  }
  Result->SetArrayField(TEXT("functions"), FunctionsArray);

  return Result;
}

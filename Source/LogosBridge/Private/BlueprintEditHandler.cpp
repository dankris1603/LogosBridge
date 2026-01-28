#include "BlueprintEditHandler.h"
#include "Editor.h" // Necessario para GEditor (Transacoes)
#include "Engine/Blueprint.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/UnrealType.h"

FString FBlueprintEditHandler::ProcessEdit(const FString &JsonPayload) {
  // 1. Parse JSON
  TSharedPtr<FJsonObject> JsonObj;
  TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonPayload);

  if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid()) {
    return TEXT("{ \"error\": \"Invalid JSON\" }");
  }

  FString AssetPath = JsonObj->GetStringField(TEXT("asset"));
  FString PropName = JsonObj->GetStringField(TEXT("property"));
  TSharedPtr<FJsonValue> NewValue = JsonObj->TryGetField(TEXT("value"));

  if (AssetPath.IsEmpty() || PropName.IsEmpty() || !NewValue.IsValid()) {
    return TEXT("{ \"error\": \"Missing fields: asset, property, or value\" }");
  }

  // 2. Carregar Asset
  UObject *LoadedObject =
      StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
  if (!LoadedObject) {
    return TEXT("{ \"error\": \"Asset not found\" }");
  }

  // 3. Resolver Objeto alvo (CDO se for Blueprint)
  UObject *TargetObject = LoadedObject;
  if (UBlueprint *Blueprint = Cast<UBlueprint>(LoadedObject)) {
    if (Blueprint->GeneratedClass) {
      TargetObject = Blueprint->GeneratedClass->GetDefaultObject();
    }
  }

  // 4. Encontrar Propriedade
  FProperty *Property = TargetObject->GetClass()->FindPropertyByName(*PropName);
  if (!Property) {
    return FString::Printf(
        TEXT("{ \"error\": \"Property '%s' not found on object\" }"),
        *PropName);
  }

  // 5. INICIAR TRANSACAO (Undo system)
  if (GEditor) {
    GEditor->BeginTransaction(FText::FromString(TEXT("LogosBridge AI Edit")));
  }

  TargetObject->Modify(); // Marca o objeto como dirty para transacao

  // 6. Aplicar Valor
  bool bSuccess = ApplyValue(TargetObject, Property, NewValue);

  // 7. FINALIZAR TRANSACAO
  if (GEditor) {
    if (bSuccess) {
      GEditor->EndTransaction();
      // Notificar mudança para atualizar editores abertos
      FPropertyChangedEvent PropertyChangedEvent(Property);
      TargetObject->PostEditChangeProperty(PropertyChangedEvent);

      // Se for CDO, marcar blueprint como dirty tambem
      if (TargetObject != LoadedObject) {
        LoadedObject->Modify();
      }
    } else {
      GEditor->CancelTransaction(0);
    }
  }

  if (bSuccess) {
    return TEXT("{ \"status\": \"success\" }");
  } else {
    return TEXT("{ \"error\": \"Failed to apply value (type mismatch?)\" }");
  }
}

bool FBlueprintEditHandler::ApplyValue(
    UObject *Object, FProperty *Property,
    const TSharedPtr<FJsonValue> &JsonValue) {
  // Float
  if (FFloatProperty *FloatProp = CastField<FFloatProperty>(Property)) {
    double Val;
    if (JsonValue->TryGetNumber(Val)) {
      FloatProp->SetPropertyValue_InContainer(Object, (float)Val);
      return true;
    }
  }
  // Int
  else if (FIntProperty *IntProp = CastField<FIntProperty>(Property)) {
    int32 Val;
    if (JsonValue->TryGetNumber(Val)) {
      IntProp->SetPropertyValue_InContainer(Object, (int32)Val);
      return true;
    }
  }
  // Bool
  else if (FBoolProperty *BoolProp = CastField<FBoolProperty>(Property)) {
    bool Val;
    if (JsonValue->TryGetBool(Val)) {
      BoolProp->SetPropertyValue_InContainer(Object, Val);
      return true;
    }
  }
  // String
  else if (FStrProperty *StrProp = CastField<FStrProperty>(Property)) {
    FString Val;
    if (JsonValue->TryGetString(Val)) {
      StrProp->SetPropertyValue_InContainer(Object, Val);
      return true;
    }
  }

  return false;
}

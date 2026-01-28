#pragma once

#include "CoreMinimal.h"

/**
 * Handles modification requests (Edit) via Reflection API with Transaction
 * support.
 */
class LOGOSBRIDGE_API FBlueprintEditHandler {
public:
  /**
   * Processa um pedido de edição JSON.
   * Formato esperado: { "asset": "/Game/Path", "property": "Name", "value": 123
   * } Deve ser executado na GameThread.
   */
  static FString ProcessEdit(const FString &JsonPayload);

private:
  static bool ApplyValue(UObject *Object, FProperty *Property,
                         const TSharedPtr<FJsonValue> &JsonValue);
};

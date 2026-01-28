#pragma once

#include "CoreMinimal.h"

/**
 * Handles asset creation, duplication, and management actions using AssetTools.
 */
class LOGOSBRIDGE_API FAssetActionHandler {
public:
  /**
   * Cria um novo asset de uma determinada classe.
   * JSON: { "path": "/Game/Folder/Name", "class": "DataAsset" }
   */
  static FString ProcessCreate(const FString &JsonPayload);

  /**
   * Duplica um asset existente.
   * JSON: { "source": "/Game/Source", "destination": "/Game/Dest" }
   */
  static FString ProcessDuplicate(const FString &JsonPayload);
};

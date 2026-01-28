#pragma once

#include "CoreMinimal.h"

/**
 * Handles introspection requests (Query) via Reflection API.
 */
class LOGOSBRIDGE_API FBlueprintQueryHandler {
public:
  /**
   * Busca dados de um Asset e retorna como JSON string.
   * Deve ser executado na GameThread.
   * @param AssetPath Caminho do pacote (ex: /Game/Blueprints/BP_Hero)
   */
  static FString ProcessQuery(const FString &AssetPath);

  /**
   * Lista todos os assets em um diretorio (Recursivo).
   * @param Path Caminho do diretorio (ex: /Game/Data)
   */
  static FString ProcessList(const FString &Path);

private:
  static TSharedPtr<FJsonObject> SerializeObject(UObject *Object);
};

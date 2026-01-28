#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PillarGenerator.generated.h"

class USceneComponent;
class UBillboardComponent;
class UHierarchicalInstancedStaticMeshComponent;

/**
 * ALogosPillarGenerator
 * Ator responsavel por gerar pilares procedurais no nivel.
 * Permite controle direto via Painel Details no Unreal Editor.
 */
UCLASS()
class LOGOSBRIDGE_API ALogosPillarGenerator : public AActor {
  GENERATED_BODY()

public:
  ALogosPillarGenerator();

  // --- Componentes ---

  UPROPERTY(VisibleAnywhere, Category = "Logos")
  USceneComponent *SceneRoot;

  UPROPERTY(VisibleAnywhere, Category = "Logos")
  UBillboardComponent *EditorIcon;

  // --- Parametros de Geracao ---

  UPROPERTY(EditAnywhere, Category = "Logos")
  int32 PillarCount = 200;

  UPROPERTY(EditAnywhere, Category = "Logos")
  float SpawnRadius = 20000.0f;

  UPROPERTY(EditAnywhere, Category = "Logos")
  float MinHeight = 250.0f;

  UPROPERTY(EditAnywhere, Category = "Logos")
  float MaxHeight = 1200.0f;

  UPROPERTY(EditAnywhere, Category = "Logos")
  float ZOffset = 0.0f;

  UPROPERTY(EditAnywhere, Category = "Logos")
  UStaticMesh *PillarMesh;

  // --- Comandos ---

  /** Gera os pilares baseados nos parametros atuais. Limpa os anteriores. */
  UFUNCTION(CallInEditor, Category = "Logos")
  void Generate();

  /** Limpa todos os pilares gerados por este ator. */
  UFUNCTION(CallInEditor, Category = "Logos")
  void Clear();

protected:
  virtual void BeginPlay() override;

  UPROPERTY(VisibleAnywhere, Category = "Logos")
  UHierarchicalInstancedStaticMeshComponent *InstancedMesh;
};

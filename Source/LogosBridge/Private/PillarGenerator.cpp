#include "PillarGenerator.h"
#include "Components/BillboardComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"

ALogosPillarGenerator::ALogosPillarGenerator() {
  PrimaryActorTick.bCanEverTick = false;

  // 1. Criar Componentes de Editor
  SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
  RootComponent = SceneRoot;

  InstancedMesh =
      CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(
          TEXT("InstancedMesh"));
  InstancedMesh->SetupAttachment(RootComponent);

  EditorIcon = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorIcon"));
  EditorIcon->SetupAttachment(RootComponent);
  EditorIcon->bIsEditorOnly = true;

  // 2. Carregar mesh padrao se disponivel
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
      TEXT("/Game/LevelPrototyping/Meshes/SM_Cube"));
  if (MeshAsset.Succeeded()) {
    PillarMesh = MeshAsset.Object;
  }
}

void ALogosPillarGenerator::BeginPlay() { Super::BeginPlay(); }

void ALogosPillarGenerator::Generate() {
  Clear();

  if (!PillarMesh) {
    UE_LOG(LogTemp, Warning, TEXT("[LogosBridge] PillarMesh nao configurado!"));
    return;
  }

  if (!InstancedMesh)
    return;

  // Configurar o Mesh no componente HISM
  InstancedMesh->SetStaticMesh(PillarMesh);

  for (int32 i = 0; i < PillarCount; i++) {
    // 1. Calcular Posicao Aleatoria
    FVector Location = GetActorLocation();
    Location.X +=
        UKismetMathLibrary::RandomFloatInRange(-SpawnRadius, SpawnRadius);
    Location.Y +=
        UKismetMathLibrary::RandomFloatInRange(-SpawnRadius, SpawnRadius);

    float Height = UKismetMathLibrary::RandomFloatInRange(MinHeight, MaxHeight);

    // O Cubo padrao tem 100 unidades, pivo no centro.
    // Queremos a base no chao encontrado.
    float GroundZ = 0.0f;
    float ZScale = Height / 100.0f;

    // Ajuste: Removendo (Height * 0.5f) pois suspeita-se que o pivo ja seja na
    // base. Adicionando ZOffset manual para ajuste finos.
    Location.Z = GroundZ + ZOffset;

    // 2. Adicionar Instancia HISM
    FTransform InstanceTransform;
    InstanceTransform.SetLocation(Location);
    InstanceTransform.SetRotation(FQuat::Identity);
    InstanceTransform.SetScale3D(FVector(2.0f, 2.0f, ZScale));

    InstancedMesh->AddInstance(InstanceTransform, true);
  }

  UE_LOG(LogTemp, Log, TEXT("[LogosBridge] %d instancias geradas via HISM."),
         PillarCount);
}

void ALogosPillarGenerator::Clear() {
  if (InstancedMesh) {
    InstancedMesh->ClearInstances();
  }
}

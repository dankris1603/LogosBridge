# 🧠 Handler Reference: Deep Dive Técnica

Os "Handlers" são o núcleo operacional do LogosBridge. Enquanto o `HttpServerManager` cuida do transporte (IO), os Handlers executam a lógica de engine (Game Thread).

Esta documentação detalha a implementação interna de cada handler.

---

## 1. AssetActionHandler (`AssetActionHandler.cpp`)

Este handler manipula o **Asset Registry** e o **Content Browser**. Ele não opera sobre atores da cena, mas sobre arquivos em disco (`.uasset`).

### Como Funciona

#### Criação de Assets (`ProcessCreate`)

1. **Resolução de Classe**: Recebe o nome da classe (ex: "MyDataAsset"). Tenta encontrar a classe em memória usando `FindObject<UClass>` ou carrega do disco com `StaticLoadClass`.
2. **Seleção de Factory**:
   - O Unreal exige uma `UFactory` para criar assets.
   - Para classes genéricas, o handler tenta usar a factory padrão.
   - **Caso Especial (Data Assets):** Se a classe alvo herda de `UDataAsset`, o handler instancia explicitamente um `UDataAssetFactory` e injeta a classe alvo na propriedade `DataAssetClass`.
3. **Instanciação**: Chama `IAssetTools::Get().CreateAsset()`. Isso garante que o asset seja registrado corretamente no AssetRegistry e marcado como "sujo" para salvamento.

#### Duplicação (`ProcessDuplicate`)

- Usa `StaticLoadObject` para carregar o original.
- Chama `IAssetTools::Get().DuplicateAsset()`.
- **Nota:** Isso cria uma cópia profunda, preservando referências internas sempre que possível.

---

## 2. BlueprintQueryHandler (`BlueprintQueryHandler.cpp`)

Responsável pela **Introspecção** (Reflection). Ele permite que a IA "leia" a memória do Unreal.

### Lógica de Resolução (O Segredo do CDO)

Um erro comum ao ler Blueprints é inspecionar o objeto `UBlueprint` em si. O LogosBridge evita isso:

1. Carrega o `UBlueprint`.
2. Acessa o `GeneratedClass` (o código compilado do BP).
3. Obtém o **CDO (Class Default Object)** via `GetDefaultObject()`.
   - _Por que?_ O CDO contém os valores reais das variáveis padrões configuradas no editor. O `UBlueprint` contém apenas grafos e metadados.

### Iteração de Propriedades

O handler usa `TFieldIterator<FProperty>` para varrer a classe:

- **Filtros:** Ignora propriedades `Transient` ou `EditorOnly` para reduzir ruído JSON.
- **Tipagem:** Faz cast de `FProperty` para tipos específicos (`FFloatProperty`, `FIntProperty`, `FObjectProperty`, etc.) para extrair o valor correto da memória.

---

## 3. BlueprintEditHandler (`BlueprintEditHandler.cpp`)

O sistema mais crítico e complexo do plugin. Ele altera a memória do Unreal em tempo real, exigindo rigorosos controles de segurança e tipo.

### Sistema de Transação (Undo/Redo)

O Unreal Engine possui um histórico de transações robusto. Para que a IA não faça alterações destrutivas irreversíveis, envolvemos toda operação em uma transação oficial:

```cpp
// BeginTransaction cria um snapshot do estado atual
GEditor->BeginTransaction(TEXT("LogosBridge AI Edit"));

// Modify() marca o objeto C++ como "prestes a mudar" no sistema de transação
TargetObject->Modify();

// ... Alteração da propriedade ocorre aqui ...

if (Success) {
    // Confirma a transação (adiciona à pilha de Undo)
    GEditor->EndTransaction();
} else {
    // Reverte silenciosamente se algo deu errado
    GEditor->CancelTransaction(0);
}
```

Isso permite que o usuário pressione **Ctrl+Z** no Editor para reverter instantaneamente uma edição feita remotamente via API.

### Mecânica de Tipagem (Type Safety)

O handler não tenta "adivinhar" tipos. Ele usa o sistema de Reflection (`FProperty`) para verificar estritamente se o valor JSON corresponde ao tipo da variável C++.

| Tipo Unreal        | Método de Verificação (Cast) | Método de Aplicação                             |
| :----------------- | :--------------------------- | :---------------------------------------------- |
| **Float / Double** | `CastField<FFloatProperty>`  | `SetPropertyValue_InContainer(Obj, (float)Val)` |
| **Integer**        | `CastField<FIntProperty>`    | `SetPropertyValue_InContainer(Obj, (int32)Val)` |
| **Boolean**        | `CastField<FBoolProperty>`   | `SetPropertyValue_InContainer(Obj, Val)`        |
| **String**         | `CastField<FStrProperty>`    | `SetPropertyValue_InContainer(Obj, Val)`        |

_Nota: Atualmente, Structs complexas e Arrays não são suportados diretamente via endpoint genérico `/edit`, exigindo lógica customizada._

### Propagação de Mudanças (Dirty State)

Alterar o valor na memória RAM não é suficiente para o Editor. O sistema de UI precisa ser notificado para atualizar painéis e marcar o arquivo para salvamento.

1. **Notificação de Propriedade:**

   ```cpp
   FPropertyChangedEvent PropertyChangedEvent(Property);
   TargetObject->PostEditChangeProperty(PropertyChangedEvent);
   ```

   Isso força o painel _Details_ a repintar o slider/texto com o novo valor.

2. **Propagação CDO -> Blueprint:**
   Como estamos editando o _Class Default Object_ (CDO) — que é uma instância interna escondida — precisamos avisar o asset pai (`UBlueprint`) que algo mudou.
   ```cpp
   if (TargetObject != LoadedObject) {
       LoadedObject->Modify(); // Marca o arquivo .uasset do Blueprint com asterisco (*)
   }
   ```

---

## Resumo Técnico

| Handler            | Escopo               | Segurança              | Ferramentas Unreal Usadas                                  |
| :----------------- | :------------------- | :--------------------- | :--------------------------------------------------------- |
| **AssetAction**    | Arquivos (`.uasset`) | Safe                   | `AssetTools`, `Factory`, `AssetRegistry`                   |
| **BlueprintQuery** | Leitura (Read-Only)  | N/A                    | `Reflection System`, `CDO`, `TFieldIterator`               |
| **BlueprintEdit**  | Escrita (Write)      | **Transaction (Undo)** | `GEditor`, `Modify`, `PostEditChangeProperty`, `CastField` |

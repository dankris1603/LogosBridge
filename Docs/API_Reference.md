# 🔌 LogosBridge API Reference (Deep Dive)

Documentação técnica oficial para interação com o plugin LogosBridge.

**Base URL:** `http://localhost:8080/api`  
**Content-Type:** `application/json` (Exceto onde notado)  
**Encoding:** UTF-8

---

## 🟢 Core System

### 1. Health Check

Verifica conectividade e disponibilidade do servidor. Útil para _polling_ inicial.

- **Endpoint:** `GET /health`
- **Response (200 OK):**
  ```json
  {
    "status": "alive",
    "identity": "LogosBridge"
  }
  ```

### 2. Run Python Script

Permite Injeção de Código Arbitrário. O corpo da requisição é tratado como um arquivo `.py` bruto.

- **Segurança:** O código roda com privilégios de Editor. Use com cuidado.

* **Endpoint:** `POST /run_script`
* **Headers:** `Content-Type: text/plain` (Recomendado, mas opcional)
* **Body:** String contendo código Python válido.
  ```python
  import unreal
  unreal.log("Script remoto executado!")
  ```
* **Response (200 OK):**
  ```json
  {
    "status": "script_executed",
    "path": "F:/.../Saved/TempLogosScript.py"
  }
  ```
  _Nota: O script é salvo em disco na pasta `Saved` antes de rodar._

---

## 🔍 Discovery (Leitura)

### 3. List Assets

Escaneia um diretório do projeto e retorna todos os assets encontrados. Otimizado para não carregar objetos na memória, apenas lê o registro.

- **Endpoint:** `GET /list`
- **Query Params:**
  - `path`: O caminho virtual do Unreal (ex: `/Game/Textures`).
- **Response (200 OK):**
  ```json
  {
    "count": 2,
    "assets": [
      {
        "name": "T_Hero_D",
        "path": "/Game/Textures/T_Hero_D",
        "class": "/Script/Engine.Texture2D"
      },
      {
        "name": "T_Hero_N",
        "path": "/Game/Textures/T_Hero_N",
        "class": "/Script/Engine.Texture2D"
      }
    ]
  }
  ```
- **Error (200 OK - Soft Error):**
  ```json
  { "error": "Param 'path' is required" }
  ```

### 4. Query Blueprint (Introspection)

Realiza engenharia reversa em um Asset para expor suas propriedades internas.

- **Suporta:** Blueprints (lê o CDO), Data Assets, e Atores no Mundo.

* **Endpoint:** `GET /query`
* **Query Params:**
  - `asset`: Caminho do pacote (ex: `/Game/BP_Player`) OU caminho de objeto (ex: `/Game/Maps/Level1:PersistentLevel.PillarGenerator`).
* **Response (200 OK):**
  ```json
  {
    "name": "Default__BP_Player_C",
    "class": "BP_Player_C",
    "Health": 100.0,
    "MaxSpeed": 600.0,
    "IsDead": false,
    "CharacterName": "Hero",
    "functions": ["Jump", "Fire", "Die"]
  }
  ```

---

## ✏️ Manipulation (Escrita)

### 5. Create Asset

Cria um novo arquivo `.uasset` no disco.

- **Endpoint:** `POST /create`
- **Body (JSON):**
  | Campo | Tipo | Descrição |
  | :--- | :--- | :--- |
  | `path` | String | Caminho completo + Nome (ex: `/Game/Data/DA_MyConfig`). |
  | `class` | String | Nome da classe C++ ou Blueprint Pai (ex: `LogosData`, `UDataAsset`). |

- **Exemplo:**
  ```json
  {
    "path": "/Game/Data/NewItem",
    "class": "UDataAsset"
  }
  ```
- **Response (200 OK):**
  ```json
  { "status": "success", "path": "/Game/Data/NewItem.NewItem" }
  ```

### 6. Duplicate Asset

Clona um asset existente (Deep Copy).

- **Endpoint:** `POST /duplicate`
- **Body (JSON):**
  | Campo | Tipo | Descrição |
  | :--- | :--- | :--- |
  | `source` | String | Caminho do asset original. |
  | `destination` | String | Caminho completo do novo asset. |

- **Exemplo:**
  ```json
  {
    "source": "/Game/Templates/BP_BaseEnemy",
    "destination": "/Game/Enemies/Forest/BP_Goblin"
  }
  ```

### 7. Edit Property

**[CRÍTICO]** Modifica valor na memória e propaga mudança para UI e Arquivo.

- **Endpoint:** `POST /edit`
- **Body (JSON):**
  | Campo | Tipo | Descrição |
  | :--- | :--- | :--- |
  | `asset` | String | Caminho do objeto alvo. |
  | `property` | String | Nome exato da variável (Case Sensitive). |
  | `value` | Mixed | O novo valor (deve casar com o tipo C++). |

- **Tipos Suportados:**
  - `float`, `int`, `bool`, `string`.

- **Exemplo:**

  ```json
  {
    "asset": "/Game/Blueprints/BP_Config.BP_Config_C",
    "property": "DifficultyMultiplier",
    "value": 2.5
  }
  ```

- **Response (200 OK):**
  ```json
  { "status": "success" }
  ```
- **Erro comum:**
  ```json
  { "error": "Property 'X' not found on object" }
  ```

---

## 🛡️ Códigos de Erro

A API tende a retornar `200 OK` mesmo para erros lógicos (soft errors), contendo um campo `"error"` no JSON.

- **HTTP 200** `{ "status": "success" }`: Operação OK.
- **HTTP 200** `{ "error": "..." }`: Falha lógica (Asset não encontrado, JSON inválido).
- **HTTP 500**: Crash ou exceção não tratada no servidor C++ (Raro).

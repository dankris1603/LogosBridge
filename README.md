# 🌊 LogosBridge

**A Ponte entre Mente (IA) e Matéria (Unreal Engine).**

> _LogosBridge é um plugin para Unreal Engine 5 que expõe uma API HTTP de alta performance (C++) para permitir que inteligências externas (Python, LLMs) controlem o Editor em tempo real._

## 🚀 Funcionalidades

- **🎛️ Controle Total via HTTP**: API REST na porta 8080 para criar, editar e consultar assets.
- **🏛️ Pillar Generator (HISM)**: Exemplo prático de geração procedural capaz de spawnar 6.000+ instâncias a 60 FPS.
- **🐍 Python Bridge**: Injete scripts Python arbitrários diretamente no contexto do Editor via API. (pedir para a IA gerar os scripts e inseri-lo manualmente no unreal parece ser mais rapido).
- **🛡️ Undo/Redo Seguro**: Todas as operações API são transacionadas, permitindo `Ctrl+Z`.
- **👁️ Blueprint Introspection**: Habilidade de ler variáveis e estruturas de Blueprints e CDOs.

## 📦 Instalação Rápida

1.  Baixe este repositório.
2.  Copie a pasta `LogosBridge` para `SeuProjeto/Plugins/`.
3.  Adicione as dependências no seu `.uproject`:
    ```json
    "Plugins": [
        { "Name": "LogosBridge", "Enabled": true },
        { "Name": "PythonScriptPlugin", "Enabled": true },
        { "Name": "EditorScriptingUtilities", "Enabled": true }
    ]
    ```
4.  Delete as pastas `Binaries/` e `Intermediate/` do seu projeto.
5.  Reabra o projeto e clique em **Yes** para recompilar.

👉 [Guia de Instalação Detalhado](Docs/Installation_Guide.md)

## 🎮 Como Usar

### 1. Iniciar o Servidor

O servidor inicia automaticamente com o Editor. Verifique o **Output Log**:
`[LogosBridge] Servidor Online!`

### 2. Exemplo de Requisição (Python Externo)

```python
import requests

# Criar um novo Data Asset
response = requests.post("http://localhost:8080/api/create", json={
    "path": "/Game/Data/MyNewAsset",
    "class": "UDataAsset"
})
print(response.json())
```

### 3. Usando o Gerador de Pilares

1. Arraste `LogosPillarGenerator` para a cena.
2. No painel **Details**, selecione um Mesh (cubo).
3. Defina `Pillar Count` como 5000.
4. Clique em **Generate**.

## 📚 Documentação

- [Guia de Arquitetura](Docs/Architecture.md)
- [Referência de API](Docs/API_Reference.md)
- [Referência dos Handlers C++](Docs/Handler_Reference.md)
- [Manual de Ferramentas](Docs/Tools_Manual.md)

---

_Desenvolvido com 💎 por Aqua Sophia._

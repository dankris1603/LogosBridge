# 📦 Guia de Instalação: LogosBridge

Este guia descreve o processo passo a passo para integrar o plugin LogosBridge em um novo projeto Unreal Engine (5.x).

## ✅ Pré-requisitos

1.  **Unreal Engine 5.0+** instalado.
2.  **Visual Studio 2022** (ou compatível) com as cargas de trabalho "Game Development with C++".
3.  O código fonte do plugin `LogosBridge` (pasta contém `LogosBridge.uplugin`, `Source`, `Resources`).

---

## 🚀 Passo 1: Preparação do Projeto

Se você já tem um projeto C++, pule para o Passo 2.

1.  Abra o Unreal Engine.
2.  Crie um novo projeto **C++** (ex: `ThirdPerson` ou `Blank`).
    - _Nota: Projetos Blueprint-only precisam ser convertidos para C++ para compilar plugins customizados._
3.  Feche o Editor após criar o projeto.

---

## 📂 Passo 2: Copiar o Plugin

1.  Navegue até a pasta raiz do seu projeto (onde fica o arquivo `.uproject`).
2.  Crie uma pasta chamada `Plugins` (se não existir).
3.  Copie a pasta inteira `LogosBridge` para dentro de `Plugins`.
    - Caminho final deve ser: `.../SeuProjeto/Plugins/LogosBridge/LogosBridge.uplugin`

---

## ⚙️ Passo 3: Configurar Dependências

O LogosBridge depende do sistema Python do Unreal. Você precisa ativá-lo implicitamente ou explicitamente.

1.  Abra o arquivo `.uproject` do seu projeto com um editor de texto (Notepad, VSCode).
2.  Procure a seção `"Plugins"`. Adicione ou verifique se as seguintes entradas existem:

```json
"Plugins": [
    {
        "Name": "LogosBridge",
        "Enabled": true
    },
    {
        "Name": "PythonScriptPlugin",
        "Enabled": true
    },
    {
        "Name": "EditorScriptingUtilities",
        "Enabled": true
    }
]
```

---

## 🔨 Passo 4: Compilação

Como o LogosBridge é um plugin C++, ele precisa ser compilado. A geração de arquivos do Visual Studio pode falhar dependendo do ambiente, então recomendamos **uma das opções abaixo**:

### 🅰️ Opção A: Recompilação Automática (Recomendado)

Esta é a forma mais simples e não exige abrir o Visual Studio manualmente.

1.  Vá até a pasta raiz do seu projeto.
2.  **Delete** as pastas `Binaries` e `Intermediate`.
3.  Dê um duplo clique no arquivo `SeuProjeto.uproject`.
4.  O Unreal exibirá uma janela de aviso:
    > _"The following modules are missing or built with a different engine version... Would you like to rebuild them now?"_
5.  Clique em **Yes**.
6.  Aguarde. O Unreal compilará o plugin e abrirá o Editor automaticamente.

### 🅱️ Opção B: Via Terminal (Robusto)

Use esta opção se a Opção A falhar. Ela usa o script oficial de build da Engine.

1.  Abra o **PowerShell** na pasta do seu projeto.
2.  Execute o comando abaixo (ajuste o caminho da Engine e nome do projeto):

```powershell
# Exemplo (Ajuste os caminhos!):
& "C:\Program Files\Epic Games\UE_5.x\Engine\Build\BatchFiles\Build.bat" NomeDoProjetoEditor Win64 Development "C:\Projetos\SeuProjeto\SeuProjeto.uproject" -waitmutex
```

3.  Se terminar com `BUILD SUCCESSFUL`, o plugin está pronto.

---

## 🔌 Passo 5: Validação

1.  Abra o projeto no Unreal Editor (clique duplo no `.uproject` ou F5 no Visual Studio).
2.  Vá em **Edit > Plugins** e confirme se `LogosBridge` está ativado.
3.  Abra o **Output Log** (Window > Output Log).
4.  Procure pela mensagem:
    `[LogosBridge] Servidor Online!` ou `[LogosBridge] Iniciando Servidor HTTP na porta 8080...`
5.  Teste a conexão abrindo um navegador e acessando:
    `http://localhost:8080/api/health`
    - Resposta esperada: `{ "status": "alive", "identity": "LogosBridge" }`

---

## ⚠️ Troubleshooting

**Erro: "The following modules are missing or built with a different engine version..."**

- Isso significa que você copiou o plugin mas não recompilou. Siga o Passo 4 rigorosamente.

**Erro: Porta 8080 ocupada**

- Verifique se outro software (Tomcat, Jenkins, outro Editor aberto) não está usando a porta.
- Atualmente a porta é hardcoded no `HttpServerManager.h`. Para mudar, edite o C++ e recompile.

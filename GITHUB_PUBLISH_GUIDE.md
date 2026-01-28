# 🐙 Como Publicar no GitHub

Este guia ensina como transformar esta pasta local em um repositório GitHub público.

## 1. Criar o Repositório Remoto

1. Acesse [github.com/new](https://github.com/new).
2. Nomeie o repositório como `LogosBridge`.
3. Deixe como **Public**.
4. **Não** marque "Add a README" (já criamos um).
5. Clique em **Create repository**.

## 2. Preparar o Local (Git Init)

Abra o terminal (Git Bash ou PowerShell) nesta pasta `F:\Geral Novo\Projetos\plugin LogosBridge` e rode os comandos:

```bash
# 1. Inicializar o Git
git init

# 2. Conectar com o GitHub ( SUBSTITUA A URL ABAIXO PELA SUA! )
git remote add origin https://github.com/SEU_USUARIO/LogosBridge.git
# Exemplo: git remote add origin https://github.com/dani/LogosBridge.git

# 3. Adicionar arquivos (O .gitignore vai bloquear os lixos)
git add .

# 4. Criar o primeiro commit
git commit -m "Initial commit: LogosBridge Plugin v1.0"

# 5. Enviar para a nuvem
git branch -M main
git push -u origin main
```

## ⚠️ Dica Importante

O arquivo `.gitignore` que eu criei já impede que pastas pesadas (`Binaries`, `Intermediate`) subam. Isso mantém o upload rápido e profissional.

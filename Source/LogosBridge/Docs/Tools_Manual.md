# 📖 Manual de Ferramentas Procedurais

## Gerador de Pilares (HISM Edition) - `ALogosPillarGenerator`

Uma ferramenta de nível profissional para espalhamento de geometria vertical com performance extrema.

### 🚀 Alta Performance (HISM)

Diferente de geradores comuns que criam um Ator para cada objeto, este gerador usa **Hierarchical Instanced Static Meshes**.

- **Capacidade:** Testado com 6.000+ instâncias a 60 FPS.
- **Memória:** Custo de memória reduzido em 90% comparado a atores individuais.
- **Draw Calls:** Apenas 1 chamada de desenho para todos os pilares.

### 🎛️ Parâmetros do Painel

| Propriedade        | Descrição                                    | Valor Recomendado                           |
| :----------------- | :------------------------------------------- | :------------------------------------------ |
| **Pillar Mesh**    | O modelo 3D a ser replicado.                 | `SM_Cube` ou `SM_Pillar`                    |
| **Pillar Count**   | Quantidade total de instâncias.              | 500 - 10.000                                |
| **Spawn Radius**   | Área circular de geração em volta do ator.   | 20.000 (200m)                               |
| **Min/Max Height** | Variação de escala Z (aleatória).            | 200 - 1500                                  |
| **Z Offset**       | Ajuste vertical fino (fixo em Z=0 absoluto). | 0.0 (Ajuste se o mesh tiver pivô deslocado) |

### 🎮 Como Usar

1. **Colocação**: Arraste a classe `LogosPillarGenerator` do Content Browser (C++ Classes) para o nível.
2. **Configuração**: No painel **Details**, ajuste o `Pillar Mesh` (obrigatório) e o `Pillar Count`.
3. **Geração**: Clique no botão **Generate**.
4. **Limpeza**: Clique no botão **Clear** para remover apenas as instâncias geradas (preserva outros atores).
5. **Ghostbusters**: Se houver atores legados (sistema antigo), o `Clear` também os removerá.

### ⚠️ Troubleshooting

- **Pilares Flutuando?** Ajuste o `Z Offset`. O sistema alinha o pivô do mesh com o Z=0 global.
- **Crash no Generate?** Reduza o `Pillar Count` se estiver acima de 100.000 (limite de hardware, não do plugin).

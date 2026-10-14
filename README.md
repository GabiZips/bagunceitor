# T1 - Programação de Baixo Nivel

## "Bagunceitor"

# Bagunceitor

Operações reversíveis:

- **Confusão:** modifica os valores das cores dos pixels.
- **Difusão:** troca blocos da imagem de posição, formando um mosaico.

Depois, o programa desfaz as duas operações e compara a imagem recuperada com a original.

## Execução

```bash
./bagunceitor loboguara.png
```

## Arquivos gerados

| Arquivo | Resultado |
|---|---|
| `mosaico.png` | Somente os blocos trocados |
| `cores.png` | Somente as cores alteradas |
| `output.png` | Blocos trocados e cores alteradas |
| `recuperada.png` | Imagem após desfazer as transformações |

## Funcionamento

### Mosaico

A imagem é dividida em uma grade de 6 × 6, totalizando 36 blocos.
Um vetor guarda o destino de cada bloco.

Os destinos são embaralhados por trocas de posições, sem repetição.
Para recuperar a organização original, o programa utiliza o mesmo mapa
para copiar os blocos de volta.

As sobras nas bordas, quando as dimensões não são divisíveis por 6,
mantêm suas posições.

### Cores

Os canais vermelho, verde e azul recebem deslocamentos calculados
a partir da posição do pixel. O operador `% 256` mantém os valores
entre 0 e 255.

A recuperação subtrai os mesmos deslocamentos.

### Recuperação

As operações são desfeitas na ordem contrária:

Original → alterar cores → aplicar mosaico
Bagunçada → desfazer mosaico → recuperar cores

A função `imagens_iguais` compara todos os canais RGB de todos os pixels.
A impressão de uma região de 8 × 8 pixels no terminal ajuda a acompanhar
as mudanças em cada etapa.

## Uso de IA

Foi utilizado IA para explicar conceitos de C, discutir
estratégias, gerar e modificar trechos de código, auxiliar nos testes (teste de mesa)
e elaborar textos de documentação. (Como este README e o relátorio)

## Integrantes

- Gabriel Zanini da Silva
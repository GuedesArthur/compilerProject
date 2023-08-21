# Compiladores 2023.2
## Projeto final
### Arthur Navarro Guedes - 11085314

## Linguagem utilizada
#### C++17
## Itens obrigatórios
- [x] Possui 2 tipos de variáveis
- [x] Possui a estrutura if.. else
- [x] Possui a estrutura de controle while/do while
- [x] Operações aritméticas executadas corretamente
- [x] Atribuições realizadas corretamente
- [x] Possui operação de Entrada e Saída
- [x] Aceita números decimais
- [x] Verificar se a variável já foi previamente declarada
- [x] Verificar se a variável foi declarada e não foi usada
- [x] Verificar se uma variável está sendo usada sem ter valor inicial

## Itens opcionais
- [x] Operadores lógicos ('e' e 'ou')
- [x] Geração de várias linguagens-alvo (C e Lua)
- [x] Auto execução do código (-autorun)
- [x] Constantes hexadecimais

## Como compilar o código
Certifique que você tenha instalados o programa ***CMake*** e um compilador com suporte a ***C++17***.

Vá até a pasta clonada, e digite
`sudo cmake`

Depois, digite:
`sudo make`

Será gerado, então, o arquivo ***zCompiler***.

## Usando o compilador
Para compilar um arquivo *.isi*, execute o programa passando o endereço do arquivo como argumento:
`sudo ./zCompiler input.isi`

O programa também oferece as seguintes flags como opção de execução:
| Flag |Atributo|
|-|-|
| -lua |Compila o código em Lua, ao invés de C.|
| -autorun|Executa o código após sua compilação|
|-token|Gera um arquivo listando todos os tokens|

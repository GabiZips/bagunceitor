/*

Para compilar de dentro do VS Code:

Windows/Linux: CTRL+SHIFT+B (Terminal -> Run Build Task)
macOS: COMMAND+SHIFT+B

Para executar de dentro do VS Code:

Executar normalmente: CTRL+F5 (Run -> Run Without Debugging)
Debugar: F5 (Run -> Start Debugging)

Pelo terminal:

Windows: mingw32-make
Linux/macOS: make

Para executar:

./bagunceitor [arquivo com a imagem de entrada]

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strings
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>
#include <stb_image_write.h>

// Um pixel Pixel (24 bits)
typedef struct {
    unsigned char r, g, b;
} Pixel;

// Uma imagem Pixel
typedef struct {
    int width, height; // largura, altura
    int channels;      // qtd de canais (geralmente 3, RGB)
    Pixel *pixels;
} Img;

// As 2 imagens
Img in, out;

// Protótipos
void load(char *name, Img *pic);

// Regra para embaralhar o mosaico
void regra_embaralhamento(int *mapa);

// Aplicar mosaico e desembaralhar
void aplicar_mosaico(Img *entrada, Img *saida, const int *mapa);
void desembaralhar_mosaico(Img *entrada, Img *saida, const int *mapa);

// Aplicar confusão de cores e arrumar ela
void aplicar_confusao(Img *imagem);
void desfazer_confusao(Img *imagem);

// Validação de imagem igual
int imagens_iguais(Img *original, Img *recuperada);

// Main
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("bagunceitor [origem]\n");
        exit(1);
    }

    // Carrega a imagem original
    load(argv[1], &in);

    // Exibe as dimensões na tela, para conferência
    printf("Origem   : %s %d x %d\n", argv[1], in.width, in.height);

    printf("Processando...\n");

    // Cria imagem de saída e "zera" ela
    int tam = in.width * in.height;
    out = in;
    out.pixels = malloc(tam * sizeof(Pixel));
    Img recuperada = in;
    recuperada.pixels = malloc(tam * sizeof(Pixel));

    if (out.pixels == NULL || recuperada.pixels == NULL) {
        printf("Erro ao alocar memoria\n");
        free(in.pixels);
        free(out.pixels);
        free(recuperada.pixels);
        return 1;
    }
    memset(out.pixels, 0, tam * sizeof(Pixel));

    // Converte para interpretar como matrizes
    Pixel(*pin)[in.width] = (Pixel(*)[in.width])in.pixels;
    Pixel(*pout)[in.width] = (Pixel(*)[in.width])out.pixels;

    // Copia a original para poder comparar no final.
    for (int y = 0; y < in.height; y++) {
        for (int x = 0; x < in.width; x++) {
            int posicao = y * in.width + x;
            recuperada.pixels[posicao] = pin[y][x];
        }
    }

    /* Mapa para o mosaico, defini que são 36 (6x6) quadrados de mosaico.
    Cada indice do vetor é um bloco do mosaico, e o valor o destino.
    mapa[0] = 10 -> o bloco 0 vai para o bloco 10*/
    int mapa[36];
    srand(time(NULL)); /* <- AUXILIO DE IA | random seed usando o tempo atual em segundos,
    a ideia era gerar mosaico aleatorio*/
    regra_embaralhamento(mapa);

    // Aplica o mosaico com os blocos trocados
    aplicar_mosaico(&in, &out, mapa);

    // Mostra as mesmas posicoes de 8 x 8 pixels para comparar as etapas
    printf("\nDepois do mosaico:\n");
    for (int y = 0; y < 8 && y < out.height; y++) {
        for (int x = 0; x < 8 && x < out.width; x++) {
            int posicao = y * out.width + x;
            printf("[%02X %02X %02X] ", out.pixels[posicao].r, out.pixels[posicao].g,
                   out.pixels[posicao].b);
        }
        printf("\n");
    }

    // Salva o mosaico.png
    int salvou_mosaico = stbi_write_png("mosaico.png", out.width, out.height, 3, out.pixels, 0);

    // Altera as cores da imagem original (NAO MOSAICO)
    aplicar_confusao(&recuperada);

    // Mostra as mesmas posicoes de 8 x 8 pixels para comparar as etapas
    printf("\nDepois de alterar somente as cores:\n");
    for (int y = 0; y < 8 && y < recuperada.height; y++) {
        for (int x = 0; x < 8 && x < recuperada.width; x++) {
            int posicao = y * recuperada.width + x;
            printf("[%02X %02X %02X] ", recuperada.pixels[posicao].r, recuperada.pixels[posicao].g,
                   recuperada.pixels[posicao].b);
        }
        printf("\n");
    }

    // Salva a imagem cores.png onde somente as cores foram aplicadas
    int salvou_cores =
        stbi_write_png("cores.png", recuperada.width, recuperada.height, 3, recuperada.pixels, 0);

    // Combina as cores alteradas com a troca dos blocos
    aplicar_mosaico(&recuperada, &out, mapa);

    // Mostra as mesmas posicoes de 8 x 8 pixels para comparar as etapas
    printf("\nDepois do mosaico e das cores :\n");
    for (int y = 0; y < 8 && y < out.height; y++) {
        for (int x = 0; x < 8 && x < out.width; x++) {
            int posicao = y * out.width + x;
            printf("[%02X %02X %02X] ", out.pixels[posicao].r, out.pixels[posicao].g,
                   out.pixels[posicao].b);
        }
        printf("\n");
    }

    // Grava a imagem como PNG para registro
    int salvou_output = stbi_write_png("output.png", out.width, out.height, 3, pout, 0);

    // Desfaz as transformacoes na ordem contraria
    desembaralhar_mosaico(&out, &recuperada, mapa);

    // Mostra as mesmas posicoes de 8 x 8 pixels para comparar as etapas
    printf("\nDepois de desfazer o mosaico (cores ainda alteradas): \n");
    for (int y = 0; y < 8 && y < recuperada.height; y++) {
        for (int x = 0; x < 8 && x < recuperada.width; x++) {
            int posicao = y * recuperada.width + x;
            printf("[%02X %02X %02X] ", recuperada.pixels[posicao].r, recuperada.pixels[posicao].g,
                   recuperada.pixels[posicao].b);
        }
        printf("\n");
    }

    desfazer_confusao(&recuperada);

    // Mostra as mesmas posicoes de 8 x 8 pixels para comparar as etapas
    printf("\nDepois de recuperar as cores (imagem recuperada): \n");
    for (int y = 0; y < 8 && y < recuperada.height; y++) {
        for (int x = 0; x < 8 && x < recuperada.width; x++) {
            int posicao = y * recuperada.width + x;
            printf("[%02X %02X %02X] ", recuperada.pixels[posicao].r, recuperada.pixels[posicao].g,
                   recuperada.pixels[posicao].b);
        }
        printf("\n");
    }

    int salvou_recuperada = stbi_write_png("recuperada.png", recuperada.width, recuperada.height, 3,
                                           recuperada.pixels, 0);

    int iguais = imagens_iguais(&in, &recuperada);
    if (iguais) {
        printf("Recuperacao exata, todos os canais RGB são iguais\n");
    } else {
        printf("Erro na recuperacao!\n");
    }

    if (!salvou_mosaico || !salvou_cores || !salvou_output || !salvou_recuperada) {
        printf("Erro ao salvar as imagens!\n");
    }

    free(in.pixels);
    free(out.pixels);
    free(recuperada.pixels);
    if (iguais && salvou_mosaico && salvou_cores && salvou_output && salvou_recuperada) {
        return 0;
    } else {
        return 1;
    }
}

void load(char *name, Img *pic) {
    pic->pixels = (Pixel *)stbi_load(name, &pic->width, &pic->height, &pic->channels, 0);
    if (!pic->pixels) {
        printf("Erro de leitura: %s\n", stbi_failure_reason());
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, pic->channels);
    // Exibe um bloco de 8 x 8 pixels em hexadecimal (teste)
    for (int i = 0; i < 8 && i < pic->height; i++) {
        for (int j = 0; j < 8 && j < pic->width; j++) {
            int posicao = i * pic->width + j;
            printf("[%02X %02X %02X] ", pic->pixels[posicao].r, pic->pixels[posicao].g,
                   pic->pixels[posicao].b);
        }
        printf("\n");
    }
}

void aplicar_mosaico(Img *entrada, Img *saida, const int *mapa) {
    // 6x6
    int blocos_x = 6;
    int blocos_y = 6;
    int total_blocos = blocos_x * blocos_y; // 36

    // Largura e altura em pixels
    int largura_bloco = entrada->width / blocos_x;
    int altura_bloco = entrada->height / blocos_y;

    // Copia a imagem pra saida antes de embaralhar
    for (int y = 0; y < entrada->height; y++) {
        for (int x = 0; x < entrada->width; x++) {
            int posicao = y * entrada->width + x;
            saida->pixels[posicao] = entrada->pixels[posicao];
        }
    }

    for (int bloco = 0; bloco < total_blocos; bloco++) {
        // Qual o destino do bloco atual
        int destino = mapa[bloco];

        // Onde cada bloco começa e termina
        int origem_x = (bloco % blocos_x) * largura_bloco;
        int origem_y = (bloco / blocos_x) * altura_bloco;

        int destino_x = (destino % blocos_x) * largura_bloco;
        int destino_y = (destino / blocos_x) * altura_bloco;

        for (int y = 0; y < altura_bloco; y++) {
            for (int x = 0; x < largura_bloco; x++) {
                int pos_origem = (origem_y + y) * entrada->width + origem_x + x;

                int pos_destino = (destino_y + y) * saida->width + destino_x + x;

                saida->pixels[pos_destino] = entrada->pixels[pos_origem];
            }
        }
    }
}

void desembaralhar_mosaico(Img *entrada, Img *saida, const int *mapa) {
    int blocos_x = 6;
    int blocos_y = 6;
    int total_blocos = blocos_x * blocos_y;

    int largura_bloco = entrada->width / blocos_x;
    int altura_bloco = entrada->height / blocos_y;

    for (int y = 0; y < entrada->height; y++) {
        for (int x = 0; x < entrada->width; x++) {
            int posicao = y * entrada->width + x;
            saida->pixels[posicao] = entrada->pixels[posicao];
        }
    }

    for (int bloco = 0; bloco < total_blocos; bloco++) {
        int embaralhado = mapa[bloco];

        int origem_x = (embaralhado % blocos_x) * largura_bloco;
        int origem_y = (embaralhado / blocos_x) * altura_bloco;

        int destino_x = (bloco % blocos_x) * largura_bloco;
        int destino_y = (bloco / blocos_x) * altura_bloco;

        for (int y = 0; y < altura_bloco; y++) {
            for (int x = 0; x < largura_bloco; x++) {
                int pos_origem = (origem_y + y) * entrada->width + origem_x + x;

                int pos_destino = (destino_y + y) * saida->width + destino_x + x;

                saida->pixels[pos_destino] = entrada->pixels[pos_origem];
            }
        }
    }
}

void regra_embaralhamento(int *mapa) {
    // Preenche o mapa
    for (int i = 0; i < 36; i++) {
        mapa[i] = i;
    }
    // Regra de substituição, troca os valores de duas posições
    for (int i = 35; i > 0; i--) {
        int j = rand() % (i + 1);

        int temporario = mapa[i];
        mapa[i] = mapa[j];
        mapa[j] = temporario;
    }
}

// Altera as cores sem perder os valores originais.
void aplicar_confusao(Img *imagem) {
    for (int y = 0; y < imagem->height; y++) {
        for (int x = 0; x < imagem->width; x++) {
            int posicao = y * imagem->width + x;

            // A posicao determina quanto somar em cada canal
            int chave = ((x % 256) + 3 * (y % 256) + 17) % 256;
            Pixel *pixel = &imagem->pixels[posicao];

            // % 256 mantem os valores entre 0 e 255
            pixel->r = (pixel->r + chave) % 256;
            pixel->g = (pixel->g + (chave + 53) % 256) % 256;
            pixel->b = (pixel->b + (chave + 101) % 256) % 256;
        }
    }
}

void desfazer_confusao(Img *imagem) {
    for (int y = 0; y < imagem->height; y++) {
        for (int x = 0; x < imagem->width; x++) {
            int posicao = y * imagem->width + x;
            int chave = ((x % 256) + 3 * (y % 256) + 17) % 256;
            Pixel *pixel = &imagem->pixels[posicao];

            pixel->r = (pixel->r - chave + 256) % 256;
            pixel->g = (pixel->g - (chave + 53) % 256 + 256) % 256;
            pixel->b = (pixel->b - (chave + 101) % 256 + 256) % 256;
        }
    }
}

int imagens_iguais(Img *original, Img *recuperada) {
    if (original->width != recuperada->width || original->height != recuperada->height) {
        return 0;
    }

    for (int y = 0; y < original->height; y++) {
        for (int x = 0; x < original->width; x++) {
            int posicao = y * original->width + x;
            Pixel a = original->pixels[posicao];
            Pixel b = recuperada->pixels[posicao];

            if (a.r != b.r || a.g != b.g || a.b != b.b) {
                return 0;
            }
        }
    }
    return 1;
}

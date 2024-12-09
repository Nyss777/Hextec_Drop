#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_W 500
#define SCREEN_H 600
#define PLATFORM_W 120
#define PLATFORM_H 40
#define STONE_W 30
#define STONE_H 30
#define MAX_STONES 10
#define FPS 60 // Frame rate

// Estrutura da plataforma
typedef struct {
    float x, y;             // Posição da plataforma
    ALLEGRO_BITMAP *sprite; // Imagem da plataforma
} Plataforma;

// Estrutura das pedras
typedef struct {
    float x, y;             // Posição da pedra
    ALLEGRO_BITMAP *sprite; // Imagem da pedra
    bool active;            // Se a pedra está ativa ou não
} Pedra;

// Função para verificar erros de inicialização
void error_h(bool test, const char *description)
{
    if (test) return;

    printf("Nao foi capaz de inicializar o/a %s\n", description);
    exit(1);
}

// Função para verificar colisão entre a plataforma e a pedra
bool verifica_colisao(Plataforma *plataforma, Pedra *pedra) {
    return (plataforma->x < pedra->x + STONE_W && plataforma->x + PLATFORM_W > pedra->x &&
            plataforma->y < pedra->y + STONE_H && plataforma->y + PLATFORM_H > pedra->y);
}

// Função principal
int main() {
    // Variável de delay entre os spawns das pedras
    float SPAWN_DELAY = 0.9;
    // Carregando a inicialização do Allegro
    if (!al_init()) {
        error_h(false, "Allegro");
    }
    // Carregando o teclado
    if (!al_install_keyboard()) {
        error_h(false, "Keyboard");
    }
    // Carregando a importação de imagens para os sprites da plataforma e das pedras e do background
    if (!al_init_image_addon()) {
        error_h(false, "Image Addon");
    }
    // Carregando a tela do display
    ALLEGRO_DISPLAY *display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display) {
        error_h(false, "Display");
    }
    // Carregando o timer 1/60
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
    if (!timer) {
        error_h(false, "Timer");
    }
    // Carregando o background
    ALLEGRO_BITMAP *background = al_load_bitmap("imagens/bg.jpg");
    if (!background) {
        error_h(false, "Imagem do Background");
    }
    // Carregando o ttf para fontes de maior qualidade e para aumentar o tamanho das letras
    if (!al_init_ttf_addon()) {
        error_h(false, "TTF Addon");
    }

    if (!al_install_audio()) {
        error_h(false, "Audio Addon");
    }

    if (!al_init_acodec_addon()) {
        error_h(false, "Audio Codec Addon");
    }

    if (!al_reserve_samples(1)) {
        error_h(false, "Reserva de Samples");
    }

    ALLEGRO_SAMPLE_INSTANCE *musica = al_load_sample("imagens/bg_music.wav");
    if (!musica) {
        error_h(false, "Musica de Fundo");
    }

    ALLEGRO_FONT *fonte = al_create_builtin_font();
    if (!fonte) {
        error_h(false, "Fonte padrão");
    }
    // Criação a fila de eventos
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());

    // Inicialização da plataforma
    Plataforma plataforma;
    plataforma.x = SCREEN_W / 2 - 50; // Posição inicial horizontal
    plataforma.y = SCREEN_H - 150;     // Posição inicial vertical
    plataforma.sprite = al_load_bitmap("imagens/professor.png"); // Carrega o sprite
    if (!plataforma.sprite) {
        error_h(false, "Imagem da Plataforma");
    }

    // Inicialização das pedras
    Pedra pedras[MAX_STONES];
    srand(time(NULL)); // Inicializa a semente para aleatoriedade
    for (int i = 0; i < MAX_STONES; i++) {
        pedras[i].x = -STONE_W;  // Começam fora da tela (acima)
        pedras[i].y = -STONE_H;  // Começam fora da tela
        pedras[i].active = false; // Inicialmente inativas
        pedras[i].sprite = al_load_bitmap("imagens/pedra.png"); // Carrega o sprite da pedra
        if (!pedras[i].sprite) {
            error_h(false, "Imagem da Pedra");
        }
    }

    // Variáveis principais
    bool running = true, redraw = true;

    float velocidadePlataforma = 0;
    float incrementoVelocidadePlataforma = 6;
    float velocidadePedra = 4;
    int pontuacao = 0;
    int pontuacaoRequerida = 10;
    float tempo_ultimo_spawn = 0; // Armazena o tempo desde o último spawn

    // Inicia o timer
    al_start_timer(timer);

    al_play_sample(musica, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);


    // Loop principal
    while (running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_TIMER) {
            tempo_ultimo_spawn += 1.0 / FPS;  // Incrementa o tempo a cada quadro

            // Verifica se é hora de spawnar uma nova pedra
            if (tempo_ultimo_spawn >= SPAWN_DELAY) {
                for (int i = 0; i < MAX_STONES; i++) {
                    if (!pedras[i].active) {
                        pedras[i].x = rand() % (SCREEN_W - STONE_W);  // Posição aleatória
                        pedras[i].y = -STONE_H;  // Começa fora da tela, no topo
                        pedras[i].active = true;  // Marca como ativa
                        break; // Spawnou uma pedra, sai do loop
                    }
                }
                tempo_ultimo_spawn = 0; // Reseta o tempo
            }

            // Atualiza a posição das pedras
            for (int i = 0; i < MAX_STONES; i++) {
                if (pedras[i].active) {
                    pedras[i].y += velocidadePedra; // As pedras caem para baixo

                    // Verifica se a pedra colidiu com a plataforma
                    if (verifica_colisao(&plataforma, &pedras[i])) {
                        pedras[i].active = false; // Desativa a pedra
                        pontuacao++;

                        if (pontuacao == pontuacaoRequerida) {
                            incrementoVelocidadePlataforma ++;
                            velocidadePedra += 0.5;
                            pontuacaoRequerida += 10;
                            SPAWN_DELAY -= 0.05;
                        }
                    }

                    // Se a pedra cair no chão, ela reinicia no topo
                    if (pedras[i].y > SCREEN_H) {
                        running = false;
                        pedras[i].active = false; // Desativa a pedra
                    }
                }
            }

            plataforma.x += velocidadePlataforma;
            // Limita a plataforma dentro da tela
            if (plataforma.x < 0) plataforma.x = 0;
            if (plataforma.x > SCREEN_W - PLATFORM_W) plataforma.x = SCREEN_W - PLATFORM_W;

            redraw = true;
        }
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            // Fecha a janela
            running = false;
        }
        else if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
            // Movimenta a plataforma com as setas
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                velocidadePlataforma = -incrementoVelocidadePlataforma; // Altera velocidade pra ser negativa
            } else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                velocidadePlataforma = incrementoVelocidadePlataforma; // Altera velocidade pra ser positiva
            }
        }
        else if (event.type == ALLEGRO_EVENT_KEY_UP) {
            // Quando uma tecla for solta, zera a velocidade
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT || event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                velocidadePlataforma = 0; // Zera a velocidade
            }
        }

        if (redraw && al_is_event_queue_empty(queue)) {

            redraw = false;

            al_clear_to_color(al_map_rgb(0, 0, 0)); // Limpa a tela com cor preta

            // Desenhe o background
            al_draw_bitmap(background, 0, 0, 0);

            // Desenha a plataforma
            al_draw_bitmap(plataforma.sprite, plataforma.x, plataforma.y, 0);

            // Desenha as pedras
            for (int i = 0; i < MAX_STONES; i++) {
                if (pedras[i].active) {
                    al_draw_bitmap(pedras[i].sprite, pedras[i].x, pedras[i].y, 0);
                }
            }
            // DESENHA A LABEL DE TEXTO
            al_draw_textf(fonte, al_map_rgb(255, 255, 255), 10, 10, 0, "Pontuacao: %d", pontuacao);

            al_flip_display(); // ATUALIZA A TELA
        }
    }

    // Libera recursos
    al_destroy_bitmap(plataforma.sprite);
    al_destroy_sample(musica);
    al_uninstall_audio();
    for (int i = 0; i < MAX_STONES; i++) {
        al_destroy_bitmap(pedras[i].sprite);
    }
    al_destroy_bitmap(background);
    al_destroy_font(fonte);
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}

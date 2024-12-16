#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

#define SCREEN_W 500
#define SCREEN_H 600
#define PLATFORM_W 120
#define PLATFORM_H 200
#define STONE_W 30
#define STONE_H 30
#define MAX_STONES 10
#define FPS 60
#define FONTSIZE 20

typedef struct {
    float x, y;             // Posi��o
    ALLEGRO_BITMAP *sprite; // Imagem do professor
} Professor;

typedef struct {
    float x, y;             // Posi��o
    ALLEGRO_BITMAP *sprite; // Imagem da pedra
    bool active;            // Se a pedra est� ativa ou n�o
} Pedra;

// Funcao para verficar erros de iniciaiza��o
void error_h(bool test, const char *description)
{
    if (test) return;

    printf("Nao foi capaz de inicializar o/a %s\n", description);
    exit(1);
}


bool verifica_colisao(Professor *professor, Pedra *pedra) {
    return (professor->x < pedra->x + STONE_W && professor->x + PLATFORM_W > pedra->x &&
            professor->y < pedra->y + STONE_H && professor->y + PLATFORM_H > pedra->y);
}

int main() {
    //delay entre os spawns das pedra
    float SPAWN_DELAY = 0.9;

    error_h(al_init(), "Allegro");
    error_h(al_install_keyboard(), "Keyboard");
    error_h(al_init_image_addon(), "Image Addon");

    setlocale(LC_ALL, "portuguese");
    al_init_font_addon();
    al_init_ttf_addon();

    ALLEGRO_DISPLAY *display = al_create_display(SCREEN_W, SCREEN_H);
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
    ALLEGRO_BITMAP *background = al_load_bitmap("imagens/bg.jpg");


    error_h(display, "Display");
    error_h(timer, "Timer");
    error_h(background, "Imagem do Background");
    error_h(al_install_audio(), "Audio Addon");
    error_h(al_init_acodec_addon(), "Audio Codec Addon");
    error_h(al_reserve_samples(3), "Reserva de Samples");

    ALLEGRO_SAMPLE *musica = al_load_sample("imagens/bg_music.wav");

    ALLEGRO_SAMPLE *sompedra = al_load_sample("imagens/stone.wav");

    ALLEGRO_SAMPLE *sompowerup = al_load_sample("imagens/powerup.wav");

    ALLEGRO_FONT *fonte = al_load_font("fonts/FONT.ttf", FONTSIZE, 0);

    error_h(musica, "Musica de Fundo");
    error_h(sompedra, "som de coletar pedra");
    error_h(sompowerup, "som de pontua��o atingida");
    error_h(fonte, "Fonte padr�o");

    // cria��o a fila de eventos
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());

    // inicializa��o do professor
    Professor professor;
    professor.x = SCREEN_W / 2 - 50; // Posi��o inicial horizontal
    professor.y = SCREEN_H - 150;     // Posi��o inicial vertical
    professor.sprite = al_load_bitmap("imagens/professor.png"); // Carrega o sprite
    error_h(professor.sprite, "Imagem da Professor");

    // Inicializa��o das pedras
    Pedra pedras[MAX_STONES];
    srand(time(NULL)); // Inicializa a semente (KKKK) para aleatoriedade
    for (int i = 0; i < MAX_STONES; i++) {
        pedras[i].x = -STONE_W;
        pedras[i].y = -STONE_H;
        pedras[i].active = false;
        pedras[i].sprite = al_load_bitmap("imagens/pedra.png");
        error_h(pedras[i].sprite, "Imagem da Pedra");
    }

    // Vari�veis principais
    bool running = true, redraw = true;

    float velocidadeProfessor = 0;
    float incrementoVelocidadeProfessor = 6;
    float velocidadePedra = 4;
    int pontuacao = 0;
    int pontuacaoRequerida = 10;
    float tempo_ultimo_spawn = 0; // Armazena o tempo desde o �ltimo spawn
    bool teclaEsquerdaPressionada = false;
    bool teclaDireitaPressionada = false;

    // Inicia o timer
    al_start_timer(timer);

    al_play_sample(musica, 0.3, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);


    // Loop principal
    while (running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);
        if (event.type == ALLEGRO_EVENT_TIMER) {
            tempo_ultimo_spawn += 1.0 / FPS;  // Incrementa o tempo ???

            // Verifica se � hora de spawnar uma nova pedra
            if (tempo_ultimo_spawn >= SPAWN_DELAY) {
                for (int i = 0; i < MAX_STONES; i++) {
                    if (!pedras[i].active) {
                        pedras[i].x = rand() % (SCREEN_W - STONE_W);  // Posi��o aleatoria
                        pedras[i].y = -STONE_H;  // Come�a fora da tela, no topo
                        pedras[i].active = true;  // Marca como ativa
                        break; // Spawnou uma pedra, sai do loop
                    }
                }
                tempo_ultimo_spawn = 0; // Reseta o spaw
            }

            // Atualiza  a posi��o das pedras
            for (int i = 0; i < MAX_STONES; i++) {
                if (pedras[i].active) {
                    pedras[i].y += velocidadePedra; // As pedras caem para baixo

                    // Verifica colis�o
                    if (verifica_colisao(&professor, &pedras[i])) {
                        pedras[i].active = false; // Desativa a pedra
                        pontuacao++;
                        al_play_sample(sompedra, 0.8, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

                        if (pontuacao == pontuacaoRequerida) {
                            incrementoVelocidadeProfessor ++;
                            velocidadePedra += 0.5;
                            pontuacaoRequerida += 10;
                            SPAWN_DELAY -= 0.05;
                            al_play_sample(sompowerup, 0.3, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

                        }
                    }

                    // Se a pedra cair no ch�o, ela volta
                    if (pedras[i].y > SCREEN_H) {
                        running = false;
                        pedras[i].active = false; // Mataa a pedra
                    }
                }
            }

            professor.x += velocidadeProfessor;
            // Limita a professor dentro da tela
            if (professor.x < 1) professor.x = 2;
            if (professor.x > SCREEN_W - PLATFORM_W - 1) professor.x = SCREEN_W - PLATFORM_W -6;

            redraw = true;
        }
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            // Fecha a janela
            running = false;
        }
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                velocidadeProfessor -= incrementoVelocidadeProfessor;
                teclaEsquerdaPressionada = true;
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                velocidadeProfessor += incrementoVelocidadeProfessor;
                teclaDireitaPressionada = true;
            }
        }

        else if (event.type == ALLEGRO_EVENT_KEY_UP) {

            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                velocidadeProfessor += incrementoVelocidadeProfessor;
                teclaEsquerdaPressionada = false;

            }  if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                    velocidadeProfessor -=incrementoVelocidadeProfessor;
                    teclaDireitaPressionada = false;
            }
                if (!teclaDireitaPressionada && !teclaEsquerdaPressionada){
                    velocidadeProfessor = 0;
            }
        }



        if (redraw && al_is_event_queue_empty(queue)) {

            redraw = false;

            al_draw_bitmap(background, 0, 0, 0);
            al_draw_bitmap(professor.sprite, professor.x, professor.y, 0);

            // Desenha as pedras
            for (int i = 0; i < MAX_STONES; i++) {
                if (pedras[i].active) {
                    al_draw_bitmap(pedras[i].sprite, pedras[i].x, pedras[i].y, 0);
                }
            }
            // desenha texto
            al_draw_textf(fonte, al_map_rgb(255, 255, 255), 10, 10, 0, "SCORE: %d", pontuacao);

            al_flip_display(); // ATUALIZA A TELA
        }
    }

    al_destroy_bitmap(professor.sprite);
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
    printf("SCORE: %d", pontuacao);
    return 0;
}

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
#define PROFESSOR_W 120
#define PROFESSOR_H 200
#define STONE_W 30
#define STONE_H 30
#define MAX_STONES 10
#define FPS 60
#define FONTSIZE 20

typedef struct {
    float x, y;             // Posição
    ALLEGRO_BITMAP *sprite; // Imagem do professor
} Professor;

typedef struct {
    float x, y;             // Posição
    ALLEGRO_BITMAP *sprite; // Imagem da pedra
    bool active;            // Se a pedra está ativa ou não
} Pedra;
// Funcao para verficar erros de iniciaização
void error_h(bool test, const char *description){
    if (test) return;

    printf("Nao foi capaz de inicializar o/a %s\n", description);
    exit(1);
}

bool verifica_colisao(Professor *professor, Pedra *pedra) {
    return (professor->x + 20 < pedra->x + STONE_W && professor->x + PROFESSOR_W > pedra->x &&
            professor->y < pedra->y + STONE_H && professor->y + PROFESSOR_H > pedra->y);
}

void display_death_screen(const ALLEGRO_FONT* font, const ALLEGRO_FONT* font_2, int score) {
    al_clear_to_color(al_map_rgb(0, 0, 0)); // Black background
    al_draw_text(font, al_map_rgb(255, 0, 0), SCREEN_W/ 2, SCREEN_H / 3, ALLEGRO_ALIGN_CENTRE, "GAME OVER");
    al_draw_textf(font_2, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 3 + 55, ALLEGRO_ALIGN_CENTRE, "FINAL SCORE: %d", score);
    al_draw_text(font_2, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 3 + 85, ALLEGRO_ALIGN_CENTRE, "Press \"ESC\" to exit.");
    al_flip_display();
}
// funcao para flippar o sprite do personagem
bool last_direction(bool l, bool r, bool last){
    bool invert = last;
    if (l && !r){
        invert = true;
        }
    else if (!l && r){
        invert = false;
        }
    return invert;
}

int main() {
    //delay entre os spawns das pedra
    float SPAWN_DELAY = 0.9;

    error_h(al_init(), "Allegro");
    error_h(al_install_keyboard(), "Keyboard");
    error_h(al_init_image_addon(), "Image Addon");

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
    ALLEGRO_FONT *death_font = al_load_font("fonts/FONT.ttf", 40, 0);

    error_h(musica, "Musica de Fundo");
    error_h(sompedra, "som de coletar pedra");
    error_h(sompowerup, "som de pontuação atingida");
    error_h(fonte, "Fonte padrão");
    error_h(death_font, "Fonte de Encerramento");

    // criação a fila de eventos
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());

    // inicialização do professor
    Professor professor;
    professor.x = SCREEN_W / 2 - 50; // Posição inicial horizontal
    professor.y = SCREEN_H - 150;     // Posição inicial vertical
    professor.sprite = al_load_bitmap("imagens/professor.png"); // Carrega o sprite
    error_h(professor.sprite, "Imagem da Professor");

    // Inicialização das pedras
    Pedra pedras[MAX_STONES];
    srand(time(NULL)); // Inicializa a semente (KKKK) para aleatoriedade
    for (int i = 0; i < MAX_STONES; i++) {
        pedras[i].x = -STONE_W;
        pedras[i].y = -STONE_H;
        pedras[i].active = false;
        pedras[i].sprite = al_load_bitmap("imagens/pedra.png");
        error_h(pedras[i].sprite, "Imagem da Pedra");
    }

    // Variáveis principais
    bool running = true, redraw = true;
    float velocidadeProfessor = 0;
    float incrementoVelocidadeProfessor = 6;
    float velocidadePedra = 4;
    int pontuacao = 0;
    int pontuacaoRequerida = 10;
    float tempo_ultimo_spawn = 0; // Armazena o tempo desde o último spawn
    bool left = false;
    bool right= false;
    bool last = false;


    // Inicia o timer
    al_start_timer(timer);

    al_play_sample(musica, 0.3, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);

    // Loop principal
    while (running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_TIMER) {
            tempo_ultimo_spawn += 1.0 / FPS;  // Incrementa o tempo ???

            if (tempo_ultimo_spawn >= SPAWN_DELAY) {
                for (int i = 0; i < MAX_STONES; i++) {
                    if (!pedras[i].active) {
                        pedras[i].x = rand() % (SCREEN_W - STONE_W);  // Posicão aleatoria
                        pedras[i].y = -STONE_H;  // Começa fora da tela, no topo
                        pedras[i].active = true;  // Marca como ativa
                        break; // Spawnou uma pedra, sai do loop
                    }
                }
                tempo_ultimo_spawn = 0; // Reseta o spaw
            }

            // Atualiza  a posição das pedras
            for (int i = 0; i < MAX_STONES; i++) {
                if (pedras[i].active) {
                    pedras[i].y += velocidadePedra; // As pedras caem para baixo

                    // Verifica colisão
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

                    // Se a pedra cair no chão, ela volta
                    if (pedras[i].y > SCREEN_H) {
                        running = false;
                        pedras[i].active = false; // Mataa a pedra
                    }
                }
            }
            redraw = true;
        }

        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }

        else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT || event.keyboard.keycode == ALLEGRO_KEY_A) {
                velocidadeProfessor -= incrementoVelocidadeProfessor;
                left = true;
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT || event.keyboard.keycode == ALLEGRO_KEY_D) {
                velocidadeProfessor += incrementoVelocidadeProfessor;
                right= true;
            }
                else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE){
                    running = false;
                }
        }

        else if (event.type == ALLEGRO_EVENT_KEY_UP) {

            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT || event.keyboard.keycode == ALLEGRO_KEY_A) {
                velocidadeProfessor += incrementoVelocidadeProfessor;
                left = false;

            }  if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT || event.keyboard.keycode == ALLEGRO_KEY_D) {
                    velocidadeProfessor -=incrementoVelocidadeProfessor;
                    right= false;
            }
                if (!right&& !left){
                    velocidadeProfessor = 0;
            }
        }

        professor.x += velocidadeProfessor;
            // Limita a professor dentro da tela
            if (professor.x < 1) professor.x = 2;
            if (professor.x > SCREEN_W - PROFESSOR_W - 1) professor.x = SCREEN_W - PROFESSOR_W -6;

        if (redraw && al_is_event_queue_empty(queue)) {

            redraw = false;

            al_draw_bitmap(background, 0, 0, 0);

            last = last_direction(left, right, last);
            if (last == false){
                al_draw_bitmap(professor.sprite, professor.x, professor.y, 0);
            }
            else {
                al_draw_bitmap(professor.sprite, professor.x + 35, professor.y, ALLEGRO_FLIP_HORIZONTAL);
            }

            // Desenha as pedras
            for (int i = 0; i < MAX_STONES; i++) {
                if (pedras[i].active) {
                    al_draw_bitmap(pedras[i].sprite, pedras[i].x, pedras[i].y, 0);
                }
            }

            al_draw_textf(fonte, al_map_rgb(255, 255, 255), 10, 10, 0, "SCORE: %d", pontuacao);

            al_flip_display(); // ATUALIZA A TELA
        }
    }
    al_destroy_sample(musica);
    while (true) {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);
        display_death_screen(death_font,fonte, pontuacao);
        if (event.type)
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE){
                break;
        }
    };

    al_destroy_bitmap(professor.sprite);
    for (int i = 0; i < MAX_STONES; i++) {
        al_destroy_bitmap(pedras[i].sprite);
    }
    al_destroy_bitmap(background);
    al_destroy_font(fonte);
    al_destroy_font(death_font);
    al_destroy_sample(musica);
    al_uninstall_audio();
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    return 0;
}

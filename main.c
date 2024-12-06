/* Copyright (C) 2016 Marcelo Serrano Zanetti - All Rights Reserved

 * Licensed under the GNU GPL V3.0 license. All conditions apply.

 * Powered by Allegro: http://liballeg.org/license.html

 */



#include <stdio.h>

#include <allegro5/allegro.h>

#include <allegro5/allegro_image.h>



const float FPS = 60;

const int SCREEN_W = 1024;

const int SCREEN_H = 768;

const int BOLA_TAMANHO = 77;

const int REBATEDOR_LARGURA = 257;

const int REBATEDOR_ALTURA = 506;

void error_h(bool test, const char *description)
{
    if(test) return;

    printf("Nao foi capaz de inicializar o/a %s\n", description);
    exit(1);
}

int main(int argc, char **argv)

{

    ALLEGRO_DISPLAY *display = NULL;

    ALLEGRO_EVENT_QUEUE *event_queue = NULL;

    ALLEGRO_TIMER *timer = NULL;

    error_h(al_init(), "allegro");

    timer = al_create_timer(1.0 / FPS);

    error_h(timer, "timer");

    bool redraw = true;

    al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_WINDOWED);

    display = al_create_display(SCREEN_W, SCREEN_H);

    error_h(display, "display");

    error_h(al_install_keyboard(), "keyboard");

    error_h(al_install_mouse(), "mouse");

    error_h(al_init_image_addon(), "image addon");


    ALLEGRO_BITMAP *bola = NULL;

    ALLEGRO_BITMAP *rebatedorEsquerda = NULL;

    ALLEGRO_BITMAP *rebatedorDireita = NULL;

    ALLEGRO_BITMAP *background = NULL;



    float bola_pos_x = SCREEN_W / 2.0 - BOLA_TAMANHO / 2.0;

    float bola_pos_y = SCREEN_H / 2.0 - BOLA_TAMANHO / 2.0;

    float bola_vx = -4;

    float bola_vy = -4;



    float rebatedorEsquerda_pos_x = 0;

    float rebatedorEsquerda_pos_y = SCREEN_H / 2.0 - REBATEDOR_ALTURA / 2.0;

    float rebatedorEsquerda_vy = 0;
    float rebatedorEsquerda_vx = 0;



    float rebatedorDireita_pos_x = SCREEN_W - REBATEDOR_LARGURA;

    float rebatedorDireita_pos_y = SCREEN_H / 2.0 - REBATEDOR_ALTURA / 2.0;

    float rebatedorDireita_vy = 0;



    int click=0;

    int savebvx;

    int savebvy;



    bola              =  al_load_bitmap("imagens/bola.png");

    rebatedorEsquerda = al_load_bitmap("imagens/maradona.png");

    rebatedorDireita  = al_load_bitmap("imagens/pele.png");

    background        = al_load_bitmap("imagens/campo.png");



    error_h(bola, "bola");
    error_h(rebatedorEsquerda, "rebatedorEsquerda");
    error_h(rebatedorDireita, "rebatedorDireita");
    error_h(background, "background");


    al_set_target_bitmap(al_get_backbuffer(display));



    event_queue = al_create_event_queue();

    error_h(event_queue, "event_queue");


    al_register_event_source(event_queue, al_get_display_event_source(display));

    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    al_register_event_source(event_queue, al_get_keyboard_event_source());

    al_register_event_source(event_queue, al_get_mouse_event_source());



    al_clear_to_color(al_map_rgb(0,0,0));



    al_flip_display();



    al_start_timer(timer);



    while(1)

    {

        ALLEGRO_EVENT ev;

        al_wait_for_event(event_queue, &ev);



        if(ev.type==ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)

        {

            switch(ev.mouse.button)

            {

            case 1:

                printf("x:%d y:%d\n",ev.mouse.x, ev.mouse.y);

                if(click==0)

                {

                    savebvx=bola_vx;

                    savebvy=bola_vy;

                    bola_vx=0;

                    bola_vy=0;

                    click=1;

                }

                else

                {

                    bola_vx=savebvx;

                    bola_vy=savebvy;

                    click=0;

                }

                break;

            }

        }

        else if(ev.type == ALLEGRO_EVENT_TIMER)

        {

            if(bola_pos_y < 0 || bola_pos_y > SCREEN_H - BOLA_TAMANHO)

            {

                bola_vy = -bola_vy;

            }



            if(bola_pos_x < 0 || bola_pos_x > SCREEN_W - BOLA_TAMANHO)

            {

                bola_pos_x = SCREEN_W / 2.0 - BOLA_TAMANHO / 2.0;

                bola_pos_y = SCREEN_H / 2.0 - BOLA_TAMANHO / 2.0;

                bola_vx = -4, bola_vy = -4;

            }



            if(bola_pos_x <= rebatedorEsquerda_pos_x + REBATEDOR_LARGURA)

                if(bola_pos_y+BOLA_TAMANHO>=rebatedorEsquerda_pos_y)

                    if(bola_pos_y-BOLA_TAMANHO<=rebatedorEsquerda_pos_y+REBATEDOR_ALTURA)

                        bola_vx = -bola_vx;



            if(bola_pos_x + BOLA_TAMANHO >= rebatedorDireita_pos_x )

                if(bola_pos_y+BOLA_TAMANHO>=rebatedorDireita_pos_y)

                    if(bola_pos_y-BOLA_TAMANHO<=rebatedorDireita_pos_y+REBATEDOR_ALTURA)

                        bola_vx = -bola_vx;



            bola_pos_x              += bola_vx;

            bola_pos_y              += bola_vy;

            rebatedorEsquerda_pos_y += rebatedorEsquerda_vy;

            rebatedorEsquerda_pos_x += rebatedorEsquerda_vx;

            rebatedorDireita_pos_y  += rebatedorDireita_vy;

            redraw = true;

        }

        else if(ev.type == ALLEGRO_EVENT_KEY_DOWN)

        {

            switch(ev.keyboard.keycode)

            {

            case ALLEGRO_KEY_D:

                rebatedorEsquerda_vx=4;

                break;



            case ALLEGRO_KEY_A:

                rebatedorEsquerda_vx=-4;

                break;



            case ALLEGRO_KEY_W:

                rebatedorEsquerda_vy=-4;

                break;

            case ALLEGRO_KEY_S:

                rebatedorEsquerda_vy=4;

                break;





            case ALLEGRO_KEY_ESCAPE:

                return 0;

                break;

            }

        }

        else if(ev.type == ALLEGRO_EVENT_KEY_UP)

        {

            switch(ev.keyboard.keycode)

            {

            case ALLEGRO_KEY_D:

            case ALLEGRO_KEY_A:

                rebatedorEsquerda_vx=0;

                break;



            case ALLEGRO_KEY_W:

            case ALLEGRO_KEY_S:

                rebatedorEsquerda_vy=0;

                break;

            }

        }

        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)

        {

            break;

        }



        if(redraw && al_is_event_queue_empty(event_queue))

        {

            redraw = false;



            al_clear_to_color(al_map_rgb(0,0,110));



            al_draw_bitmap(background, 0, 0, 0);

            al_draw_bitmap(rebatedorEsquerda,rebatedorEsquerda_pos_x, rebatedorEsquerda_pos_y, 0);

            al_draw_bitmap(rebatedorDireita,rebatedorDireita_pos_x, rebatedorDireita_pos_y, 0);

            al_draw_bitmap(bola, bola_pos_x, bola_pos_y, 0);



            al_flip_display();

        }

    }



    al_destroy_bitmap(bola);

    al_destroy_bitmap(rebatedorEsquerda);

    al_destroy_bitmap(rebatedorDireita);

    al_destroy_timer(timer);

    al_destroy_display(display);

    al_destroy_event_queue(event_queue);



    return 0;

}


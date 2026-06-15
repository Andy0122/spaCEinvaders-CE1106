/**
 * @file gui.c
 * @brief Implementación de las funciones de renderizado del cliente utilizando Sprites redimensionados.
 */

#include "../include/gui.h"
#include "../include/constantes.h"
#include "raylib.h"
#include "stdlib.h"

// Variables globales estáticas para almacenar las texturas en la VRAM
static Texture2D tex_jugador;
static Texture2D tex_pulpo;
static Texture2D tex_cangrejo;
static Texture2D tex_calamar;
static Texture2D tex_bunker;
static Texture2D tex_ovni;

void inicializar_gui() {
    InitWindow(ANCHO_PANTALLA, ALTO_PANTALLA, "spaCEinvaders - Cliente C");
    SetTargetFPS(FPS_OBJETIVO);

    // Carga las imágenes temporalmente en la memoria RAM
    Image img_jugador = LoadImage("assets/jugador.png");
    Image img_pulpo = LoadImage("assets/pulpo.png");
    Image img_cangrejo = LoadImage("assets/cangrejo.png");
    Image img_calamar = LoadImage("assets/calamar.png");
    Image img_bunker = LoadImage("assets/bunker.png");
    Image img_ovni = LoadImage("assets/ovni.png");

    // Redimensiona las imágenes usando las dimensiones exactas de constantes.h
    ImageResize(&img_jugador, ANCHO_CANON, ALTO_CANON);
    ImageResize(&img_pulpo, ANCHO_ALIEN, ALTO_ALIEN);
    ImageResize(&img_cangrejo, ANCHO_ALIEN, ALTO_ALIEN);
    ImageResize(&img_calamar, ANCHO_ALIEN, ALTO_ALIEN);
    ImageResize(&img_bunker, 60, 40); 
    ImageResize(&img_ovni, ANCHO_ALIEN * 2, ALTO_ALIEN * 1);

    // Convierte las imágenes modificadas a texturas para la VRAM
    tex_jugador = LoadTextureFromImage(img_jugador);
    tex_pulpo = LoadTextureFromImage(img_pulpo);
    tex_cangrejo = LoadTextureFromImage(img_cangrejo);
    tex_calamar = LoadTextureFromImage(img_calamar);
    tex_bunker = LoadTextureFromImage(img_bunker);
    tex_ovni = LoadTextureFromImage(img_ovni);

    // Liberar la memoria RAM temporal para evitar fugas
    UnloadImage(img_jugador);
    UnloadImage(img_pulpo);
    UnloadImage(img_cangrejo);
    UnloadImage(img_calamar);
    UnloadImage(img_bunker);
    UnloadImage(img_ovni);
}

void dibujar_jugador(Jugador *j) {
    // WHITE indica que se respeta el color original del PNG
    DrawTexture(tex_jugador, j->posicion_x, ALTO_PANTALLA - 60, WHITE);
}

void dibujar_hud(int puntuacion, int vidas) {
    DrawText(TextFormat("PUNTOS: %04d", puntuacion), 20, 20, 20, WHITE);
    DrawText(TextFormat("VIDAS: %d", vidas), ANCHO_PANTALLA - 120, 20, 20, GREEN);
}

void dibujar_bunkers(Bunker bunkers[], int cantidad) {
    for (int i = 0; i < cantidad; i++) {
        if (bunkers[i].porcentaje_salud > 0) {
            // Filtro de color para mostrar el deterioro del escudo
            Color tinte_bunker = GREEN;
            if (bunkers[i].porcentaje_salud <= 70) tinte_bunker = YELLOW;
            if (bunkers[i].porcentaje_salud <= 40) tinte_bunker = RED;

            DrawTexture(tex_bunker, bunkers[i].x, bunkers[i].y, tinte_bunker);
        }
    }
}

void dibujar_matriz_aliens(Extraterrestre aliens[], int total_aliens) {
    for (int i = 0; i < total_aliens; i++) {
        if (aliens[i].estado == 1) { 
            if (aliens[i].tipo == 40) DrawTexture(tex_pulpo, aliens[i].x, aliens[i].y, WHITE);
            else if (aliens[i].tipo == 20) DrawTexture(tex_cangrejo, aliens[i].x, aliens[i].y, WHITE);
            else if (aliens[i].tipo == 10) DrawTexture(tex_calamar, aliens[i].x, aliens[i].y, WHITE);
        }
    }
}

/**
 * @brief Renderizado del OVNI.
 */
void dibujar_ovni(Ovni *o) {
    if (o != NULL && o->activo == 1) {
        DrawTexture(tex_ovni, o->x, o->y, WHITE);
    }
}

void cerrar_gui() {
    // Liberar la memoria de la tarjeta de video antes de cerrar el programa
    UnloadTexture(tex_jugador);
    UnloadTexture(tex_pulpo);
    UnloadTexture(tex_cangrejo);
    UnloadTexture(tex_calamar);
    UnloadTexture(tex_bunker);
    UnloadTexture(tex_ovni); 
    CloseWindow();
}
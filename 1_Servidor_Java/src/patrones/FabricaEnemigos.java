package patrones;

import modelo.Enemigo;
import modelo.Extraterrestre;

/**
 * Patrón Factory Method: centraliza la creación de todos los tipos de enemigo.
 * Desacopla al cliente (Juego.java) de las clases concretas.
 */
public class FabricaEnemigos {
    public static final int TIPO_BAJO  = 1;  // Calamar  → 10 pts
    public static final int TIPO_MEDIO = 2;  // Cangrejo → 20 pts
    public static final int TIPO_ALTO  = 3;  // Pulpo    → 40 pts

    /** Crea un extraterrestre por tipo (1/2/3). */
    public static Enemigo crearExtraterrestre(int tipo, int x, int y) {
        return new Extraterrestre(tipo, x, y);
    }

    /**
     * Crea un extraterrestre deduciendo el tipo a partir de los puntos:
     *   pts <= 10  → tipo 1 (Calamar)
     *   pts <= 20  → tipo 2 (Cangrejo)
     *   pts >  20  → tipo 3 (Pulpo)
     * Nota: los puntos del objeto quedan fijados por el tipo deducido,
     * no por el valor arbitrario del admin.  Si se necesitara puntaje libre,
     * habría que añadir un cuarto constructor en Extraterrestre.
     */
    public static Enemigo crearExtraterrestrePorPuntos(int x, int y, int puntos) {
        int tipo;
        if      (puntos <= 10) tipo = TIPO_BAJO;
        else if (puntos <= 20) tipo = TIPO_MEDIO;
        else                   tipo = TIPO_ALTO;
        return new Extraterrestre(tipo, x, y);
    }

    public static Enemigo crearExtraterrestreBajo(int x, int y) {
        return crearExtraterrestre(TIPO_BAJO, x, y);
    }

    public static Enemigo crearExtraterrestreMedio(int x, int y) {
        return crearExtraterrestre(TIPO_MEDIO, x, y);
    }

    public static Enemigo crearExtraterrestreAlto(int x, int y) {
        return crearExtraterrestre(TIPO_ALTO, x, y);
    }

    public static Enemigo crearExtraterrestreDesdeCategoria(String categoria, int x, int y) {
        switch (categoria.toLowerCase()) {
            case "bajo":  case "facil":   return crearExtraterrestreBajo(x, y);
            case "medio": case "normal":  return crearExtraterrestreMedio(x, y);
            case "alto":  case "dificil": return crearExtraterrestreAlto(x, y);
            default:                      return crearExtraterrestreBajo(x, y);
        }
    }
}
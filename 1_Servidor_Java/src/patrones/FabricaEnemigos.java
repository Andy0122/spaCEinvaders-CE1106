package patrones;

import modelo.Enemigo;
import modelo.Extraterrestre;

public class FabricaEnemigos {
    public static final int TIPO_BAJO = 1;
    public static final int TIPO_MEDIO = 2;
    public static final int TIPO_ALTO = 3;

    public static Enemigo crearExtraterrestre(int tipo, int x, int y) {
        return new Extraterrestre(tipo, x, y);
    }

    public static Enemigo crearExtraterrestrePorPuntos(int x, int y, int puntos) {
        return new Extraterrestre(x, y, puntos);
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
            case "bajo":
            case "facil":
                return crearExtraterrestreBajo(x, y);
            case "medio":
            case "normal":
                return crearExtraterrestreMedio(x, y);
            case "alto":
            case "dificil":
                return crearExtraterrestreAlto(x, y);
            default:
                return crearExtraterrestreBajo(x, y);
        }
    }
}

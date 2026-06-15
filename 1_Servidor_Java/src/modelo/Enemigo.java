package modelo;

public interface Enemigo {
    int getId();
    int getTipo();
    int getVida();
    int getPuntos();
    int getVelocidad();
    void setVelocidad(int velocidad);
    int getPosicionX();
    int getPosicionY();
    void setPosicion(int x, int y);
    void mover(int deltaX, int deltaY);
    boolean estaDestruido();
}

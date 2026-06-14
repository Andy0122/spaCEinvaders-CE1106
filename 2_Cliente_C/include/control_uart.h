#ifndef CONTROL_UART_H
#define CONTROL_UART_H

int uart_inicializar(const char *puerto);
int uart_leer_comando(char *comando);   // retorna 1 si leyó un comando válido
void uart_cerrar(void);

#endif
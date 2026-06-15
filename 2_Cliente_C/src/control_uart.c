#include <windows.h>
#include <stdio.h>
#include "control_uart.h"

static HANDLE hSerial = INVALID_HANDLE_VALUE;

int uart_inicializar(const char *puerto)
{
    char nombre_puerto[32];
    snprintf(nombre_puerto, sizeof(nombre_puerto), "\\\\.\\%s", puerto);

    hSerial = CreateFileA(
        nombre_puerto,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("[UART] No se pudo abrir %s. Error Windows: %lu\n", puerto, GetLastError());
        return 0;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    // Obtenemos la configuración actual para no enojar a Windows
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return 0;
    }

    // Solo tocamos lo indispensable (9600 baudios)
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("[UART] Error en SetCommState. Error: %lu\n", GetLastError());
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return 0;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;

    SetCommTimeouts(hSerial, &timeouts);
    PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

    printf("[UART] Puerto %s abierto correctamente.\n", puerto);
    return 1;
}

int uart_leer_comando(char *comando)
{
    if (hSerial == INVALID_HANDLE_VALUE) return 0;

    DWORD errores;
    COMSTAT estado;
    ClearCommError(hSerial, &errores, &estado);

    // Bucle inteligente: Lee todo lo que haya hasta encontrar L, R o S
    while (estado.cbInQue > 0) {
        char c = 0;
        DWORD bytesLeidos = 0;

        if (ReadFile(hSerial, &c, 1, &bytesLeidos, NULL) && bytesLeidos == 1) {
            // Si es un comando válido, lo mandamos al juego y salimos
            if (c == 'L' || c == 'R' || c == 'S') {
                *comando = c;
                return 1; 
            }
        }
        // Actualizamos para ver si queda más basura o comandos en la cola
        ClearCommError(hSerial, &errores, &estado); 
    }

    // Si llegamos aquí, el buffer estaba vacío o solo había basura
    return 0; 
}

void uart_cerrar(void)
{
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
    }
}
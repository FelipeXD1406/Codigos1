/*
 * File:   main.c
 * Author: Alejandro
 *
 * Created on August 31, 2023, 7:54 PM
 */

#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#pragma config FOSC = INTIO67
#pragma config WDTEN = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 16000000
#define time 10

// Definir constantes para el LCD (según el código proporcionado)
#define CD 0x01
#define RH 0x02
#define EMS 0x06
#define DC 0x0F
#define DSr 0x1C
#define DSl 0x18
#define FS 0x28
#define RAW1 0x80
#define RAW2 0xC0
#define E LATE0
#define RS LATE1

// Funciones para el LCD (según el código proporcionado)
void SettingsLCD(unsigned char word);
void WriteLCD(unsigned char word);
void LCD(unsigned char word);

// Funciones UART
void UART1_Write_Char(char data);
void UART1_Write_String(const char *text);
char UART1_Read_Char();

// Variables globales
float initialX, initialY; // Coordenadas iniciales (decimales)
float finalX, finalY; // Coordenadas finales (decimales)
unsigned char mode; // Variable para almacenar el modo (S/s: Send, K/k: OK)

void settings(void) {
    OSCCON = 0x72;
    ANSELC = 0x00;
    ANSELD = 0x00;
    ANSELE = 0x00;
    TRISCbits.RC5 = 0;
    LATCbits.LC5 = 0;
    TRISD = 0x00;
    TRISE = 0x00;
    LATD = 0x00;
    LATE = 0x00;

    // Configuración del LCD
    SettingsLCD(0x02);
    SettingsLCD(EMS);
    SettingsLCD(DC);
    SettingsLCD(FS);
    SettingsLCD(CD);

    // Configuración UART1
    TRISCbits.RC7 = 1;
    TRISCbits.RC6 = 0;
    SPBRG1 = 0x19;
    RCSTA1 = 0x90;
    TXSTA1 = 0x20;

    // Habilitar interrupciones
    GIE = 1;
    PEIE = 1;
    RCIE = 1;
    RCIF = 0;
}

void main(void) {
    settings();

    while (1) {
        // Esperar hasta que se reciba el modo 'S'
        while (mode != 'S' && mode != 's') {
            // Supongamos que se recibirá el modo por Bluetooth
            mode = UART1_Read_Char();
        }

        // Solicitar las coordenadas iniciales al teléfono móvil
        UART1_Write_String("Enviar coordenadas iniciales: ");

        // Supongamos que las coordenadas iniciales están almacenadas en initialX e initialY
        // y se deben convertir a cadena de caracteres antes de enviar
        char buffer[20];
        sprintf(buffer, "%.2f,%.2f", initialX, initialY);
        UART1_Write_String(buffer);
        UART1_Write_Char('K');

        // Recibir las coordenadas iniciales del teléfono móvil y almacenarlas en initialX e initialY
        char bufferInicial[20]; // Buffer para almacenar las coordenadas iniciales recibidas
        int index = 0; // Índice para el bufferInicial
        char receivedChar;
        do {
            receivedChar = UART1_Read_Char();
            if (receivedChar != '\0' && receivedChar != 'K') {
                bufferInicial[index++] = receivedChar;
            }
        } while (receivedChar != 'K');
        bufferInicial[index] = '\0'; // Terminador de cadena

        // Implementar la lógica para extraer los valores de la cadena bufferInicial y almacenarlos en initialX e initialY
        // Puedes utilizar una técnica de "parsing" manual para separar los valores de las coordenadas y convertirlos a float.

        // Ejemplo de implementación utilizando strtod:
        float coordX, coordY;
        char *token;
        token = strtok(bufferInicial, ",");
        if (token) {
            coordX = strtod(token, NULL);
            token = strtok(NULL, ",");
            if (token) {
                coordY = strtod(token, NULL);
            } else {
                // Error: No se encontró la segunda coordenada
            }
        } else {
            // Error: No se encontró la primera coordenada
        }
        initialX = coordX;
        initialY = coordY;

        // Imprimir las coordenadas iniciales en el LCD
        char lcdBuffer[20];
        sprintf(lcdBuffer, "Coord. iniciales: %.2f,%.2f", initialX, initialY);
        SettingsLCD(RAW1);
        UART1_Write_String(lcdBuffer);

        // Solicitar las coordenadas finales al teléfono móvil
        UART1_Write_String("Enviar coordenadas finales: ");

        // Supongamos que las coordenadas finales están almacenadas en finalX y finalY
        // y se deben convertir a cadena de caracteres antes de enviar
        sprintf(buffer, "%.2f,%.2f", finalX, finalY);
        UART1_Write_String(buffer);
        UART1_Write_Char('K');

        // Recibir las coordenadas finales del teléfono móvil y almacenarlas en finalX y finalY
        char bufferFinal[20]; // Buffer para almacenar las coordenadas finales recibidas
        index = 0; // Índice para el bufferFinal
        do {
            receivedChar = UART1_Read_Char();
            if (receivedChar != '\0' && receivedChar != 'K') {
                bufferFinal[index++] = receivedChar;
            }
        } while (receivedChar != 'K');
        bufferFinal[index] = '\0'; // Terminador de cadena

        // Implementar la lógica para extraer los valores de la cadena bufferFinal y almacenarlos en finalX y finalY
        // Utiliza una técnica de "parsing" manual similar a la que se utilizó para las coordenadas iniciales.

        // Ejemplo de implementación utilizando strtod:
        token = strtok(bufferFinal, ",");
        if (token) {
            coordX = strtod(token, NULL);
            token = strtok(NULL, ",");
            if (token) {
                coordY = strtod(token, NULL);
            } else {
                // Error: No se encontró la segunda coordenada
            }
        } else {
            // Error: No se encontró la primera coordenada
        }
        finalX = coordX;
        finalY = coordY;

        // Confirmar el envío de las coordenadas finales con el caracter 'K'
        UART1_Write_Char('K');

        // Imprimir las coordenadas finales en el LCD
        sprintf(lcdBuffer, "Coord. finales: %.2f,%.2f", finalX, finalY);
        SettingsLCD(RAW2);
        UART1_Write_String(lcdBuffer);

        // Aquí puedes enviar las coordenadas finales al teléfono móvil si es necesario
    }
}

void SettingsLCD(unsigned char word) {
    RS = 0;
    LCD(word >> 4);
    LCD(word & 0x0F);
}

void WriteLCD(unsigned char word) {
    RS = 1;
    LCD(word >> 4);
    LCD(word & 0x0F);
}

void LCD(unsigned char data) {
    E = 1;
    __delay_us(time);
    LATD = data;
    __delay_us(time);
    E = 0;
    __delay_us(time);
}

void UART1_Write_Char(char data) {
    while (!TX1IF);
    TXREG1 = data;
}

void UART1_Write_String(const char *text) {
    while (*text != '\0') {
        UART1_Write_Char(*text);
        text++;
    }
}

char UART1_Read_Char() {
    if (RC1IF) {
        return RCREG1;
    } else {
        return '\0';
    }
}

void __interrupt() RXTX(void) {
    if (RC1IF) {
        char receivedChar = RCREG1;
        if (receivedChar == 'M' || receivedChar == 'm') {
            SettingsLCD(RAW1);
            WriteLCD(receivedChar);
        } else if (receivedChar == 'A' || receivedChar == 'a') {
            UART1_Write_String("Enviar coordenadas:");
            UART1_Write_Char(0x0D); // retorno de carro
        } else if (receivedChar == 'S' || receivedChar == 's') {
            mode = 'S'; // Modo Send
        }
    }
    //    } else if (receivedChar == 0x0D) {
    //        SettingsLCD(RH);
    //    } else {
    //        WriteLCD(receivedChar);
    //    }
}
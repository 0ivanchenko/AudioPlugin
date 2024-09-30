#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

typedef struct {
	float* data; // указатель на массив данных аудиосигнала
	size_t size; // размер буфера (количество сэмплов)
	int sampleRate; // частота дискретизации
} AudioBuffer;

typedef struct {
	float mix; // коэффициент смешивания сухого и обработанного сигнала
	void (*process)(AudioBuffer* input, AudioBuffer* output); // функция для обработки входного аудиобуфера
} AudioEffect;

int main() {

	setlocale(LC_ALL, "ru");

}
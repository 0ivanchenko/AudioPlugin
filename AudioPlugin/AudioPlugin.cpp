#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

class AudioBuffer {
private:
    float* data; // указатель на массив данных аудиосигнала
    size_t size; // размер буфера (кол-во сэмплов)
    int sampleRate; // частота дискретизации

public:
    AudioBuffer() : data(nullptr), size(0), sampleRate(0) {}

    void init(size_t size, int sampleRate) {
        this->data = (float*)malloc(size * sizeof(float));
        this->size = size;
        this->sampleRate = sampleRate;
    }
    void freeBuffer() {
        free(this->data);
    }
    float* getData() const { return data; }
    size_t getSize() const { return size; }
    int getSampleRate() const { return sampleRate; }
};

class AudioEffect {
protected:
    float mix; // коэффициент смешивания сухого и обработанного сигнала
    virtual void processEffect(AudioBuffer* input, AudioBuffer* output) = 0; // чисто виртуальная функция

public:
    AudioEffect() : mix(0.0f) {}
    virtual ~AudioEffect() {}

    void setMix(float mix) { this->mix = mix; }
    float getMix() const { return mix; }

    void applyEffect(AudioBuffer* input, AudioBuffer* output) {
        // Основная структура алгоритма
        processEffect(input, output);
    }
};

class Reverb : public AudioEffect {
private:
    float roomSize; // размер комнаты для реверберации
    float dampening; // уровень демпфирования

public:
    Reverb() : roomSize(0.0f), dampening(0.0f) {}

    void init(float roomSize, float dampening, float mix) {
        this->roomSize = roomSize;
        this->dampening = dampening;
        this->mix = mix;
    }

    void processEffect(AudioBuffer* input, AudioBuffer* output) override {
        const size_t bufferSize = input->getSize();
        float* delayBuffer1 = (float*)malloc(bufferSize * sizeof(float));
        float* delayBuffer2 = (float*)malloc(bufferSize * sizeof(float));

        for (size_t i = 0; i < bufferSize; i++) {
            delayBuffer1[i] = 0.0f;
            delayBuffer2[i] = 0.0f;
        }

        for (size_t i = 0; i < bufferSize; i++) {
            size_t delayIndex1 = (i + (size_t)(roomSize * input->getSampleRate())) % bufferSize;
            size_t delayIndex2 = (i + (size_t)((roomSize + 0.1f) * input->getSampleRate())) % bufferSize;

            float delayedSample1 = delayBuffer1[delayIndex1];
            float delayedSample2 = delayBuffer2[delayIndex2];

            float outputSample = input->getData()[i] + dampening * (delayedSample1 + delayedSample2);

            delayBuffer1[i] = input->getData()[i] + dampening * delayedSample1;
            delayBuffer2[i] = input->getData()[i] + dampening * delayedSample2;

            output->getData()[i] = outputSample;
        }

        free(delayBuffer1);
        free(delayBuffer2);
    }
};

class Delay : public AudioEffect {
private:
    float delayTime; // время задержки в миллисекундах
    float feedback; // уровень обратной связи

public:
    Delay() : delayTime(0.0f), feedback(0.0f) {}

    void init(float delayTime, float feedback, float mix) {
        this->delayTime = delayTime;
        this->feedback = feedback;
        this->mix = mix;
    }

    void processEffect(AudioBuffer* input, AudioBuffer* output) override {
        const size_t bufferSize = input->getSize();
        float* delayBuffer = (float*)malloc(bufferSize * sizeof(float));

        for (size_t i = 0; i < bufferSize; i++) {
            delayBuffer[i] = 0.0f;
        }

        for (size_t i = 0; i < bufferSize; i++) {
            size_t delayIndex = (i + (size_t)(delayTime * input->getSampleRate() / 1000.0f)) % bufferSize;

            float delayedSample = delayBuffer[delayIndex];
            float outputSample = input->getData()[i] + feedback * delayedSample;

            delayBuffer[i] = input->getData()[i] + feedback * delayedSample;

            output->getData()[i] = outputSample;
        }

        free(delayBuffer);
    }
};

class AudioFile {
private:
    char* filePath;

public:
    AudioFile(const char* filePath) {
        this->filePath = _strdup(filePath);
    }

    ~AudioFile() {
        free(filePath);
    }

    const char* getFilePath() const { return filePath; }
};

class AudioInput {
private:
    AudioBuffer buffer; // аудио буффер с входными данными
    AudioFile* source; // источник аудиосигнала (например, файл)

public:
    AudioInput() : source(nullptr) {}

    void init(size_t size, int sampleRate, AudioFile* source) {
        buffer.init(size, sampleRate);
        this->source = source;
    }

    void freeInput() {
        buffer.freeBuffer();
    }

    AudioBuffer* getBuffer() { return &buffer; }
    AudioFile* getSource() const { return source; }
};

class AudioOutput {
private:
    AudioBuffer buffer; // аудио буффер с входными данными
    AudioFile* destination; // место сохранения или проигрывания аудиосигнала

public:
    AudioOutput() : destination(nullptr) {}

    void init(size_t size, int sampleRate, AudioFile* destination) {
        buffer.init(size, sampleRate);
        this->destination = destination;
    }

    void freeOutput() {
        buffer.freeBuffer();
    }

    AudioBuffer* getBuffer() { return &buffer; }
    AudioFile* getDestination() const { return destination; }
};

class PluginSettings {
private:
    float gain; // усиление сигнала
    int bypass; // флаг включения/выключения эффекта

public:
    PluginSettings() : gain(1.0f), bypass(0) {}

    void setGain(float gain) { this->gain = gain; }
    void setBypass(int bypass) { this->bypass = bypass; }

    float getGain() const { return gain; }
    int getBypass() const { return bypass; }
};

class AudioPlugin {
private:
    AudioInput input; // аудиовход
    AudioOutput output; // аудиовыход
    AudioEffect** effects; // массив указателей на эффекты
    size_t numEffects; // количество эффектов
    PluginSettings settings; // настройки плагина

public:
    AudioPlugin() : effects(nullptr), numEffects(0) {}

    void init(size_t inputSize, int inputSampleRate, AudioFile* inputSource,
        size_t outputSize, int outputSampleRate, AudioFile* outputDestination,
        AudioEffect** effects, size_t numEffects) {
        input.init(inputSize, inputSampleRate, inputSource);
        output.init(outputSize, outputSampleRate, outputDestination);
        this->effects = effects;
        this->numEffects = numEffects;
    }

    void freePlugin() {
        input.freeInput();
        output.freeOutput();
        for (size_t i = 0; i < numEffects; i++) {
            delete effects[i];
        }
        delete[] effects;
    }

    void applyEffects() {
        for (size_t i = 0; i < numEffects; i++) {
            effects[i]->applyEffect(input.getBuffer(), output.getBuffer());
        }
    }

    AudioInput* getInput() { return &input; }
    AudioOutput* getOutput() { return &output; }
    PluginSettings* getSettings() { return &settings; }
};

int main() {
    setlocale(LC_ALL, "ru");

    // Создание динамического массива объектов класса
    AudioEffect** effects = new AudioEffect * [2];
    effects[0] = new Reverb();
    effects[1] = new Delay();

    // Инициализация эффектов
    ((Reverb*)effects[0])->init(0.8f, 0.5f, 0.7f);
    ((Delay*)effects[1])->init(500.0f, 0.4f, 0.6f);

    // Создание объектов AudioFile
    AudioFile* inputFile = new AudioFile("input.wav");
    AudioFile* outputFile = new AudioFile("output.wav");

    // Создание аудиоплагина
    AudioPlugin* plugin = new AudioPlugin();
    plugin->init(1024, 44100, inputFile, 1024, 44100, outputFile, effects, 2);

    // Применение эффектов
    plugin->applyEffects();

    // Освобождение памяти
    plugin->freePlugin();
    delete plugin;
    delete inputFile;
    delete outputFile;

    std::cout << "Аудио плагин обработал звук." << std::endl;

    return 0;
}

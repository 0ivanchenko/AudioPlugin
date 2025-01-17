#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdexcept>
#include <string>

// Класс для буфера аудиоданных
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

    // Возврат значения через ссылку
    float& operator[](size_t index) {
        return data[index];
    }

    // Возврат значения через указатель
    float* getDataPointer() {
        return data;
    }
};

// Базовый класс для аудиоэффекта
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

    // Перегрузка оператора вывода для производных классов
    virtual std::ostream& print(std::ostream& os) const {
        os << "AudioEffect с коэффициентом смешивания: " << getMix();
        return os;
    }

    // Перегрузка оператора вывода
    friend std::ostream& operator<<(std::ostream& os, const AudioEffect& effect) {
        return effect.print(os);
    }
};

// Класс реверберации
class Reverb : public AudioEffect {
private:
    float roomSize; // размер комнаты для реверберации
    float dampening; // уровень демпфирования

public:
    Reverb() : roomSize(0.0f), dampening(0.0f) {}

    Reverb(float roomSize, float dampening, float mix) : AudioEffect() {
        this->roomSize = roomSize;
        this->dampening = dampening;
        this->mix = mix;
    }

    Reverb& operator=(const AudioEffect& other) {
        if (this != &other) {
            this->mix = other.getMix();
        }
        return *this;
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

    std::ostream& print(std::ostream& os) const override {
        os << "Reverb с размером комнаты: " << roomSize << ", демпфированием: " << dampening << ", коэффициентом смешивания: " << getMix();
        return os;
    }
};

// Класс задержки (Delay)
class Delay : public AudioEffect {
private:
    float delayTime; // время задержки в миллисекундах
    float feedback;  // коэффициент обратной связи

public:
    Delay() : delayTime(0.0f), feedback(0.0f) {}

    Delay(float delayTime, float feedback, float mix) : AudioEffect() {
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
            size_t delayIndex = (i + (size_t)(delayTime * input->getSampleRate() / 1000)) % bufferSize;
            float delayedSample = delayBuffer[delayIndex];

            float outputSample = input->getData()[i] + feedback * delayedSample;

            delayBuffer[i] = input->getData()[i] + feedback * delayedSample;

            output->getData()[i] = outputSample;
        }

        free(delayBuffer);
    }

    std::ostream& print(std::ostream& os) const override {
        os << "Delay с временем задержки: " << delayTime << " мс, коэффициентом обратной связи: " << feedback << ", коэффициентом смешивания: " << getMix();
        return os;
    }
};

// Класс файла аудио
class AudioFile {
private:
    std::string filePath;

public:
    AudioFile(const std::string& filePath) : filePath(filePath) {}

    AudioFile(const AudioFile& other) : filePath(other.filePath) {}

    AudioFile& operator=(const AudioFile& other) {
        if (this != &other) {
            filePath = other.filePath;
        }
        return *this;
    }

    const std::string& getFilePath() const { return filePath; }

    friend std::ostream& operator<<(std::ostream& os, const AudioFile& file) {
        os << "Путь к аудиофайлу: " << file.getFilePath();
        return os;
    }
};

// Класс настроек плагина
class PluginSettings {
private:
    float gain;
    int bypass;

public:
    PluginSettings() : gain(1.0f), bypass(0) {}

    void setGain(float gain) { this->gain = gain; }
    void setBypass(int bypass) { this->bypass = bypass; }

    float getGain() const { return gain; }
    int getBypass() const { return bypass; }
};

// Класс плагина
class AudioPlugin {
private:
    AudioBuffer input;
    AudioBuffer output;
    AudioEffect** effects;
    size_t numEffects;
    PluginSettings settings;

public:
    AudioPlugin() : effects(nullptr), numEffects(0) {}

    void init(size_t inputSize, int inputSampleRate, AudioFile* inputSource,
        size_t outputSize, int outputSampleRate, AudioFile* outputDestination,
        AudioEffect** effects, size_t numEffects) {
        input.init(inputSize, inputSampleRate);
        output.init(outputSize, outputSampleRate);
        this->effects = effects;
        this->numEffects = numEffects;
    }

    void freePlugin() {
        input.freeBuffer();
        output.freeBuffer();
        for (size_t i = 0; i < numEffects; i++) {
            delete effects[i];
        }
        delete[] effects;
    }

    void applyEffects() {
        try {
            for (size_t i = 0; i < numEffects; i++) {
                effects[i]->applyEffect(&input, &output);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка при применении эффекта: " << e.what() << std::endl;
            throw; // Повторное выбрасывание исключения
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const AudioPlugin& plugin) {
        os << "Аудиоплагин с эффектами:\n";
        for (size_t i = 0; i < plugin.numEffects; i++) {
            os << *plugin.effects[i] << std::endl;
        }
        return os;
    }
};

int main() {
    setlocale(LC_ALL, "ru");

    try {
        // Создание объекта плагина
        AudioPlugin plugin;

        AudioFile* inputFile = new AudioFile("input.wav");
        AudioFile* outputFile = new AudioFile("output.wav");

        // Инициализация эффектов
        AudioEffect** effects = new AudioEffect * [2];
        effects[0] = new Reverb(0.8f, 0.5f, 0.7f);
        effects[1] = new Delay(500.0f, 0.4f, 0.6f);

        plugin.init(1024, 44100, inputFile, 1024, 44100, outputFile, effects, 2);

        // Применение эффектов
        plugin.applyEffects();

        // Вывод информации о плагине
        std::cout << plugin << std::endl;

        // Сообщение об успешной обработке
        std::cout << "Звук успешно обработан!" << std::endl;

        // Освобождение памяти
        plugin.freePlugin();
        delete inputFile;
        delete outputFile;

    }
    catch (const std::exception& e) {
        std::cerr << "Произошла ошибка: " << e.what() << std::endl;
    }

    return 0;
}
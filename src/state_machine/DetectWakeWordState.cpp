#include <Arduino.h>
#include "I2SSampler.h"
#include "AudioProcessor.h"
#include "NeuralNetwork.h"
#include "RingBuffer.h"
#include "DetectWakeWordState.h"

#define WINDOW_SIZE 320
#define STEP_SIZE 160
#define POOLING_SIZE 6
#define AUDIO_LENGTH 16000

#define LED_PIN 2

DetectWakeWordState::DetectWakeWordState(I2SSampler *sample_provider)
{
    // save the sample provider for use later
    m_sample_provider = sample_provider;
    // some stats on performance
    m_average_detect_time = 0;
    m_number_of_runs = 0;
}
void DetectWakeWordState::enterState()
{
    digitalWrite(LED_PIN, LOW);
    pinMode(LED_PIN, OUTPUT); // Установите пин как выход
    // Create our neural network
    m_nn = new NeuralNetwork();
    Serial.println("Created Neral Net");
    // create our audio processor
    m_audio_processor = new AudioProcessor(AUDIO_LENGTH, WINDOW_SIZE, STEP_SIZE, POOLING_SIZE);
    Serial.println("Created audio processor");

    m_number_of_detections = 0;
}
bool DetectWakeWordState::run()
{
    if (m_wake_word_detected_time > 0 &&
        (millis() - m_wake_word_detected_time) >= 5000)
    {
        digitalWrite(LED_PIN, LOW);
        m_wake_word_detected_time = 0;
    }

    // time how long this takes for stats
    long start = millis();
    // get access to the samples that have been read in
    RingBufferAccessor *reader = m_sample_provider->getRingBufferReader();
    // rewind by 1 second
    reader->rewind(16000);
    // get hold of the input buffer for the neural network so we can feed it data
    float *input_buffer = m_nn->getInputBuffer();
    // process the samples to get the spectrogram
    m_audio_processor->get_spectrogram(reader, input_buffer);
    // finished with the sample reader
    delete reader;
    // get the prediction for the spectrogram
    float output = m_nn->predict();
    Serial.printf("Detection confidence: %.4f\n", output);
    long end = millis(); 
    // compute the stats
    m_average_detect_time = (end - start) * 0.1 + m_average_detect_time * 0.9;
    m_number_of_runs++;
    // log out some timing info
    if (m_number_of_runs == 100)
    {
        m_number_of_runs = 0;
        Serial.printf("Average detection time %.fms\n", m_average_detect_time);
    }
    // use quite a high threshold to prevent false positives
    if (output > 0.70)
    {
        m_number_of_detections++;
        Serial.printf("Potential detection: count=%d, confidence=%.4f\n",
                      m_number_of_detections, output);
        if (m_number_of_detections > 1)
        {
            m_number_of_detections = 0;
            digitalWrite(LED_PIN, HIGH);
            Serial.println("WAKE WORD DETECTED!");
            m_wake_word_detected_time = millis();
            // detected the wake word in several runs, move to the next state
            Serial.printf("P(%.2f): Here I am, brain the size of a planet...\n", output);
            return true;
        }
    }
    // nothing detected stay in the current state
    return false;
}
void DetectWakeWordState::exitState()
{
    // Create our neural network
    delete m_nn;
    m_nn = NULL;
    delete m_audio_processor;
    m_audio_processor = NULL;
    digitalWrite(LED_PIN, LOW);
}
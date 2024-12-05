#ifndef _DETECT_WAKE_WORD_STATE_H_
#define _DETECT_WAKE_WORD_STATE_H_

#include "I2SSampler.h"
#include "AudioProcessor.h"
#include "NeuralNetwork.h"
#include "States.h"

class DetectWakeWordState : public State
{
public:
    DetectWakeWordState(I2SSampler *sample_provider);
    void enterState() override;
    bool run() override;
    void exitState() override;

private:
    I2SSampler *m_sample_provider;
    NeuralNetwork *m_nn;
    AudioProcessor *m_audio_processor;

    float m_average_detect_time = 0;
    int m_number_of_runs = 0;
    int m_number_of_detections = 0;
    unsigned long m_wake_word_detected_time = 0;
};

#endif

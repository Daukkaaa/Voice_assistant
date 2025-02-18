#include <Arduino.h>
#include "Application.h"
#include "state_machine/DetectWakeWordState.h"

Application::Application(I2SSampler *sample_provider)
{
    // detect wake word state - waits for the wake word to be detected
    m_detect_wake_word_state = new DetectWakeWordState(sample_provider);
    // start off in the detecting wakeword state
    m_current_state = m_detect_wake_word_state;
    m_current_state->enterState();
}

// process the next batch of samples
void Application::run()
{
    bool state_done = m_current_state->run();
    if (state_done)
    {
        m_current_state->exitState();
        // возвращаемся в состояние обнаружения wake word
        m_current_state = m_detect_wake_word_state;
        m_current_state->enterState();
    }
    vTaskDelay(10);
}

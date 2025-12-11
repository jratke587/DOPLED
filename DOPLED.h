/*
  DOPLED.h - Library for controling 24v Data Over Power LEDs
*/

#ifndef DOPLED_h
#define DOPLED_h

#include "Arduino.h"
#include "driver/rmt_tx.h"

class DOPLED
{
public:
    DOPLED(uint8_t pin, uint8_t timeBase_us);
    DOPLED(uint8_t pin);
    void begin();
    void end();
    void sendRaw(uint8_t *data, size_t size);
    void setPixelMasked(uint8_t mask, uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    void setPixelColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    void fill(uint8_t r, uint8_t g, uint8_t b);
    void setRandom(uint8_t mask, uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    void setFlags(uint8_t flagByte);
    ~DOPLED();

private:
    static const unsigned long RMT_RESOLUTION_HZ = 1000000;
    static const unsigned int RMT_TICK_US = (RMT_RESOLUTION_HZ / 1000000);

    static const unsigned int DEFAULT_TIMING_BASE_US = 70;

    static const unsigned int T0L = 1 * RMT_TICK_US;
    static const unsigned int T0H = 1 * RMT_TICK_US;
    static const unsigned int T1L = 1 * RMT_TICK_US;
    static const unsigned int T1H = 3 * RMT_TICK_US;
    static const unsigned int PRE_PKT_L = 1 * RMT_TICK_US;
    static const unsigned int INTER_PKT_H = 7 * RMT_TICK_US;

    unsigned int timebase;

    uint8_t _setCommand = 0b00110000;
    uint8_t _fillCommand = 0b00011000;
    uint8_t _randFillCommand = 0b01001000;

    struct dopled_encoder_t
    {
        rmt_encoder_t base;
        rmt_encoder_t *bytes_encoder;
        rmt_encoder_t *copy_encoder;
        int state;

        rmt_symbol_word_t leading_symbol;
        rmt_symbol_word_t ending_symbol;
    };

    uint8_t _pin;
    bool _initialized = false;
    rmt_channel_handle_t led_chan = NULL;
    rmt_encoder_handle_t dopled_encoder = NULL;
    rmt_transmit_config_t tx_config;

    static size_t rmt_encode_dopled(
        rmt_encoder_t *encoder,
        rmt_channel_handle_t channel,
        const void *primary_data,
        size_t data_size,
        rmt_encode_state_t *ret_state)
    {
        dopled_encoder_t *led_encoder = __containerof(encoder, dopled_encoder_t, base);
        rmt_encoder_handle_t bytes_encoder = led_encoder->bytes_encoder;
        rmt_encoder_handle_t copy_encoder = led_encoder->copy_encoder;
        rmt_encode_state_t session_state = RMT_ENCODING_RESET;
        rmt_encode_state_t state = RMT_ENCODING_RESET;
        size_t encoded_symbols = 0;
        switch (led_encoder->state)
        {
        case 0: // leading symbol
            encoded_symbols += copy_encoder->encode(
                copy_encoder,
                channel,
                &led_encoder->leading_symbol,
                sizeof(rmt_symbol_word_t),
                &session_state);
            if (session_state & RMT_ENCODING_COMPLETE)
            {
                led_encoder->state = 1; // we can only switch to the next state when the current encoder finished
            }
            if (session_state & RMT_ENCODING_MEM_FULL)
            {
                state = RMT_ENCODING_MEM_FULL;
                goto out; // yield if there is no free space to put other encoding artifacts
            }
        // fall-through
        case 1: // send data
            encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
            if (session_state & RMT_ENCODING_COMPLETE)
            {
                led_encoder->state = 2; // switch to next state when current encoding session finished
            }
            if (session_state & RMT_ENCODING_MEM_FULL)
            {
                state = RMT_ENCODING_MEM_FULL;
                goto out; // yield if there is no free space to put other encoding artifacts
            }
        // fall-through
        case 2: // send reset code
            encoded_symbols += copy_encoder->encode(
                copy_encoder,
                channel,
                &led_encoder->ending_symbol,
                sizeof(rmt_symbol_word_t),
                &session_state);
            if (session_state & RMT_ENCODING_COMPLETE)
            {
                led_encoder->state = RMT_ENCODING_RESET; // back to the initial encoding session
                state = RMT_ENCODING_COMPLETE;           // telling the caller the encoding has finished
            }
            if (session_state & RMT_ENCODING_MEM_FULL)
            {
                state = RMT_ENCODING_MEM_FULL;
                goto out; // yield if there is no free space to put other encoding artifacts
            }
        }
    out:
        *ret_state = state;
        return encoded_symbols;
    }

    static esp_err_t rmt_del_dopled_encoder(rmt_encoder_t *encoder)
    {
        dopled_encoder_t *led_encoder = __containerof(encoder, dopled_encoder_t, base);
        rmt_del_encoder(led_encoder->bytes_encoder);
        rmt_del_encoder(led_encoder->copy_encoder);
        free(led_encoder);
        return ESP_OK;
    }

    static esp_err_t rmt_dopled_encoder_reset(rmt_encoder_t *encoder)
    {
        dopled_encoder_t *led_encoder = __containerof(encoder, dopled_encoder_t, base);
        rmt_encoder_reset(led_encoder->bytes_encoder);
        rmt_encoder_reset(led_encoder->copy_encoder);
        led_encoder->state = RMT_ENCODING_RESET;
        return ESP_OK;
    }

    esp_err_t rmt_new_dopled_encoder(rmt_encoder_handle_t *ret_encoder);
};

#endif
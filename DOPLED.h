/**
 * @file DOPLED.h
 * @brief Driver for Data Over Power addressable LEDs.
 *
 * See https://github.com/jratke587/DOPLED for more information on supported hardware and usage.
 *
 * @note This library only supports ESP32 microcontrollers.
 *
 * @author John Ratke
 * @date 2025
 * @license MIT
 */

#ifndef DOPLED_H
#define DOPLED_H

#include "Arduino.h"
#include "driver/rmt_tx.h"

/**
 * @class DOPLED
 * @brief Driver for Data Over Power addressable LEDs.
 *
 * See https://github.com/jratke587/DOPLED for more information on supported hardware and usage.
 *
 * @note This library only supports ESP32 microcontrollers.
 */
class DOPLED
{
public:
    /**
     * Creates a DOPLED Object with a specific timebase.
     * 
     * The timebase defines the width of each pulse. Different LEDs may tolerate 
     * different timings. Typical value is 70-120us. Lower timings allow faster 
     * updates but may not work with all LEDs. 
     * 
     * @param pin ESP32 output pin
     * @param timeBase_us Timebase in microseconds (default: 70us)
     */
    DOPLED(uint8_t pin, uint8_t timeBase_us);
    /**
     * Creates a DOPLED Object with a specific timebase.
     * 
     * @note Uses a default timebase of 70 us.
     * @see DOPLED(uint8_t, uint8_t)
     * 
     * @param pin ESP32 output pin
     */
    DOPLED(uint8_t pin);
    /**
     * Must be called before sending any data to the LEDs.
     * Calling any pixel or fill function before begin() will result in no output.
     * 
     * Enables the ESP32's RMT output.
     */
    void begin();
    /**
     * Disables the ESP32's RMT output and set the pin LOW.
     * DOP LEDs use the same amount of power when displaying black as when displaying white.
     * This function should be called when the LED is off to save power.
     */
    void end();
    /**
     * @warning ADVANCED FUNCTIONALITY
     * Improper usage may cause undefined behavior.
     * 
     * @param data Byte array containing the raw data to be sent.
     * @param size Size of the byte array.
     */
    void sendRaw(uint8_t *data, size_t size);
    /**
     * Sets a group of pixels according to the mask and address.
     * 
     * @details
     * The mask determines which bits of the pixel address are checked by 
     * the LEDs. Pixels matching the specified bits will be set.
     * 
     * Ex: setPixelMasked(0b00000011, 0, 255, 0, 0) - set pixels 0, 4, 8, 12, etc. to red.
     * Ex: setPixelMasked(0b11111000, 0, 0, 0, 255) - set pixels 0-7 to blue.
     * Ex: setPixelMasked(0b00000010, 0, 0, 255, 0) - set alternating groups of 2 to green.
     * 
     * @param mask 8-bit bitmask indicating which address bits will be checked.
     * @param index 8-bit address of the first pixel to be set.
     * @param r Red value (0-255)
     * @param g Green value (0-255)
     * @param b Blue value (0-255)
     */
    void setPixelMasked(uint8_t mask, uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    /**
     * Sets a specific pixel to a specific color.
     * @param index 8-bit address of the pixel to be set.
     * @param r Red value (0-255)
     * @param g Green value (0-255)
     * @param b Blue value (0-255)
     */
    void setPixelColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    /**
     * Fills all pixels with a specific color.
     * @param r Red value (0-255)
     * @param g Green value (0-255)
     * @param b Blue value (0-255)
     */
    void fill(uint8_t r, uint8_t g, uint8_t b);
    /**
     * Creates random groups and sets them to a specific color.
     * 
     * @details
     * The mask and index specify which groups of LEDs will be set.
     * The groups are decided on the fly by the LEDs when the first random command
     * is executed. The same group can be addressed multiple times using the same
     * mask and index.
     * Sending a fill command will reset the random groups.
     * 
     * Only the last 4 bits of the mask and index are used.
     * 
     * Ex: 
     *    setRandom(0b0001, 0b0000, 255, 0, 0); - Sets random 50% of the string to red.
     *    setRandom(0b0001, 0b0001, 0, 255, 0); - Sets the other 50% to green.
     * 
     * See example project Random for more examples. 
     * 
     * @param mask 4-bit bitmask indicating which group bits will be checked.
     * @param index 4-bit address indicating the group to be addressed.
     * @param r Red value (0-255)
     * @param g Green value (0-255)
     * @param b Blue value (0-255)
     */
    void setRandom(uint8_t mask, uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    /**
     * @warning ADVANCED FUNCTIONALITY
     * These bits control undocumented behavior and may cause unexpected results.
     * 
     * Sets the last 3 bits of the command byte, which are configurable flags.
     * It is unclear what exactly these bits do.
     * 
     * @param flagByte 3-bit value containing the flags to use.
     */
    void setFlags(uint8_t flagByte);
    /**
     * Destructor for the DOPLED class.
     * Cleans up the RMT channel and encoder.
     */
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
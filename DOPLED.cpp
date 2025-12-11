#include <DOPLED.h>

DOPLED::DOPLED(uint8_t pin, uint8_t timeBase_us)
{
  _pin = pin;
  timebase = timeBase_us;
  if (timebase == 0)
  {
    timebase = DEFAULT_TIMING_BASE_US;
  }

  tx_config = {
      .loop_count = 0,
  };
  tx_config.flags.eot_level = 1;

  rmt_tx_channel_config_t tx_chan_config = {
      .gpio_num = (gpio_num_t)_pin,
      .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
      .resolution_hz = RMT_RESOLUTION_HZ,
      .mem_block_symbols = 128, // increase the block size can make the LED less flickering
      .trans_queue_depth = 4,   // set the number of transactions that can be pending in the background
  };
  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

  ESP_ERROR_CHECK(rmt_new_dopled_encoder(&dopled_encoder));
}

DOPLED::DOPLED(uint8_t pin) : DOPLED(pin, DEFAULT_TIMING_BASE_US) {}

esp_err_t DOPLED::rmt_new_dopled_encoder(rmt_encoder_handle_t *ret_encoder)
{
  dopled_encoder_t *led_encoder = NULL;
  led_encoder = (dopled_encoder_t *)rmt_alloc_encoder_mem(sizeof(dopled_encoder_t));
  if (led_encoder == NULL)
  {
    return ESP_ERR_NO_MEM;
  }

  led_encoder->base.encode = rmt_encode_dopled;
  led_encoder->base.del = rmt_del_dopled_encoder;
  led_encoder->base.reset = rmt_dopled_encoder_reset;

  led_encoder->state = RMT_ENCODING_RESET;

  led_encoder->leading_symbol = {
      .duration0 = (timebase * PRE_PKT_L) - 1,
      .level0 = 0,
      .duration1 = 1,
      .level1 = 0,
  };
  led_encoder->ending_symbol = {
      .duration0 = (timebase * INTER_PKT_H) - 1,
      .level0 = 1,
      .duration1 = 1,
      .level1 = 1,
  };

  rmt_bytes_encoder_config_t bytes_encoder_config = {
      .bit0 = {
          .duration0 = timebase * T0H,
          .level0 = 1,
          .duration1 = timebase * T0L,
          .level1 = 0,
      },
      .bit1 = {
          .duration0 = timebase * T1H,
          .level0 = 1,
          .duration1 = timebase * T1L,
          .level1 = 0,
      },
  };
  bytes_encoder_config.flags.msb_first = true;

  ESP_ERROR_CHECK(rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder->bytes_encoder));

  rmt_copy_encoder_config_t copy_encoder_config = {};

  ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_encoder_config, &led_encoder->copy_encoder));

  *ret_encoder = &led_encoder->base;
  return ESP_OK;
}

void DOPLED::begin()
{
  if (_initialized)
    return;

  // Hold signal HIGH after data is sent to allow LEDs to light
  tx_config.flags.eot_level = 1;

  ESP_ERROR_CHECK(rmt_enable(led_chan));
  _initialized = true;
}

void DOPLED::end()
{
  if (!_initialized)
    return;

  // Hold signal LOW after data is sent to turn off LEDs
  tx_config.flags.eot_level = 0;

  // Transmit a dummy symbol to update the EOT level
  rmt_symbol_word_t low_symbol = {
      .duration0 = 1,
      .level0 = 0,
      .duration1 = 1,
      .level1 = 0,
  };
  rmt_transmit(led_chan, dopled_encoder, &low_symbol, sizeof(low_symbol), &tx_config);

  ESP_ERROR_CHECK(rmt_disable(led_chan));
  _initialized = false;
}

void DOPLED::sendRaw(uint8_t *data, size_t size)
{
  if (!_initialized)
    return;
  if (data == NULL || size == 0)
    return;

  // Wait for previous packets to finish sending
  ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));

  ESP_ERROR_CHECK(rmt_transmit(
      led_chan,
      dopled_encoder,
      data,
      size,
      &tx_config));
}

void DOPLED::setPixelMasked(uint8_t mask, uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t pkt[6];
  pkt[0] = _setCommand;
  pkt[1] = mask;
  pkt[2] = index;
  pkt[3] = r;
  pkt[4] = g;
  pkt[5] = b;

  sendRaw(pkt, sizeof(pkt));
};

void DOPLED::setPixelColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
  setPixelMasked(0xFF, index, r, g, b);
};

void DOPLED::fill(uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t pkt[4];
  pkt[0] = _fillCommand;
  pkt[1] = r;
  pkt[2] = g;
  pkt[3] = b;

  sendRaw(pkt, sizeof(pkt));
};

void DOPLED::setRandom(uint8_t mask, uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t pkt[5];
  pkt[0] = _randFillCommand;
  pkt[1] = ((mask & 0x0F) << 4) | (index & 0x0F);
  pkt[2] = r;
  pkt[3] = g;
  pkt[4] = b;

  sendRaw(pkt, sizeof(pkt));
};

void DOPLED::setFlags(uint8_t flagByte)
{
  _setCommand = (_setCommand & 0b11111000) | (flagByte & 0b00000111);
  _fillCommand = (_fillCommand & 0b11111000) | (flagByte & 0b00000111);
  _randFillCommand = (_randFillCommand & 0b11111000) | (flagByte & 0b00000111);
};

DOPLED::~DOPLED()
{
  if (led_chan)
  {
    rmt_disable(led_chan);
    rmt_del_channel(led_chan);
  }
  if (dopled_encoder)
  {
    rmt_del_encoder(dopled_encoder);
  }
}

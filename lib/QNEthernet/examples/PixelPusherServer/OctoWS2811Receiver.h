// SPDX-FileCopyrightText: (c) 2022-2024 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// OctoWS2811Receiver.h defines a Receiver implementation that uses an
// OctoWS2811 for the LEDs.
//
// This file is part of the QNEthernet library.

#pragma once

// C++ includes
#include <cstddef>
#include <cstdint>
#include <memory>

#include <OctoWS2811.h>

#include "PixelPusherServer.h"
#include "Receiver.h"

class OctoWS2811Receiver : public Receiver {
 public:
  // Creates a new object. The parameters are restricted:
  // * 'numStrips': 0-255
  OctoWS2811Receiver(PixelPusherServer &pp,
                     size_t numStrips,
                     size_t pixelsPerStrip);
  ~OctoWS2811Receiver() override = default;

  // Initializes the receiver. This performs tasks that can't be done
  // upon program creation. For example, OctoWS2811::begin() can't be
  // called until setup() or the program will crash.
  //
  // This will always return true.
  bool begin() override;

  void end() override {}

  size_t numStrips() const override {
    return numStrips_;
  };

  size_t pixelsPerStrip() const override {
    return pixelsPerStrip_;
  }

  uint8_t stripFlags(size_t stripNum) const override;
  void handleCommand(uint8_t command, const uint8_t *data, size_t len) override;
  void startPixels(bool complete) override {}
  void pixels(size_t stripNum, const uint8_t *pixels,
              size_t pixelsPerStrip) override;
  void endPixels() override;
  void loop() override {}

 private:
  PixelPusherServer &pp_;
  const size_t numStrips_;
  const size_t pixelsPerStrip_;

  std::unique_ptr<uint8_t[]> displayMem_;
  std::unique_ptr<uint8_t[]> drawingMem_;
  OctoWS2811 leds_;

  uint16_t globalBri_ = UINT16_MAX;
};

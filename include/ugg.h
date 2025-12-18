// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

#pragma once

// Uncomplicated Generic Graphics
struct ugg {
	int32_t width;
	int32_t height;
	uint32_t palette[256];
	uint8_t  data[];
};


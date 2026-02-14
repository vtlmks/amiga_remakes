// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

#pragma once

#define BMP_MAGIC (uint16_t)('B' | 'M' << 8)

struct bmp_image {
	int32_t width;
	int32_t height;
	uint32_t num_colors;
	uint8_t *palette;		// points into filedata (BGRA, 4 bytes per entry)
	uint8_t *pixels;		// caller must free()
};

#pragma pack(push,1)
struct bmp_header {
	uint16_t type;							// Magic identifier
	uint32_t size;							// File size in bytes
	uint16_t res1;							//
	uint16_t res2;							//
	uint32_t offset_to_image_data;	// Offset to image data in bytes
	uint32_t info_size;					// Header size in bytes
	int32_t width;							// Width of image
	int32_t height;						// Height of image, if negative we will flip the image.
	uint16_t planes;						// Number of color planes
	uint16_t bits;							// Bits per pixel    NOTE(peter): We will only be handling 1, 4 and 8 bits per pixel
	uint32_t compression;				// Compression type  NOTE(peter): We will handle: (0) BI_RGB, (1) BI_RLE8 (2) BI_RLE4 and (3) BI_BITFIELDS without HUFFMAN 1D
	uint32_t imagesize;					// Image size in bytes
	int32_t x_ppm;							// Pixels per meter
	int32_t y_ppm;							// Pixels per meter
	uint32_t n_colors;					// Number of colors
	uint32_t important_colors;			// Important colors
};
#pragma pack(pop)

// - -=-==<{[ bmp_read_pixels_1 ]}>==-=- -
static void bmp_read_pixels_1(struct bmp_header *header, uint8_t *filedata, uint8_t *output) {
	uint8_t *src = filedata + header->offset_to_image_data;

	uint32_t image_height = (header->height < 0) ? -header->height : header->height;
	uint32_t pixels_per_row = (header->width + 7) >> 3;
	uint32_t bytes_per_row = (pixels_per_row + 3) & ~0x3;

	for(uint32_t y = 0; y < image_height; y++) {
		for(uint32_t x = 0; x < (uint32_t)header->width; x++) {
			uint8_t byte = src[x >> 3];
			*output++ = (byte >> (7 - (x & 7))) & 0x1;
		}
		src += bytes_per_row;
	}
}

// - -=-==<{[ bmp_read_pixels_4 ]}>==-=- -
static void bmp_read_pixels_4(struct bmp_header *header, uint8_t *filedata, uint8_t *output) {
	uint8_t *src = filedata + header->offset_to_image_data;

	uint32_t image_height = (header->height < 0) ? -header->height : header->height;
	uint32_t pixels_per_row = (header->width + 1) >> 1;
	uint32_t bytes_per_row = (pixels_per_row + 3) & ~0x3;

	for(uint32_t y = 0; y < image_height; y++) {
		for(uint32_t x = 0; x < (uint32_t)header->width; x++) {
			uint8_t byte = src[x >> 1];
			*output++ = (x & 1) ? byte & 0xf : byte >> 4;
		}
		src += bytes_per_row;
	}
}

// - -=-==<{[ bmp_read_pixels_8 ]}>==-=- -
static void bmp_read_pixels_8(struct bmp_header *header, uint8_t *filedata, uint8_t *output) {
	uint32_t bytes_per_row = (header->width + 3) & ~0x3;
	uint32_t image_height = (header->height < 0) ? -header->height : header->height;

	uint8_t *src = filedata + header->offset_to_image_data;
	for(uint32_t y = 0; y < image_height; ++y) {
		for(uint32_t x = 0; x < (uint32_t)header->width; ++x) {
			*output++ = src[x];
		}
		src += bytes_per_row;
	}
}

// - -=-==<{[ bmp_read_pixels_rle4 ]}>==-=- -
static void bmp_read_pixels_rle4(struct bmp_header *header, uint8_t *filedata, uint8_t *output) {
	uint32_t image_height = (header->height < 0) ? -header->height : header->height;

	bool keep_going = true;
	uint32_t current_line = 0;
	uint8_t *src = filedata + header->offset_to_image_data;
	uint8_t *dst = output;

	while(keep_going) {
		uint8_t byte1 = *src++;
		if(byte1) {
			uint8_t byte2 = *src++;
			for(uint32_t i = 0; i < (byte1 >>1); ++i) {
				*dst++ = byte2 >> 4;
				*dst++ = byte2 & 0xf;
			}

			if(byte1&1) {
				*dst++ = byte2 >> 4;
			}

		} else {
			uint8_t byte2 = *src++;
			switch(byte2) {
				case 0: {
					current_line++;
					dst = output + current_line * header->width;
				} break;

				case 1: {
					keep_going = false;
				} break;

				case 2: {
					uint8_t x_offset = *src++;
					uint8_t y_offset = *src++;
					current_line += y_offset;
					dst += y_offset * header->width + x_offset;
				} break;

				default: {
					for(uint32_t i = 0; i < (byte2 >> 1); ++i) {
						byte1 = *src++;
						*dst++ = byte1 >> 4;
						*dst++ = byte1 & 0xf;
					}
					if(byte2 & 0x1) {
						byte1 = *src++;
						*dst++ = byte1 >> 4;
					}
					src += ((byte2+1)>>1) & 0x1;

				} break;
			}
		}
		keep_going = (dst < output + (header->width * image_height));
	}
}

// - -=-==<{[ bmp_read_pixels_rle8 ]}>==-=- -
static void bmp_read_pixels_rle8(struct bmp_header *header, uint8_t *filedata, uint8_t *output) {
	uint32_t image_height = (header->height < 0) ? -header->height : header->height;

	bool keep_going = true;
	uint32_t current_line = 0;
	uint8_t *src = filedata + header->offset_to_image_data;
	uint8_t *dst = output;

	while(keep_going) {
		uint8_t byte = *src++;
		if(byte) {
			uint8_t index = *src++;
			for(uint32_t i = 0; i < byte; ++i) {
				*dst++ = index;
			}
		} else {
			byte = *src++;
			switch(byte) {
				case 0: {
					current_line++;
					dst = output + current_line*header->width;
				} break;

				case 1: {
					keep_going = false;
				} break;

				case 2: {
					uint8_t x_offset = *src++;
					uint8_t y_offset = *src++;
					current_line += y_offset;
					dst += y_offset * header->width + x_offset;
				} break;

				default: {
					for(uint32_t i = 0; i < byte; ++i) {
						uint8_t index = *src++;
						*dst++ = index;
					}
					src += byte & 0x1;
				} break;
			}
		}

		keep_going = (dst < output + (header->width * image_height));
	}
}

// - -=-==<{[ bmp_decode ]}>==-=- -
// Decodes a BMP file from memory. Returns 0 on success, non-zero on error.
// On success, out->pixels must be freed by the caller.
// out->palette points into filedata and is only valid while filedata is alive.
static int bmp_decode(const uint8_t *filedata, size_t filesize, struct bmp_image *out) {
	if(filesize < sizeof(struct bmp_header)) {
		return -1;
	}

	struct bmp_header *header = (struct bmp_header *)filedata;
	if(header->type != BMP_MAGIC) {
		return -1;
	}

	uint32_t image_height = (header->height < 0) ? -header->height : header->height;
	size_t output_image_size = header->width * image_height;

	uint8_t *pixels = malloc(output_image_size);
	if(pixels == 0) {
		return -1;
	}

	switch(header->bits) {
		case 1: {
			bmp_read_pixels_1(header, (uint8_t *)filedata, pixels);
		} break;

		case 4: {
			if(header->compression == 2) {
				bmp_read_pixels_rle4(header, (uint8_t *)filedata, pixels);
			} else {
				bmp_read_pixels_4(header, (uint8_t *)filedata, pixels);
			}
		} break;

		case 8: {
			if(header->compression == 1) {
				bmp_read_pixels_rle8(header, (uint8_t *)filedata, pixels);
			} else {
				bmp_read_pixels_8(header, (uint8_t *)filedata, pixels);
			}
		} break;

		default: {
			free(pixels);
			return -1;
		} break;
	}

	// Flip bottom-up images
	if(header->height > 0) {
		uint32_t row_size = header->width;
		uint32_t half_height = image_height / 2;
		for(uint32_t y = 0; y < half_height; ++y) {
			uint8_t *row1 = pixels + y * row_size;
			uint8_t *row2 = pixels + (image_height - 1 - y) * row_size;
			for(uint32_t x = 0; x < (uint32_t)header->width; ++x) {
				uint8_t temp = row1[x];
				row1[x] = row2[x];
				row2[x] = temp;
			}
		}
	}

	out->width = header->width;
	out->height = image_height;
	out->num_colors = header->n_colors ? header->n_colors : (1u << header->bits);
	out->palette = (uint8_t *)filedata + sizeof(struct bmp_header);
	out->pixels = pixels;
	return 0;
}

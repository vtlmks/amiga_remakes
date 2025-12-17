#define bmp_magic (uint16_t)('B' | 'M' << 8)

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
void bmp_read_pixels_1(struct bmp_header *header, uint8_t *filedata, uint8_t *output) {
	// printf("bmp_read_pixels_1()...\n");
	uint8_t *src = filedata + header->offset_to_image_data;

	uint32_t image_height = (header->height < 0) ? -header->height : header->height;
	uint32_t pixels_per_row = (header->width + 7) >> 3;		// aligned to byte, we are counting bits here.
	uint32_t bytes_per_row = (pixels_per_row + 3) & ~0x3;

#if 0
	printf("pixels_per_row: %d\n", pixels_per_row);
	printf("bytes_per_row: %d\n", bytes_per_row);
#endif

	for(uint32_t y = 0; y < image_height; y++) {
		for(uint32_t x = 0; x < header->width; x++) {
			uint8_t byte = src[x >> 3];
			*output++ = (byte >> (7 - (x & 7))) & 0x1;
		}
		src += bytes_per_row;
	}
}

// - -=-==<{[ bmp_read_pixels_4 ]}>==-=- -
void bmp_read_pixels_4(struct bmp_header *header, uint8_t *filedata, uint8_t *output) {
	// printf("bmp_read_pixels_4()...\n");
	uint8_t *src = filedata + header->offset_to_image_data;

	uint32_t image_height = (header->height < 0) ? -header->height : header->height;
	uint32_t pixels_per_row = (header->width + 1) >> 1;		// aligned to byte, we are counting nibbles here.
	uint32_t bytes_per_row = (pixels_per_row + 3) & ~0x3;

	for(uint32_t y = 0; y < image_height; y++) {
		for(uint32_t x = 0; x < header->width; x++) {
			uint8_t byte = src[x >> 1];
			*output++ = (x & 1) ? byte & 0xf : byte >> 4;
		}
		src += bytes_per_row;
	}
}

// - -=-==<{[ bmp_read_pixels_8 ]}>==-=- -
void bmp_read_pixels_8(struct bmp_header *header, uint8_t *filedata, uint8_t *output) {
	// printf("bmp_read_pixels_8()...\n");
	uint32_t bytes_per_row = (header->width + 3) & ~0x3;
	uint32_t image_height = (header->height < 0) ? -header->height : header->height;

	uint8_t *src = filedata + header->offset_to_image_data;
	for(uint32_t y = 0; y < image_height; ++y) {
		for(uint32_t x = 0; x < header->width; ++x) {
			*output++ = src[x];
		}
		src += bytes_per_row;
	}
}

// - -=-==<{[ bmp_read_pixels_rle4 ]}>==-=- -
void bmp_read_pixels_rle4(struct bmp_header *header, uint8_t *filedata, uint8_t *output) {
	// printf("bmp_read_pixels_rle4()...\n");
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
void bmp_read_pixels_rle8(struct bmp_header *header, uint8_t *filedata, uint8_t *output) {
	// printf("bmp_read_pixels_rle8()...\n");
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

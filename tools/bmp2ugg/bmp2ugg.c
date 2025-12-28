// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

#define _CRT_SECURE_NO_WARNINGS
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ugg.h"
#include "bmp.c"

// - -=-==<{[ print_usage ]}>==-=- -
static void print_usage(char **argv)
{
	printf("\033[0;31mUsage:\033[0m %s -o output_directory [-p] [-r remove_chars] [-a append_chars] inputfile1 [inputfile2 ...]\n", argv[0]);
	printf("\n");
	printf("\033[0;31mOptions:\033[0m\n");
	printf("  -o output_directory  Specify the destination directory for output files\n");
	printf("  -p                   Display a preview of the output image (only for a single file)\n");
	printf("  -r remove_chars      Remove the first N characters from the input filename\n");
	printf("  -a append_chars      Use the first N characters (before removal) as a subdirectory name\n");
}

// - -=-==<{[ handle_errno ]}>==-=- -
void handle_errno(char *filename) {
	switch(errno) {
		case ENOENT: {
			// printf("The file `%s` does not exist.\n", filename);
		} break;
		case EACCES: {
			// printf("The file `%s` cannot be accessed due to insufficient permissions.\n", filename);
		} break;
	}
}

// - -=-==<{[ conversion_task ]}>==-=- -
struct conversion_task {
	char *input_file;
	char *out_dir;
	bool preview_flag;
	int remove_chars;
	int append_chars;
};

// - -=-==<{[ convert_file ]}>==-=- -
static void *convert_file(void *arg) {
	struct conversion_task *task = (struct conversion_task *)arg;
	FILE *f = fopen(task->input_file, "rb");
	if(f == 0) {
		handle_errno(task->input_file);
		free(task);
		return 0;
	}

	fseek(f, 0, SEEK_END);
	uint32_t filesize = ftell(f);
	rewind(f);

	// Read file data
	uint8_t *filedata = malloc(filesize);
	if(filedata == 0) {
		printf("Failed to allocate memory for file data: %s\n", task->input_file);
		fclose(f);
		free(task);
		return 0;
	}
	size_t read_count = fread(filedata, 1, filesize, f);
	fclose(f);
	if(read_count != filesize) {
		printf("Failed to read file data: %s\n", task->input_file);
		free(filedata);
		free(task);
		return 0;
	}

	// Process BMP header
	struct bmp_header *header = (struct bmp_header *)filedata;
	if(header->type != bmp_magic) {
		printf("File %s is not a valid BMP file.\n", task->input_file);
		free(filedata);
		free(task);
		return 0;
	}

	size_t output_image_size = header->width * header->height;
	uint8_t *output = malloc(output_image_size);
	if(output == 0) {
		printf("Failed to allocate memory for output image: %s\n", task->input_file);
		free(filedata);
		free(task);
		return 0;
	}

	// Process pixel data based on bit depth
	switch(header->bits) {
		case 1: {
			bmp_read_pixels_1(header, filedata, output);
		} break;

		case 4: {
			if(header->compression == 2) {
				bmp_read_pixels_rle4(header, filedata, output);
			} else {
				bmp_read_pixels_4(header, filedata, output);
			}
		} break;

		case 8: {
			if(header->compression == 1) {
				bmp_read_pixels_rle8(header, filedata, output);
			} else {
				bmp_read_pixels_8(header, filedata, output);
			}
		} break;

		default: {
			printf("WARN: Not a supported bit depth in file %s!\n", task->input_file);
			free(output);
			free(filedata);
			free(task);
			return 0;
		} break;
	}

	// Flip image if needed
	if(header->height > 0) {
		uint32_t row_size = header->width;
		uint32_t half_height = header->height / 2;
		for(uint32_t y = 0; y < half_height; y++) {
			uint8_t *row1 = output + y * row_size;
			uint8_t *row2 = output + (header->height - 1 - y) * row_size;
			for(uint32_t x = 0; x < header->width; x++) {
				uint8_t temp_pixel = row1[x];
				row1[x] = row2[x];
				row2[x] = temp_pixel;
			}
		}
	}

	// If preview flag is enabled, print a preview (only applicable for a single file)
	if(task->preview_flag) {
		uint8_t *src = output;
		// ASCII characters ordered from darkest (most coverage) to lightest (least coverage)
		const char *intensity_map = " .:-=+*#%@";
		uint32_t map_len = strlen(intensity_map);

		// Build lookup table from color index to intensity character
		char color_to_char[256];
		size_t num_colors = header->n_colors ? header->n_colors : (1 << header->bits);
		uint8_t *palette_data = filedata + sizeof(struct bmp_header);
		for(uint32_t i = 0; i < num_colors; i++) {
			// Get RGB values for this palette entry (stored as BGRA)
			uint8_t b = palette_data[i * 4 + 0];
			uint8_t g = palette_data[i * 4 + 1];
			uint8_t r = palette_data[i * 4 + 2];

			// Calculate perceived brightness (weighted for human perception)
			// Using standard luminance formula: 0.299*R + 0.587*G + 0.114*B
			uint32_t intensity = (299 * r + 587 * g + 114 * b) / 1000;

			// Map intensity (0-255) to character index
			uint32_t char_idx = (intensity * (map_len - 1)) / 255;
			color_to_char[i] = intensity_map[char_idx];
		}

		for(uint32_t y = 0; y < header->height; y++) {
			for(uint32_t x = 0; x < header->width; x++) {
				printf("%c", color_to_char[*src]);
				src++;
			}
			printf("\n");
		}
	}

	/*
	* Generate file header and output file
	*/
	// Extract basename from input_file
	char *base = strrchr(task->input_file, '/');
	if(base == 0) {
		base = task->input_file;
	} else {
		base++;
	}

	// Duplicate basename for manipulation
	char sanitized[256];
	memset(sanitized, 0, sizeof(sanitized));
	strncpy(sanitized, base, sizeof(sanitized) - 1);

	// Prepare subdirectory name from first 'append_chars' if specified
	char subdir[256] = "";
	if(task->append_chars > 0) {
		if((size_t)task->append_chars > strlen(sanitized))
			task->append_chars = strlen(sanitized);
		strncpy(subdir, sanitized, task->append_chars);
		subdir[task->append_chars] = '\0';
		// Sanitize subdir: replace '.', '-' and space with '_'
		for(char *p = subdir; *p; p++) {
			if(*p == '.' || *p == '-' || *p == ' ') {
				*p = '_';
			}
		}
	}

	// Remove extension: find last dot and truncate
	char *dot = strrchr(sanitized, '.');
	if(dot != 0) {
		*dot = '\0';
	}

	// Remove first N characters if option is set
	if(task->remove_chars > 0) {
		size_t len = strlen(sanitized);
		if((size_t)task->remove_chars < len) {
			memmove(sanitized, sanitized + task->remove_chars, len - task->remove_chars + 1);
		} else {
			sanitized[0] = '\0';
		}
	}

	// Sanitize filename: replace '.', '-' and space with '_'
	for(char *p = sanitized; *p; p++) {
		if(*p == '.' || *p == '-' || *p == ' ') {
			*p = '_';
		}
	}

	// Build final output directory path
	char final_out_dir[512];
	if(task->append_chars > 0 && strlen(subdir) > 0) {
		snprintf(final_out_dir, sizeof(final_out_dir), "%s/%s", task->out_dir, subdir);
		// Create the subdirectory if it doesn't exist (ignoring error if it exists)
		mkdir(final_out_dir, 0777);
	} else {
		snprintf(final_out_dir, sizeof(final_out_dir), "%s", task->out_dir);
	}

	// Construct final output file path: final_out_dir/sanitized.ugg
	char out_path[512];
	snprintf(out_path, sizeof(out_path), "%s/%s.ugg", final_out_dir, sanitized);

	FILE *out_file = fopen(out_path, "wb");
	if(out_file == 0) {
		printf("Failed to open output file: %s\n", out_path);
		free(output);
		free(filedata);
		free(task);
		return 0;
	}

	struct ugg ugg_data = {0};
	ugg_data.width = header->width;
	ugg_data.height = header->height;

	size_t num_colors = header->n_colors ? header->n_colors : (1 << header->bits);
	uint8_t *color_data = filedata + sizeof(struct bmp_header);
	for(size_t i = 0; i < num_colors; i++) {
		uint8_t b = *color_data++;
		uint8_t g = *color_data++;
		uint8_t r = *color_data++;
		uint8_t a = i ? 0xff : 0x00;
		color_data++; // skip the real alpha value
		ugg_data.palette[i] = (r << 24) | (g << 16) | (b << 8) | a;
	}

	fwrite(&ugg_data, sizeof(ugg_data), 1, out_file);
	fwrite(output, sizeof(uint8_t), output_image_size, out_file);
	fclose(out_file);

	// Clean up allocated memory
	free(output);
	free(filedata);
	free(task);
	return 0;
}

// - -=-==<{[ main ]}>==-=- -
int main(int argc, char **argv)
{
	int opt;
	char *out_dir = 0;
	bool preview_flag = false;
	int remove_chars = 0;
	int append_chars = 0;

	// Parse options
	while((opt = getopt(argc, argv, "o:pr:a:")) != -1) {
		switch(opt) {
		case 'o': {
			out_dir = optarg;
		} break;
		case 'p': {
			preview_flag = true;
		} break;
		case 'r': {
			remove_chars = atoi(optarg);
		} break;
		case 'a': {
			append_chars = atoi(optarg);
		} break;
		default: {
			print_usage(argv);
			return 0;
		} break;
		}
	}

	// The remaining arguments are the input files
	int num_input_files = argc - optind;
	if(out_dir == 0 || num_input_files <= 0) {
		print_usage(argv);
		return 0;
	}

	// Create the main destination directory if it does not exist
	struct stat st = {0};
	if(stat(out_dir, &st) == -1) {
		if(mkdir(out_dir, 0777) != 0) {
			perror("Failed to create output directory");
			return 1;
		}
	}

	// If more than one file, disable preview
	if(num_input_files > 1) {
		preview_flag = false;
	}

	pthread_t *threads = malloc(num_input_files * sizeof(pthread_t));
	if(threads == 0) {
		printf("Failed to allocate memory for threads.\n");
		return 1;
	}

	for(int i = 0; i < num_input_files; i++) {
		struct conversion_task *task = malloc(sizeof(struct conversion_task));
		if(task == 0) {
			printf("Failed to allocate memory for conversion task.\n");
			continue;
		}
		task->input_file = argv[optind + i];
		task->out_dir = out_dir;
		task->preview_flag = preview_flag;
		task->remove_chars = remove_chars;
		task->append_chars = append_chars;

		if(pthread_create(&threads[i], 0, convert_file, (void *)task) != 0) {
			printf("Failed to create thread for file: %s\n", task->input_file);
			free(task);
		}
	}

	for(int i = 0; i < num_input_files; i++) {
		pthread_join(threads[i], 0);
	}
	free(threads);
	return 0;
}

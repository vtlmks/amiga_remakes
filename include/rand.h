// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>  // For file I/O
#include <unistd.h> // For read and close
#ifdef _WIN32
#include <windows.h> // For Windows-specific functions
#endif

struct rng_state { uint32_t x, y, z, w; };

// A Better behaved random number generator, this is slow(!)
uint32_t mks_rand(uint32_t max);

// Xor-rng good enough random numbers very fast...
void xor_init_rng(struct rng_state *state, uint32_t seed);
uint32_t xor_generate_random(struct rng_state *state);


#ifdef RAND_IMPLEMENTATION


// TODO(peter): replace with PCG
/*
struct rng_state { uint64_t state, inc; };

void pcg_init_rng(struct rng_state *s, uint64_t seed, uint64_t stream) {
    s->state = 0;
    s->inc = (stream << 1u) | 1u;
    pcg_generate_random(s);
    s->state += seed;
    pcg_generate_random(s);
}

uint32_t pcg_generate_random(struct rng_state *s) {
    uint64_t old = s->state;
    s->state = old * 6364136223846775807ULL + s->inc;
    uint32_t xorshifted = ((old >> 18u) ^ old) >> 27u;
    uint32_t rot = old >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}
*/


static uint32_t splitmix32(uint32_t *x) {
	uint32_t z = (*x += 0x9e3779b9);
	z = (z ^ (z >> 16)) * 0x85ebca6b;
	z = (z ^ (z >> 13)) * 0xc2b2ae35;
	return z ^ (z >> 16);
}

void xor_init_rng(struct rng_state *s, uint32_t seed) {
	uint32_t x = seed;
	s->x = splitmix32(&x);
	s->y = splitmix32(&x);
	s->z = splitmix32(&x);
	s->w = splitmix32(&x);
}

uint32_t xor_generate_random(struct rng_state *state) {
	uint32_t t = state->x ^ (state->x << 11);
	state->x = state->y;
	state->y = state->z;
	state->z = state->w;
	state->w = (state->w ^ (state->w >> 19)) ^ (t ^ (t >> 8));
	return state->w;
}

/*
 *     A Better behaved random number generator, this is slow(!)
 */
uint32_t mks_rand(uint32_t max) {
	uint32_t r;
#ifdef _WIN32
	HCRYPTPROV hCryptProv;
	if(CryptAcquireContext(&hCryptProv, 0, "Microsoft Base Cryptographic Provider v1.0", PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		if(!CryptGenRandom(hCryptProv, sizeof(r), (BYTE*)&r)) {
			// fprintf(stderr, "Error generating random number on Windows\n");
			exit(EXIT_FAILURE);
		}
		// Always release the context after use
		if(!CryptReleaseContext(hCryptProv, 0)) {
			// fprintf(stderr, "Failed to release context: %ld\n", GetLastError());
		}
	} else {
		// fprintf(stderr, "Failed to acquire cryptographic context: %ld\n", GetLastError());
		exit(EXIT_FAILURE);
	}
#else
	int fd = open("/dev/urandom", O_RDONLY);
	if(fd == -1) {
		// perror("Error opening /dev/urandom");
		exit(EXIT_FAILURE);
	}
	if(read(fd, &r, sizeof(r)) != sizeof(r)) {
		// perror("Error reading from /dev/urandom");
		exit(EXIT_FAILURE);
	}
	close(fd);
#endif
	return (uint64_t)r * max >> 32;
}

#endif

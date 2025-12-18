// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

// [=]===^=[ base setup ]============================================================^===[=]

#define WINDOW_WIDTH 360
#define WINDOW_HEIGHT 270
#define BUFFER_WIDTH  (346 << 0)
#define BUFFER_HEIGHT (270 << 0)

#include "platform.c"

// [=]===^=[ remake stuff below ]============================================================^===[=]

INCBIN_BYTES(chambers_music, "data/chambers_of_shaolin_training_1_(subsong_1_&_2).smod");
INCBIN_UGG(large_star, "data/large_star.ugg");
INCBIN_UGG(small_star, "data/small_star.ugg");
INCBIN_UGG(powerhouse_logo, "data/powerhouse_logo.ugg");
INCBIN_UGG(powerhouse_trees, "data/powerhouse_trees.ugg");
INCBIN_UGG(powerhouse_font, "data/powerhouse_font.ugg");

static struct fc14_state remake_song;

// [=]===^=[ simple PCG variant ]============================================================^===[=]

static uint32_t rng_state = 1;

static void init_rng_seed(uint32_t seed) {
	if(seed == 0) {
		seed = 1;
	}
	rng_state = seed;
}

static uint32_t rng_next(void) {
	rng_state = rng_state * 1664525u + 1013904223u;
	return rng_state;
}

// [=]===^=[ audio_callback ]============================================================^===[=]
static void remake_audio_callback(int16_t *data, size_t frames) {
	// PROFILE_FUNCTION();
	// memset(data, 0, 2*2*frames);
	// micromod_get_audio(&ctx, (short*)data, frames);
	fc14_get_audio(&remake_song, data, frames);


	// NOTE(peter): Enable for 75% mix if the player doesn't have that functionality!
#if 0
	for(size_t i = 0; i < frames; i++) {
		int32_t old_left = (int32_t)data[i * 2];
		int32_t old_right = (int32_t)data[i * 2 + 1];

		int32_t mixed_left = old_left + (old_right * 3) / 4;
		int32_t mixed_right = old_right + (old_left * 3) / 4;

		// Shift right by 1 to prevent clipping and scale down
		data[i * 2] = (int16_t)(mixed_left >> 1);
		data[i * 2 + 1] = (int16_t)(mixed_right >> 1);
	}
#endif
}


// [=]===^=[ render_scroll_buffer ]============================================================^===[=]


static uint32_t scroll_colors[] = {
	0x00000000, 0x337733ff, 0x559955ff, 0x77bb77ff, 0x99dd99ff, 0xbbccbbff, 0x99dd99ff, 0x77bb77ff, 0x000000ff,
};

static uint8_t scrolltext[] = {
	"                            POWER HOUSE GIVE YOU ANOTHER PRODUC"
	"TION FOR YOUR TOTAL ENJOYMENT + SATISFACTION, CALLED     - NIGH"
	"TFALL -     THIS BUNDLE OF FLUFF WAS BROUGHT TO YOUR SCREEN BY "
	"THE FOLLOWING CREATORS.                         CODE + FONT - P"
	"REDATOR! (FONT CONVERTED FROM AN INTRO ON THE COOL C64, BY  + H"
	"OTLINE +  CODED BY TSS (THE SILVER SURFER))                    "
	"                  LOGO - HEADHUNTER                            "
	"                 PICTURESQUE GFX COURTESY OF - THOMAS HEINRICH "
	"FROM THE GAME  %MYSTERIOUS WORLDS* RIPPED AND MODIFIED A TOUCH "
	"BY ME (PRED!) + THE TUNE FROM THE GAME  %CHAMBERS OF SHAOLIN*  "
	"COMPOSED BY THE MASTER OF CHIP ATMOSPHERE  - JOCHEN HIPPEL.    "
	"LOOK OUT FOR OUR MUSIC DISK       - POWER MUSIX II -       FEAT"
	"URING CHIP MUSIC BY COBRA OF   %P-H*   MAKE SURE YOU GET HOLD O"
	"F THE FULLY WORKING VERSION.  THERES A DUFF DISK FLOATING AROUN"
	"D!!     A QUICK ADD FOR SPIDEY 9009,  CALL HIS BOARD (BRIGHT ST"
	"AR) ON  ++44 21 357 9375        NOW SOME FUNKADELIC GREETS TO T"
	"HOSE GUYS WHO KNOW ME + MAYBE HATE ME TOO!!        + COBRA +  T"
	"HE MAN OF INFINITE WISDOM! IF THEY CANT TAKE IT, THEN LET THEM "
	"GET STRESSED + SWEAT IT OUT!!        + HEADHUNTER +  I SAID I W"
	"OULD DO IT, AND HERE IS THE EVIDENCE! MANY THANX FOR THE LOGO, "
	"IT LOOKS TOTALLY SEXUAL!        + R-TYME / ARENA +  ALL THE BES"
	"T WITH THE NEW CREW, I HOPE TO SEE YA SOME DAY! (TO GET MY DISK"
	"S!! HA,HA!)        + WILDTHING / ARENA +  I LOVE THOSE INTROS /"
	" MENU SCREENS, REAL COOL!        + KODAK +  HOPE YA GET THE GAM"
	"E RELEASED SOON        + PENDRAGON +  YES THIS IS REAL, I HOPE "
	"YOU LIKE THE MYSTIC ATMOSPHERE, BEST VIBES TO LITTLE - ROB - OR"
	" IS HE BIG NOW?       + MOUNTAIN MAN +  CLIMBED ANY LATELY? - I"
	" LOOK FORWARD TO THE SUMMER BARBECUES!  ANY CHANCE OF - DUCK DE"
	" LA PARK -, I THOUGHT NOT!        + THE Q / FINAL FRONTIER +  N"
	"ICE SENDINGS MATEY CHOPS! HOPE THE MAG COMES INTO EFFECT, WITH "
	"EXCLUSIVE CONTRIBS FROM - TOBIAS RICHTER - THINGS ARE LOOKING G"
	"OOD. THIS ADD WILL COST YA!!        + FRAME +  KEEP AT IT, TIME"
	" WILL MAKE IT HAPPEN! SEE YA SOME DAY SOON.        + MARK H +  "
	"FEEL FREE TO GET IN TOUCH, ITS BEEN A WHILE        + MATT +  HA"
	"VE YOU LINKED THE CAPACITOR (25,000 VOLT CHARGE) TO THE DOOR HA"
	"NDLE YET? OR DID THE PROSPECT OF 1ST DEGREE MURDER CHANGE YOUR "
	"MIND?        + NURGLE / TETRAGON +  LOVED THE MUSIC DISC, SAY H"
	"I TO MR.BAP FOR ME O.K? (HIS HAMSTER IS A WILD GUY!)        + Q"
	"IX-SND +  CAN YOU BELIEVE HOW SIMPLE IT WAS, LET THOSE COLOURS "
	"FLOW! (ABOUT TIME TOO!)        + GENOSIDE +  SORRY YA NO LONGER"
	" AROUND THE ENVIRONMENT, TO THINK, YOUR MISSING OUT ON ALL THOS"
	"E RPG-III PROGRAMMING LECTURES, I LOVE IT I DO! (BOLLOX DO I!) "
	"       + TERMINATOR / CO-OP (ST) +  MANY THANX FOR THE HELP, AS"
	" QIX-SND WILL TELL YA, I GOT THE THEORY AND CHANGED IT A LITTLE"
	" + WOW! NICE EFFECT!        + NICK +  10 OUT OF 10 FOR SATIRICA"
	"L COMMENTS! I HOPE YOU GET THE THE CAR YOU LIKE!        + TOXIC"
	" +  WHY NO CORRESPONDENCE? (IF YOU DONT GET IN TOUCH, I WILL!) "
	"       + SLAYER / IPB +  YOUR HELP WITH DBASE IS GREATLY APPREC"
	"IATED, LOGIC IS THE KEY TO SUCCESS!        + JAMMER +  THANX FO"
	"R THE ADDS, THEY CAME IN REAL HANDY!       + PAUL / NBS +  KEEP"
	" ON DOING WHAT YA DO BEST!        + NIRO +  NICE TO HAVE YOU BA"
	"CK FROM DOWN UNDER, HI ALSO TO ROOKY      + PHIL +  GOOD CHATS,"
	" THE IN-CAR CD PLAYER IS NEAT        + NITESH +  I EXPECT A DIS"
	"K FROM YOU SOON, OTHERWISE I WILL TAKE YOUR FAMILY HOSTAGE!    "
	"    + THE SCROLLER +  FOR PISSING ME OFF AND WASTING VALID KEYB"
	"OARD TIME!        POWER HOUSE ALSO SEND RESPECT TO THE FOLLOWIN"
	"G UK CREWS :-       ARENA      SLIPSTREAM       QUANTUM MECHANI"
	"X       ANARCHY       MAGNETIC FIELDS      SHARE AND ENJOY     "
	"  MIRAGE UK       GOLDFIRE       AND OUR FOREIGN NEIGHBOURS :- "
	"      TETRAGON       PHENOMENA       RSI+TRISTAR       SUPERION"
	"S (GREAT MUSIC TOOL GUYS!)       CRYPTOBURNERS       DEXION    "
	"   KEFRENS       REBELS       PARADOX       WARFALCONS       SC"
	"OOPEX       BRAINSTORM       UPFRONT       NORTHSTAR / MAHONEY "
	"+ KAKTUS       BUDBRAIN       CRIONICS       ACME       HORIZON"
	"       FRAXION       ALPHA FLIGHT       RAZOR 1911       NETWOR"
	"K       THE GANG       TRIBE       SHINING       THE SILENTS   "
	"    FLASH PRODUCTIONS       ALCATRAZ       CAVE       AND TO AL"
	"L THE MANY OTHER GREAT PURVEYORS OF CODING.       NOW REMEMBER "
	"YOUR ROLE AS A COMMUNICATOR - MAKE ONE MOVE, AND I WILL BLOW YO"
	"UR HEAD OFF!               IF YOU WANT TO DROP   %POWER HOUSE* "
	"  A LINE, THEN HERE ARE SOME ADDRESSES :-     + PREDATOR! +    "
	"25 MELROSE AVENUE,  SUTTON COLDFIELD,  WEST MIDLANDS,  B73 6NT,"
	"  ENGLAND     (LEGAL STUFF ONLY!)     OR     + COBRA +     31 B"
	"ALDMOORE LAKE ROAD,  ERDINGTON,  BIRMINGHAM,  B23 5PX,  ENGLAND"
	"          UNTIL NEXT TIME PILGRIMS, SO LONG!                   "
	"                          \0"
};

static struct scroller_state *scroll;

static void render_scroll_buffer(struct scroller_state *scr_state) {
	// PROFILE_FUNCTION();
	uint32_t *scroll_dest = buffer + scr_state->dest_offset_y * BUFFER_WIDTH;
	uint8_t *scroll_src = scr_state->buffer;

	size_t base_src_index = (scr_state->char_render_offset - 370) & (SCROLL_BUFFER_WIDTH - 1);
	for(size_t i = 0; i < scr_state->char_height; ++i) {
		scr_state->font->palette[1] = scroll_colors[i];

		for(size_t j = 0; j < BUFFER_WIDTH; ++j) {
			size_t src_index = (base_src_index + j) & (SCROLL_BUFFER_WIDTH - 1);
			uint8_t color_index = scroll_src[src_index];

			scr_state->font->palette[0] = scroll_dest[j];
			scroll_dest[j] = scr_state->font->palette[color_index];
		}
		scroll_dest += BUFFER_WIDTH;
		scroll_src += SCROLL_BUFFER_WIDTH;
	}
}

// [=]===^=[ render_copperbar ]============================================================^===[=]

static uint32_t copperbar_colors[10] = {
	0x440055ff, 0x661177ff, 0x884499ff, 0xaa77bbff, 0xccaaddff, 0xffffffff, 0x99dd99ff, 0x44cc44ff, 0x00aa00ff, 0x000000ff,
};

static void render_copperbar(void) {
	// PROFILE_FUNCTION();
	uint32_t color;
	uint32_t *src = copperbar_colors;
	uint32_t *dst = buffer + (0xe4-36) * BUFFER_WIDTH;

	for(size_t y = 0; y < ARRAYSIZE(copperbar_colors); ++y, dst += BUFFER_WIDTH) {
		color = src[y];
		for(size_t x = 0; x < BUFFER_WIDTH; ++x) {
			dst[x] = color;
		}
	}
}

// [=]===^=[ initialize_stars ]============================================================^===[=]
struct star_pos {
	int32_t x;
	int32_t y;
};

static struct star_pos large_star_positions[19];
static struct star_pos small_star_positions[17];

static void initialize_stars(void) {
	init_rng_seed(33897);

	int32_t y = (0x74 - 36);
	for(size_t i = 0; i < 19; ++i) {
		large_star_positions[i].x = rng_next() & 0x1ff;
		large_star_positions[i].y = y;
		y += 6;
	}

	y = (0x76 - 36);
	for(size_t i = 0; i < 17; ++i) {
		small_star_positions[i].x = rng_next() & 0x1ff;
		small_star_positions[i].y = y;
		y += 4;
	}
}

// [=]===^=[ render_stars ]============================================================^===[=]
static void render_stars(void) {
	// PROFILE_FUNCTION();
	for(size_t i = 0; i < 19; ++i) {
		// PROFILE_NAMED("Large_star");
		int32_t temp_x = large_star_positions[i].x;
		blit_full(large_star, temp_x, large_star_positions[i].y, 0);
		temp_x = (temp_x > 424) ? temp_x - 512 : temp_x + 6;
		large_star_positions[i].x = temp_x;
	}

	for(size_t i = 0; i < 17; ++i) {
		// PROFILE_NAMED("Small_star");
		int32_t temp_x = small_star_positions[i].x;
		blit_full(small_star, temp_x, small_star_positions[i].y, 0);
		temp_x = (temp_x > 424) ? temp_x - 512 : temp_x + 4;
		small_star_positions[i].x = temp_x;
	}
}

// [=]===^=[ render_logo ]============================================================^===[=]

static int32_t y_offset;	// NOTE(peter): y-offset for the logo 'reveal' function, initialized further down

static void render_logo(void) {
	// PROFILE_FUNCTION();
	struct rect full_rect = { 0, 0, BUFFER_WIDTH, (0x2d-36) + powerhouse_logo->height };
	blit_clipped(powerhouse_logo, (BUFFER_WIDTH - powerhouse_logo->width) / 2, (0x2d-36) + y_offset, full_rect, 0);
	y_offset = (y_offset > 0) ? y_offset - 1 : 0;
}

// [=]===^=[ render_trees ]============================================================^===[=]

static uint32_t dark_tree_palette[] = {
	0x000000ff, 0xaa7766ff, 0x330000ff, 0x661100ff, 0x993300ff, 0xcc5511ff, 0x004400ff, 0x002200ff,
	0x111111ff, 0x333333ff, 0x555555ff, 0x006600ff, 0x000000ff, 0x000055ff, 0x333388ff, 0x8855ccff,
};

static const uint8_t water_displacement[] = {
	0x0, 0x0, 0x0, 0x1, 0x0, 0x1, 0x0, 0x2, 0x0, 0x2, 0x0, 0x3, 0x0, 0x3, 0x0, 0x4,
	0x0, 0x4, 0x0, 0x4, 0x0, 0x5, 0x0, 0x5, 0x0, 0x5, 0x0, 0x4, 0x0, 0x4, 0x0, 0x4,
	0x0, 0x3, 0x0, 0x3, 0x0, 0x2, 0x0, 0x2, 0x0, 0x1, 0x0, 0x1, 0x0, 0x0, 0x0, 0x1,
	0x0, 0x0, 0x0, 0x1, 0x0, 0x1, 0x0, 0x2, 0x0, 0x2, 0x0, 0x3, 0x0, 0x3, 0x0, 0x3,
	0x0, 0x3, 0x0, 0x2, 0x0, 0x2, 0x0, 0x1, 0x0, 0x1, 0x0, 0x2, 0x0, 0x2, 0x0, 0x3,
	0x0, 0x3, 0x0, 0x3, 0x0, 0x4, 0x0, 0x4, 0x0, 0x4, 0x0, 0x4, 0x0, 0x3, 0x0, 0x3,
	0x0, 0x3, 0x0, 0x2, 0x0, 0x2, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x2,
	0x0, 0x3, 0x0, 0x3, 0x0, 0x3, 0x0, 0x2, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0x0, 0x1, 0x0, 0x2, 0x0, 0x2, 0x0, 0x2, 0x0, 0x3, 0x0, 0x3, 0x0, 0x3, 0x0, 0x3,
	0x0, 0x2, 0x0, 0x2, 0x0, 0x2, 0x0, 0x1, 0x0, 0x1, 0x0, 0x0, 0x0, 0x1, 0x0, 0x2,
	0x0, 0x3, 0x0, 0x2, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x2, 0x0, 0x1,
	0x0, 0x2, 0x0, 0x1, 0x0, 0x1, 0x0, 0x2, 0x0, 0x2, 0x0, 0x3, 0x0, 0x4, 0x0, 0x4,
	0x0, 0x3, 0x0, 0x2, 0x0, 0x2, 0x0, 0x1, 0x0, 0x2, 0x0, 0x1, 0x0, 0x2, 0x0, 0x3,
	0x0, 0x2, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x2, 0x0, 0x3, 0x0, 0x2,
	0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0
};


static void render_trees(void) {
	// PROFILE_FUNCTION();

	struct ugg * restrict trees_data = (struct ugg*)powerhouse_trees_data;

	uint32_t * restrict palette = powerhouse_trees->palette;
	uint32_t * restrict dst = buffer + (128*BUFFER_WIDTH);
	uint8_t * restrict src = powerhouse_trees->data + 20;

	for(size_t y = 0; y < powerhouse_trees->height - 1; ++y) {
		for(size_t x = 0; x < BUFFER_WIDTH; ++x) {

			uint8_t color_id = src[x];
			palette[0] = dst[x];
			dst[x] = palette[color_id];
		}
		dst += BUFFER_WIDTH;
		src += powerhouse_trees->width;
	}

	src -= powerhouse_trees->width;	// NOTE(peter): make sure we are back at the last line!
	dst -= BUFFER_WIDTH;

	static uint32_t sine_offset = 0;

	uint32_t temp_sine_offset = sine_offset;
	sine_offset = (sine_offset + 1 == sizeof(water_displacement)) ? 0 : sine_offset + 1;

	palette = dark_tree_palette;

	size_t count = (powerhouse_trees->height >> 1) - 4;	// NOTE(peter): 4 is just measured, this render the first 4 lines of the upside down trees because that is what the original demo did.

	for(size_t y = 0; y < 4; ++y) {								// NOTE(peter): Actually render those 4 lines.
		for(size_t x = 0; x < BUFFER_WIDTH; ++x) {

			uint8_t color_id = src[x];
			palette[0] = dst[x];
			dst[x] = palette[color_id];
		}
		dst += BUFFER_WIDTH;
		src -= powerhouse_trees->width << 1;
	}

	for(size_t y = 0; y < count; ++y) {
		uint32_t water_offset = water_displacement[temp_sine_offset];
		for(size_t x = 0; x < BUFFER_WIDTH; ++x) {

			uint8_t color_id = src[x + water_offset];
			palette[0] = dst[x];
			dst[x] = palette[color_id];
		}
		dst += BUFFER_WIDTH;
		src -= powerhouse_trees->width << 1;
		temp_sine_offset = (temp_sine_offset + 1 == sizeof(water_displacement)) ? 0 : temp_sine_offset + 1;
	}
}

static void remake_options(struct options *opt) {
	opt->release_group = "POWER HOUSE";
	opt->release_title = "NIGHTFALL";
	opt->window_title = "Power House - Nightfall - 1991";
}

// [=]===^=[ remake_init ]============================================================^===[=]
static void remake_init(struct mkfw_state *window) {
	// int mod_size = _2d6_end - _2d6_data;
	// micromod_initialise(&ctx, (signed char*)_2d6_data, 48000);
	// micromod_set_gain(&ctx, 64);
	// micromod_set_position(&ctx, 0);
	fc14_initialize(&remake_song, (const uint8_t*)chambers_music_data, chambers_music_end - chambers_music_data, 48000);

	initialize_stars();
	powerhouse_logo->height -= 9;
	scroll = scroller_new(8, 8,  0x76 - 36, 2, scrolltext, powerhouse_font, 0, 0);
	y_offset = powerhouse_logo->height;

	mkfw_audio_callback = remake_audio_callback;
}

// [=]===^=[ remake_frame ]============================================================^===[=]
static void remake_frame(struct mkfw_state *window) {
	// PROFILE_FUNCTION();
	render_logo();
	render_stars();
	render_copperbar();
	render_trees();
	scroller(scroll);
	render_scroll_buffer(scroll);
}

// [=]===^=[ remake_shutdown ]============================================================^===[=]
static void remake_shutdown(struct mkfw_state *window) {
	mkfw_audio_callback = 0;
	scroller_remove(scroll);
	fc14_shutdown(&remake_song);
}


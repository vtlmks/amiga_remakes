// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

// [=]===^=[ base setup ]============================================================^===[=]

#define BUFFER_WIDTH  (346 << 0)
#define BUFFER_HEIGHT (270 << 0)

#define AUDIO_STEREO_MIX
#include "platform.c"

// [=]===^=[ remake stuff below ]============================================================^===[=]

#include "sprite_anim.h"

INCBIN_UGG(bob, "data/alcatraz_bobs.ugg");
INCBIN_UGG(logo, "data/alcatraz_logo.ugg");
INCBIN_UGG(font, "data/alcatraz_font.ugg");

INCBIN_BYTES(metalwar, "data/metalwar10.mod");

#define BOB_INITIAL_X 335
#define BOB_INITIAL_Y 93
#define END_OF_RUN 0x80

#define LOGO_INNER_WIDTH 314
#define LOGO_RIGHT_BORDER_START 330
#define NUM_STARS 198

struct bob_pos {
	size_t offset;
	int32_t x;
	int32_t y;
};

struct star_scroller_state {
	uint8_t timer;
	uint16_t table_index;
	uint16_t pattern_seed;
	uint16_t pattern_count;
};

struct star {
	int16_t x;
	int16_t y;
	uint16_t speed;
};

static struct micromod_state metalwar_song;

static uint8_t star_graphics[16];

static struct star stars[NUM_STARS];
static struct star_scroller_state star_state;

static uint16_t star_mask = 0x896f;
static uint16_t star_shift_pattern = 0x8000;
static uint16_t star_bitmap_accumulator = 0x8000;
static uint8_t star_pattern_timer = 120;

static struct scroller_state scroller1;
static struct scroller_state scroller2;

static struct rect alcatraz_screen = { 13 , 7, 320, 270-13 };
static struct bob_pos bobs[6];


static uint16_t pattern_table[] = {
	1, 11, 2, 10, 3, 9, 4, 8, 5, 7, 6, 7, 5, 8, 4, 9, 3, 10, 2, 11, 1, 12,
	0	// Terminator
};

static uint16_t star_speeds_init[NUM_STARS] = {
	3, 1, 1, 1, 3, 2, 1, 3,
	1, 1, 1, 2, 1, 3, 3, 3,
	1, 1, 1, 1, 1, 2, 2, 1,
	2, 1, 1, 1, 1, 2, 2, 1,
	1, 2, 3, 2, 3, 3, 1, 2,
	1, 3, 1, 2, 1, 2, 1, 1,
	3, 3, 1, 2, 3, 2, 2, 1,
	2, 1, 2, 2, 1, 1, 2, 2,
	2, 1, 1, 2, 2, 2, 1, 1,
	1, 3, 1, 1, 1, 1, 3, 1,
	1, 1, 1, 1, 1, 2, 1, 3,
	3, 1, 1, 2, 2, 1, 1, 2,
	1, 2, 3, 3, 3, 1, 3, 2,
	2, 1, 2, 1, 1, 3, 3, 1,
	2, 1, 1, 2, 2, 1, 2, 1,
	1, 1, 1, 1, 1, 3, 1, 3,
	1, 2, 3, 2, 1, 2, 1, 2,
	1, 1, 1, 1, 3, 3, 1, 1,
	2, 2, 1, 1, 1, 1, 2, 3,
	1, 2, 1, 1, 1, 1, 2, 2,
	3, 1, 1, 3, 1, 2, 1, 1,
	1, 1, 1, 1, 1, 3, 3, 1,
	1, 3, 1, 1, 2, 2, 1, 2,
	1, 1, 1, 2, 3, 1, 1, 1,
	2, 1, 1, 1, 3, 1,
};

static uint32_t darker_font_palette[] = {
	0x00000000, 0x000022ff, 0x111144ff, 0x333355ff, 0x444466ff, 0x666688ff, 0x888899ff, 0xaaaabbff,
};

// 19c
static uint32_t color_roll1[] = {
	0x110011ff, 0x220022ff, 0x331133ff, 0x442244ff, 0x553355ff, 0x664466ff, 0x775577ff, 0x886688ff,
	0x997799ff, 0xaa88aaff, 0xbb99bbff, 0xccaaccff, 0xddbbddff, 0xddbbddff, 0xeecceeff, 0xeeddeeff,
	0xffddffff, 0xffddffff, 0xeeddeeff, 0xeecceeff, 0xddccddff, 0xddbbddff, 0xccaaccff, 0xbb99bbff,
	0xaa88aaff, 0x997799ff, 0x886688ff, 0x775577ff, 0x664466ff, 0x553355ff, 0x442244ff, 0x331133ff,
	0x220022ff, 0x110011ff, 0x000000ff, 0x0000000
};

// 19e
static uint32_t color_roll2[] = {
	0x111111ff, 0x222222ff, 0x333333ff, 0x444444ff, 0x555555ff, 0x666666ff, 0x777777ff, 0x888888ff,
	0x999999ff, 0xaaaaaaff, 0xbbbbbbff, 0xccccccff, 0xddddddff, 0xddddddff, 0xeeeeeeff, 0xeeeeeeff,
	0xffffffff, 0xffffffff, 0xeeeeeeff, 0xeeeeeeff, 0xddddddff, 0xddddddff, 0xccccccff, 0xbbbbbbff,
	0xaaaaaaff, 0x999999ff, 0x888888ff, 0x777777ff, 0x666666ff, 0x555555ff, 0x444444ff, 0x333333ff,
	0x222222ff, 0x111111ff, 0x000000ff, 0x00000000
};

static size_t color_roll_index;

static uint8_t scroll_text1[] = {
	"                    ALCATRAZ   THE CHOSEN ONE IN SWITZERLAND   "
	"    PRESENTS  FOR  YOU  ....  AUTOKICK  1.7  ....       THANKS "
	"TO ROBBY FOR THE FAST TRANSMISSION .  BANANA CRACKED BY HELIX O"
	"F ALCATRAZ .      FOR CONTACT WITH US WRITE TO THE HEADQUARTERS"
	" ...       ALCATRAZ    PO.BOX 7    CH-3807 ISELTWALD   (SWITZER"
	"LAND)              OR  OUR  SECOND  PO.BOX   ALCATRAZ    PO.BOX"
	" 1     CH-1262 EYSINS   (SWITZERLAND)         ALCATRAZ  INTERNA"
	"TIONAL  DEMO AND INTRO  MEETING  I988 OR I98B !  STARTED ON DEC"
	"EMBRE OR IN THE FIRST WEEK ON JANUARY .........    SORRY FREAKS"
	" BUT WE MUST MAKE A NEW DATE FOR THE DIM 88 , BECAUSE  ON THIS "
	"WEEKEND IN OCTOBER (14-15-16) IS THE PC-SHOW IN THE LONDONER HA"
	"MMERSMITH .  FIVE  PROGRAMMERS  ' MUST '  GO TO THI"
	"S PC-SHOW , BUT THIS FIVE PROGRAMMERS ARE THE ( HEADLINER'S"
	" ) FOR THE DIM 88 AND THE ALCATRAZ COPY-PARTY ... NOW WE MUST M"
	"AKE A NEW DATE FOR DECEMBRE OR JANUARY ... THE INVITATIONS WE W"
	"ANT SEND TO ALL OUR CONTACTS ,  OTHER FREAKS WHO LIKE THE AMIGA"
	" , FOR A INVITATION TO THE COPYPARTY  ....  NO PROBLEMS , IF YO"
	"U WANT COMMING ON THE DIM YOU MUST SEND US A  PROUDLY TO OUR GR"
	"OUP OR TO YOU (SELFMADE) , AFTHER WE CHECK THE PRODULY  AND IS "
	"IT OK , THEN WE WANT SEND TO YOU A INVITATION FOR  INTERNATIONA"
	"L DEMO INTRO MEETING - DIM .... ( SEND US A LETTER ! )  OK , NO"
	"W WE SHIP GREETINGS        OVERSEA GREETINGS GO TO :         AM"
	"IGAFORCE   ARTHUR   BOSTON FLYER   CAPT'N CRUNCH   DAVID "
	"  FRANK   HIGHT TECH.   JEFF  JIM B.   JIM T.   KEN W   MAD TUN"
	"E   MADISON   MICROKID   OZ CONNECTION   PHILLIPE   ROBBY   SEA"
	"TTLE AMIGA GROUP   SHADOW   TIM B.   UCS AUSTRALIA   YOUNGBLOOD"
	"           EUROPEANS SPECIAL GREETINGS TO :                    "
	"                 WORLD OF WONDERS      ACKERLIGHT       MEGAFOR"
	"CE       RAGE AND DNS     THRUST  ( A NICE OTTIFANTEN DEMO IS G"
	"REAT !!! )      DEFJAM     DR.MABUSE AND DOC     QUARTEX      T"
	"HE MOMORIAL MORDIL HOTLINE     JUNGLE COMMAND     RASTA CATCHA "
	"    NINJAFORCE     OKS IMPORT DIVISION     RDAP      DOMINATORS"
	"      MAGNUM FORCE      RAM HUNTER AND SILICON LEAGUE      TRIL"
	"OGY    NEMESIS     ..........      GREETINGS GO TO  :          "
	"       1001 CREW   4 TH DIMENSION   7 UP CREW   AAA   ABAKUS   "
	"ABC   ABC KILLER   ADMIRAL FUTURE   ALPHAFLIGHT   AMIGA ACTION "
	"  AMIGA INDUSTRIES   ANC EUROPE   APG   APT   ANTITRAX   AXXESS"
	"   BAMIGA SECTOR ONE   BATEMAN   BCS   BEAGLEBOYS   BELGALIVE  "
	"BFBS (HI GUYS !!! STILL ALIVE ! PLEASE WRITE TO MY ! )   BIG AL"
	"   BIG BEN AND COSMOS   BITKILLERSOFT (WRITE AGAIN!)   BLIZZARD"
	" (CONTACT US)   CASCADE   CATLON CREW (HI DRAGON !)   CBB   CBC"
	" AND THE BAND   CELTICS   COMPUTER BRAIN SERVICE   CRM CREW   D"
	"ANISH GOLD   DEATHLINE   DEATHSTAR   DELTA FORCE   DEXION   DIG"
	"I TECH   DOMINATORS   DR SOFT   ECE CREW   ELECTION   ERROR SOF"
	"T   F4CG   FAIRLIGHT   FINISH GOLD   FIRM   FREAK FACTORY   FRE"
	"E ACCESS   FREE NETWORK   GCS   GIGAFLOPS   GIX GROUP   GUANABA"
	"TS   GOONIES   HQC   IAN AND MIC   ICC   ICS   IDT   IMP 666   "
	"ISS   IMPORT FREAKS   ITALIAN BAD BOYS   ITD   JEWELS   JAZZCAT"
	"   JOCKER CREW    KENWOOD   KINGHT HAWKS ( CONTACT US ! IMPORTA"
	"NT )   LIGHT FORCE   MAGNIFICIENT FORCE    MEMORIAL MORDIL HOTL"
	"INE    NORTHEN LIGHTS   NORTHSTAR    OKS IMPORT DIVISION    OOH"
	"OO-TROOPS   OVER LORD   PATRICK   PCT   PHENOMENA   POWERSLAVES"
	"   PROPHETS AG   PIRANHAS   PHR CREW    PLASMA FORCE    PROFESS"
	"IONAL OF DNS (QUEST)   QUACKERS   QUALITE    QUARAM   QUEST  RA"
	"DWARR    RAWHEAD    RAZOR   RATS COMPANY   RISKY BUISINESS BOYS"
	"   RED SECTOR GERMANY   RTS OF HCC   ROTTEN   SCA   SCAIP   SDI"
	"   SHARKS   SPRITE   SSI   STACK ( HI ICEMEAN !)   STELL PULSE "
	"  STELLER AND BUG   SOFTBUSTERS   SUNRAIDERS   SUPREMACY   SYST"
	"EM Z    TDRS   TAC   TAURUS ONE   TETRAGON   TEW AND NATO   THE"
	" ACCUMULATORS   THE AMIGA GANG   THE DOMINATORS   THE EXTREMINA"
	"TOR   THE FINAL SUPPLIER   THE FIRM   THE LIGHT FORCE   THE MAL"
	"TESE HACKERS   THE NEW MASTERS   PRO'S OF DNS   THE SOFTK"
	"ILLER CREW   THE THREE MUSKETEERS   THE WEB INST.   THE WIZZBRI"
	"NGERS   THE ZAPP   THORIUM   THRUST   THUNDERBOLD CRACKING CREW"
	"   TRISTAR (WHO)   TSB   TSB   TYTAN   UMS   UNKNOWN FIVE   UNK"
	"NOWN OF DOC   VISION   VISION FACTORY   WILD COPPER   WIZZARDS "
	"  W.O.D.   X-RAYZ   YOUNG ONES   ZEKE WOLF  ...   AND   LAST  B"
	"UT  LAST  A  HI  TO     GNOM , YUMASOFT AND FIRESTONE       FOR"
	" CONTACT WITH THE CHOSEN ONE IN SWITZERLAND  ....... HAVE YOU A"
	" PAPER  ...   OK !  5     4    3    3,5    7   5 2 33245510    "
	" SHIT ! OK , A NEW ONE  ...        5       4       3       2   "
	"   1        WRITE TO THE THE HEADQUARTER CENTER FOR EXCHANGE OR"
	" BUY  THE LATEST NEWEST SOFTWARE    OR FOR INVITATIONS  TO  COP"
	"YPARTY - DIM 88 OR 8B                    WRITE TO :   ALCATRAZ "
	"        P.0.Box  7          CH-3807 ISELTWLAD         SWITZERLA"
	"ND            ......     ALCATRAZ     P.O.BOX  7     CH-3807  I"
	"SELTWLAD   (SWISS)          end of message                     "
	"        "
};
static uint8_t scroll_text2[] = {
	"                       ALCATRAZ WELCOMES           SWACO (CRACK"
	"ER,PROGRAMMER)       INSIDER (FONT DESIGNER,SPREADER)       IRO"
	"NHAWK (GRAPHIC DESIGNER,SPREADER)       TERMINATORS (SPREADERS)"
	"       CAPTAIN CRACK (IN FRANCE,PROGRAMMER AND CHIEF OF THE ALC"
	"ATRAZ-NEWSPAPER , IN FRENCH OF COURSE !!! ...)                 "
	"    MEMBERS ARE                HELIX           METALWAR        "
	"   PGCS           CAPTAIN CRACK           IRONHAWK           IN"
	"SIDER           MAO           SWACO           BIT           TER"
	"MINATORS DUO           CKG ...                  CODE AND MUSIC "
	"BY METALWAR    TITLE BY IRONHAWK  AND    FONT BY INSIDER       "
	"                   WRITE TO    ALCATRAZ     P.O.BOX 7    CH - 3"
	"807  ISELTWALD   (SWITZERLAND)        "
};

#define FONT_ROW_0 0
#define FONT_ROW_1 10240
#define FONT_ROW_2 20480
#define FONT_ROW_3 30720
#define FONT_ROW_4 40960

struct font_char_info font_char_infos[128] = {
	['A'] = {.offset = FONT_ROW_0 + 0,   .width = 32},
	['B'] = {.offset = FONT_ROW_0 + 32,  .width = 32},
	['C'] = {.offset = FONT_ROW_0 + 64,  .width = 32},
	['D'] = {.offset = FONT_ROW_0 + 96,  .width = 32},
	['E'] = {.offset = FONT_ROW_0 + 128, .width = 32},
	['F'] = {.offset = FONT_ROW_0 + 160, .width = 32},
	['G'] = {.offset = FONT_ROW_0 + 192, .width = 32},
	['H'] = {.offset = FONT_ROW_0 + 224, .width = 32},
	['I'] = {.offset = FONT_ROW_0 + 256, .width = 32},
	['J'] = {.offset = FONT_ROW_0 + 288, .width = 32},

	['K'] = {.offset = FONT_ROW_1 + 0,   .width = 32},
	['L'] = {.offset = FONT_ROW_1 + 32,  .width = 32},
	['M'] = {.offset = FONT_ROW_1 + 64,  .width = 32},
	['N'] = {.offset = FONT_ROW_1 + 96,  .width = 32},
	['O'] = {.offset = FONT_ROW_1 + 128, .width = 32},
	['P'] = {.offset = FONT_ROW_1 + 160, .width = 32},
	['Q'] = {.offset = FONT_ROW_1 + 192, .width = 32},
	['R'] = {.offset = FONT_ROW_1 + 224, .width = 32},
	['S'] = {.offset = FONT_ROW_1 + 256, .width = 32},
	['T'] = {.offset = FONT_ROW_1 + 288, .width = 32},

	['U'] = {.offset = FONT_ROW_2 + 0,   .width = 32},
	['V'] = {.offset = FONT_ROW_2 + 32,  .width = 32},
	['W'] = {.offset = FONT_ROW_2 + 64,  .width = 32},
	['X'] = {.offset = FONT_ROW_2 + 96,  .width = 32},
	['Y'] = {.offset = FONT_ROW_2 + 128, .width = 32},
	['Z'] = {.offset = FONT_ROW_2 + 160, .width = 32},
	['*'] = {.offset = FONT_ROW_2 + 192, .width = 32},
	[' '] = {.offset = FONT_ROW_2 + 224, .width = 32},
	['.'] = {.offset = FONT_ROW_2 + 256, .width = 32},
	['!'] = {.offset = FONT_ROW_2 + 288, .width = 32},

	['-'] = {.offset = FONT_ROW_3 + 0,   .width = 32},
	['0'] = {.offset = FONT_ROW_3 + 32,  .width = 32},
	['1'] = {.offset = FONT_ROW_3 + 64,  .width = 32},
	['2'] = {.offset = FONT_ROW_3 + 96,  .width = 32},
	['3'] = {.offset = FONT_ROW_3 + 128, .width = 32},
	['4'] = {.offset = FONT_ROW_3 + 160, .width = 32},
	['5'] = {.offset = FONT_ROW_3 + 192, .width = 32},
	['6'] = {.offset = FONT_ROW_3 + 224, .width = 32},
	['7'] = {.offset = FONT_ROW_3 + 256, .width = 32},
	['8'] = {.offset = FONT_ROW_3 + 288, .width = 32},

	['9'] = {.offset = FONT_ROW_4 + 0,   .width = 32},
	['['] = {.offset = FONT_ROW_4 + 32,  .width = 32},
	[']'] = {.offset = FONT_ROW_4 + 64,  .width = 32},
	['"'] = {.offset = FONT_ROW_4 + 96,  .width = 32},
	['$'] = {.offset = FONT_ROW_4 + 128, .width = 32},
	[','] = {.offset = FONT_ROW_4 + 160, .width = 32},
};

// [=]===^=[ audio_callback ]============================================================^===[=]
static void remake_audio_callback(int16_t *data, size_t frames) {
	micromod_get_audio(&metalwar_song, data, frames);
}

static void remake_options(struct platform_state *state) {
	state->release_group = "Alcatraz";
	state->release_title = "Autokick 1.7";
	state->window_title = "Alcatraz - Autokick 1.7 - 1988-09\0";
}

static void render_logo(struct platform_state *state) {
	uint32_t *dst = BUFFER_PTR(state, CENTER_X(state, logo->width), 7);
	uint8_t *src = logo->data;

	for(size_t y = 0; y < logo->height; ++y, src += logo->width, dst += state->buffer_width) {

		if(y >= 153 && y <= 185) {
			logo->palette[14] = color_roll1[((y-153) + color_roll_index) % ARRAYSIZE(color_roll1)];
			logo->palette[15] = color_roll2[((y-153) + color_roll_index) % ARRAYSIZE(color_roll2)];
		}

		for(size_t x = 0; x < logo->width; ++x) {
			uint8_t c = src[x];
			logo->palette[0] = dst[x];
			dst[x] = logo->palette[c];
		}
	}

	color_roll_index++;
}

static void update_star_pattern(void) {
	if(star_shift_pattern != 0) {
		star_pattern_timer--;
		if(star_pattern_timer == 0) {
			star_pattern_timer = 120;
			star_shift_pattern >>= 1;
			uint16_t bit = star_shift_pattern & star_mask;
			star_bitmap_accumulator |= bit;

			uint16_t a = star_bitmap_accumulator;
			uint16_t mask = 0x8000;
			for(size_t i = 0; i < 16; ++i) {
			   star_graphics[i] = (a & mask) ? 1 : 0;
			   mask >>= 1;
			}
		}
	}
}

static void update_stars(void) {

	star_state.timer--;
	if(star_state.timer == 0) {
		star_state.timer = 0x32;
		star_state.pattern_seed = pattern_table[star_state.table_index++];

		if(pattern_table[star_state.table_index] == 0) {
			star_state.table_index = 0;
		}
	}

	for(uint32_t i = 0; i < NUM_STARS; i++) {
		stars[i].x += stars[i].speed;

		if(stars[i].x >= 400) {
			stars[i].x -= 448;
			stars[i].speed = star_state.pattern_count;

			star_state.pattern_count--;
			if(star_state.pattern_count == 0) {
				star_state.pattern_count = star_state.pattern_seed;
			}
		}
	}
}

static void init_stars(void) {
	star_state.timer = 0x3d;
	star_state.table_index = 2;
	star_state.pattern_seed = 0xb;
	star_state.pattern_count = 6;

	for(uint32_t i = 0; i < NUM_STARS; i++) {
		stars[i].x = 224;
		stars[i].y = i + 8;
		stars[i].speed = star_speeds_init[i];
	}

	for(uint32_t i = 0; i < 500; i++) {
		update_stars();
	}
}

static void render_stars(struct platform_state *state) {
   int32_t left_border = LOGO_RIGHT_BORDER_START - LOGO_INNER_WIDTH;
   int32_t right_border = LOGO_RIGHT_BORDER_START;

   for(size_t i = 0; i < NUM_STARS; ++i) {
      int32_t x = stars[i].x;
      int32_t y = stars[i].y;

      uint32_t c = 0x11111100 * (stars[i].speed + 3) | 0xff;

		uint32_t *dst = BUFFER_PTR(state, x, y);
      for(int32_t j = 0; j < 16; ++j) {
         int32_t pixel_x = x + j;

         if(pixel_x < left_border || pixel_x >= right_border) continue;

         dst[j] = star_graphics[j] ? c : dst[j];
      }
   }
}

// [=]===^=[ remake_init ]============================================================^===[=]
static void remake_init(struct platform_state *state) {
	platform_change_resolution(state, BUFFER_WIDTH, BUFFER_HEIGHT);

	init_stars();
	for(size_t i = 0; i < 6; ++i) {
		bobs[i].x = BOB_INITIAL_X;
		bobs[i].y = BOB_INITIAL_Y;
		bobs[i].offset = i * 10;
	}

	scroller1 = (struct scroller_state) {
		.char_width = 32,
		.char_height = 32,
		.dest_offset_y = 209,
		.speed = 4,
		.text = scroll_text1,
		.font = font,
		.char_info = font_char_infos,
		.palette = font->palette,
	};
	scroller2 = (struct scroller_state) {
		.char_width = 32,
		.char_height = 32,
		.dest_offset_y = 233,
		.speed = 2,
		.text = scroll_text2,
		.font = font,
		.char_info = font_char_infos,
		.palette = darker_font_palette,
	};

	scroller_new(&scroller1);
	scroller_new(&scroller2);

	micromod_initialize(&metalwar_song, (int8_t*)metalwar, 48000);
	mkfw_set_audio_callback(remake_audio_callback);
}

// [=]===^=[ remake_frame ]============================================================^===[=]
static void remake_frame(struct platform_state *state) {
	platform_clear_buffer(state);

	update_stars();
	update_star_pattern();
	render_stars(state);

	for(size_t i = 0; i < 6; ++i) {
		blit_full_src(state, bob, bobs[i].x, bobs[i].y, alcatraz_screen, 0);
		bobs[i].x += (int32_t)sprite_anim[bobs[i].offset++];
		bobs[i].y += (int32_t)sprite_anim[bobs[i].offset++];

		if((uint8_t)sprite_anim[bobs[i].offset] == END_OF_RUN) {
			bobs[i].x = BOB_INITIAL_X;
			bobs[i].y = BOB_INITIAL_Y;
			bobs[i].offset = 0;
		}
	}

	render_logo(state);

	scroller_update(state, &scroller2);
	scroller_update(state, &scroller1);
}

// [=]===^=[ remake_shutdown ]============================================================^===[=]
static void remake_shutdown(struct platform_state *state) {
	mkfw_set_audio_callback(0);
}

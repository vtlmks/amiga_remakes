// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

/*
 *
 */

INCBIN_UGG(part8_end_logo_1_data, "data/p8_the_end_logo_1.ugg");
INCBIN_UGG(part8_end_logo_2_data, "data/p8_the_end_logo_2.ugg");
INCBIN_UGG(part8_end_logo_3_data, "data/p8_the_end_logo_3.ugg");
INCBIN_UGG(part8_end_logo_4_data, "data/p8_the_end_logo_4.ugg");
INCBIN_UGG(part8_end_logo_5_data, "data/p8_the_end_logo_5.ugg");
INCBIN_UGG(part8_end_logo_6_data, "data/p8_the_end_logo_6.ugg");
INCBIN_UGG(part8_end_logo_7_data, "data/p8_the_end_logo_7.ugg");
INCBIN_UGG(part8_end_logo_8_data, "data/p8_the_end_logo_8.ugg");
INCBIN_UGG(part8_end_logo_9_data, "data/p8_the_end_logo_9.ugg");


static uint32_t p8_the_end_animation_index = 0;

static uint32_t p8_scroller_inner_colors[] = {
	0xeeee88ff, 0xeecc66ff, 0xdd9933ff, 0xcc7722ff, 0xbb5500ff, 0xaa4400ff, 0x000000ff, 0x000000ff,
	0x000000ff, 0xaa4400ff, 0xbb5500ff, 0xcc7722ff, 0xdd9933ff, 0xeecc66ff, 0xeeee88ff, 0xffffeeff, 0xffffeeff,
};

static uint32_t p8_scroller_border_colors[] = {
	0xaa4400ff, 0xbb5500ff, 0xcc7722ff, 0xdd9933ff, 0xeecc66ff, 0xeeee88ff, 0xffffeeff, 0xffffeeff,
	0xeeee88ff, 0xeecc66ff, 0xdd9933ff, 0xcc7722ff, 0xbb5500ff, 0xaa4400ff, 0x000000ff, 0x000000ff
};

static uint8_t p8_real_inner_color_index;
static uint8_t p8_real_border_color_index;

static uint8_t p8_scroll_text_1[] = {
	"WELL, EVERYTHING HAS ITS END. HOPE YOU HAVE ENJOYED THE DEMOS. "
	"THIS LITTLE PIECE OF CODING WAS DONE BY ATOM 1988-03-31. IN THIS SCROLL WIL"
	"L I TELL YOU A LITTLE ABOUT NORTH STAR.    NORTH STAR WAS FOUNDED "
	"1987 BY ME(ATOM) AND STARFIRE. WE HAVE NOW EXPANDED A BIT, FROM"
	"2 MEMBERS TO 14. UFO WAS KICKED OUT DURING THE TIME WE MAKED TH"
	"IS DEMO DISK SO IN SOME DEMOS HE'S A MEMBER OF NORTH STAR, BUT HE ISN'T "
	"ANYMORE !!!   THE MEMBERS ARE: ATOM * STARFIRE * HEDDA * TITAN * "
	"RILLE * REX * LIBERATOR * FOETUS * CRAB * MC * ZOETROPE * JAZZE * "
	"ELECTRO AND GIZMO . . . . . WE ARE LOOKING FOR FOREING MEMBERS "
	"IF YOU ARE THE RIGHT ONE, MAIL ME AT: CHRISTOFFER NILSSON, JULIG.7, "
	"252 52 HELSINGBORG, SWEDEN . . . . BACK TO NORTH STAR, WE ARE "
	"ANIT-NAZISTS AND ANIT-COMMUNISTS . . . . WE HOPE TO BE THE BIGGEST "
	"AMIGA GROUP IN SWEDEN ONE DAY !!!  HOPE YOU HAVEN'T GOT A PROBLEM WITH "
	"READING ALL THE SCROLLYS AT THE SAME TIME !!! OK, LETS TALK ABOUT "
	"THE MEMBERS AND WHAT THEY DO !!!   ATOM *** GOES IN HIGH-SCHOOL, "
	"17(SOON!) YEARS OLD, MAKES GRAPHIX TO COMMERCIALL GAMES, "
	"MAKES THESE DEMOS WHEN THERE',$27,'S SOME TIME LEFT (SCHOOL TIME THAT IS !!!) "
	", FAVOURITE MUSIC: SYNTH MUZAX AND AMIGA TUNES !!! "
	"           STARFIRE *** WORKS (SOMETIMES!), 20 YEARS OLD, "
	"MAKES ANIT-VIRUS PROGGYS SOME DEMOS WILL POP UP IN THE FUTURE, "
	"FAVOURITE MUSIC: T',$27,'PAU AND HEAVY !! (UURRK!) "
	"       HEDDA *** GOES IN 9:TH CLASS, 16(SOON!) YEARS OLD, CRACK "
	"SIXTY-FOUR GAMES FOR US AND MAKES 64 DEMOS AS WELL, FAVOURITE MUSIC: "
	"EH ?? PET-SHOP BOYS MAYBE ?!?       TITAN *** GOES IN HIGH-SCHOOL, "
	"17 (OR?) YEARS OLD, MAKES SUPER-COOL TUNES FOR US LIKE THE ONE YOU "
	"ARE LISTENING ON FOR THE MOMENT, FAVOURITE MUSIC: HIS OWN ??!? "
	"OR JARRE !!!       REX *** 23(??) YEARS OLD, IS OUR MEGA-SWAPPER AND "
	"GAME SUPPLIER !!!!       RILLE *** 17(OR?), IS OUR CLOWN AND TELEPHONE TERROR MAN "
	"!!!!       FOETUS *** 24(??) OUR SIXTY-FOUR MEGA-SWAPPER.       LIBERATOR *** 18, "
	"cA HARDWARE FREAK !!       WELL THE REST I DON'T KNOW SO MUCH ABOUT, BUT ALL "
	"NORTH STAR MEMBERS ARE GREAT !!!                           !WARP!  ^"
};

static uint8_t p8_scroll_text_2[] = {
	"  THIS DEMO DISK WAS MADE DURING A PERIOD OF 1.5 MONTHS. "
	"I (ATOM) WOULD LIKE TO THANK SOME PEOPLE WHO HAVE HELPED ME "
	"WITH THIS DEMO DISK SOMEWAY.           CREDITS TO: "
	"   SINE FOR HELPING ME TO CONTROL THIS UNCONTRALABLE(?) MACHINE. "
	"     STARFIRE FOR ASSISTING ME AND ALSO FOR BEEING A VERY GOOD FRIEND. "
	"     TITAN FOR ALL TUNES.    JAZZE FOR ALL SAMPLES AND SOME GRAPHIX. "
	"     REX FOR SPREADING THIS DEMO-DISK.     HQC FOR THE MARVELLOUS "
	"CRUNCHER.      SUNE FOR COPYING ALL THE AMIGA MANUALS TO ME. "
	"     RADIO-ORESUND FOR MAKING SUCH GOOD RADIO PROGRAMS. "
	"     ALL PEOPLE WHO HAVE SEEN MY FIRST DEMO AND WRITTEN TO US. "
	"     THE AMIGA CONSTRUCTORS FOR MAKING SUCH A GREAT MACHINE. "
	"     FOETUS FOR ALL SIXTY FOUR DEMOS.      ANTITRAX "
	"FOR MAKING SUCH SUPER-COOL MEGA-DEMO.     MY LITTLE BROTHER (RIP) "
	"FOR COMPLAINING.      MY DOG FOR BEEING SO COOL !!. "
	"      QWERT FOR NOT DESTROYING MY SOURCE FILES. "
	"      KAKTUS/TRIAD FOR THE ULTIMATE SOUNDTRACKER. "
	"   THANKS TO THE AUTHERS OF DPAINTII, DELUXE CONVERTER, "
	"METACOMPCO ASSEMBLER, SEKA ASSEMBLER, CMON, AMON, DISKCRAFT, THE "
	"ULTIMATE SOUND TRACKER, AUDIOMASTER, MONAM AND GENAM. "
	"    THAT',$27,'S IT !! SORRY IF I',$27,'VE MISSED SOMEONE... "
	"                   !WARP!            ^"
};

static uint8_t p8_scroll_text_3[] = {
	"CONTACT US FOR SWAPPING...... PHONE (SWEDEN) 42-260664 (REX) OR "
	"WRITE TO:  HOGASTENSGATAN.13  ,  252 32 HELSINGBORG  ,  SWEDEN. "
	" OR WRITE TO: MALORTSVAGEN.44  ,  260 40 VIKEN  ,  SWEDEN. "
	"   TO SWAP 64 PROGGYS..... PHONE (SWEDEN) 46-157574 (FOETUS) OR"
	" WRITE TO:  MAGNUS KRISTENSSON  ,  VASTERVIKSGATAN 20  ,  252 51 HELSINGBORG  , "
	" SWEDEN.       IF YOU HAVE ANY COMMENT ABOUT THIS DEMOS OR ANYTHING "
	"ELSE WICH CONCERNS ME, CALL ME AT (SWEDEN) 42-153302 YOU'LL FIND MY "
	"ADDRESS IN THE UPPER SCROLL.       9   8   7   6   5   4   3   2   1 "
	"   0 (!WARP!) ^",
};

static uint8_t p8_scroll_text_4[] = {
	"HERE ARE THE LAST UPDATED GREETINGS LIST.......SPECIAL "
	"HELLOS TO OUR CONTACTS, FRIEND AND FOES : "
	"  SINE, TRIAD, XAKK, VORTEX 42, THE VIKINGS, THE SILENTS, SCIENCE "
	"451, BEASTIE BOYS, TRIUMPH, FAIRLIGHT, POWEREXTREME, THE AMIGA GANG/HOLLAND, "
	"THE AEXENON/BELGIUM, THE PROS/AUSTRIA, THE SHADOWS/NORWAY, "
	"FREE ACCES, NEW FORCES, THE YOUNG CRIMINALS/WEST-GERMANY, "
	"THE DREAM WARRIORS, ACADEMY/NORWAY, THE SQAUD/NORWAY, "
	"NEW ORDER/WEST-GERMANY, DEFIERS, TCT, TAS, ET, THE ARNE BOYS, "
	"COCA COLE KID, MCL, TTW, TEC, BSI, DEP SOFT, XL CRACKERS, MY SOFT, "
	"FIREMAN, WOLFMAN, PPCS, HALLOWEEN.C.S, TETRAGON, PMI, NINJA CRACKING "
	"CREW, LAMF, POS, WARHEAD.C.C, THUNDERCATS, MIRACLE BOYS, SEMI, "
	"AMIGOS, MR NOBODY, WHITE TRASH, CPU/CANADA, S.O.S, NORTHERN LIGHT, "
	"KARATE KID, WACO, HORROR CRACKINGS, PCS, ZEEN OF 666, WARLORDS, "
	"WARLORDS, SAS, TGP... HM? CAN THAT BE ALL OF THEM, I MUST HAVE FORGOTTEN "
	"SOMEBODY, IF THAT',$27,'S THE CASE I',$27,'M SORRY !!! "
	" SORRY FOR THE SLOW REPLY TO ALL THE GROUPS WHO WROTE TO US, BUT "
	"THIS DEMO TOOK LONGER TIME TO MAKE THAN EXPECTED !!! ANYWAY HERE IT IS !!!! "
	"   THIS DEMO WILL BE SENT TO MOST OF OUR CONTACTS AND (TO WICH WE THINK ARE "
	"AMONG THE BEST GROUPS IN THE WORLD, THEY ARE):  THE CHAMPS, ANTITRAX, "
	"THE MOVERS, UNIT A, THE FIRM, TRISTAR, HOTLINE, HQC, TLC AND BLIZZARD "
	"I',$27,'M NOT SURE THATS EVERYONE, CAUSE IT',$27,'S REX WHO WILL SPREAD THE DEMO !!!. "
	"    OK, HOPE YOU ALL HAVE HAD A NICE TIME WATCHING THE DEMOS !!! "
	"    WE MIGHT DO A NEW DEMO DISK IN THE FUTURE !!!!            BYE BYE !!! "
	"      !SNURRE-RUNTARE!            ^"
};

static struct scroller_state *p8_scroll_1;
static struct scroller_state *p8_scroll_2;
static struct scroller_state *p8_scroll_3;
static struct scroller_state *p8_scroll_4;

static uint8_t p8_process_char(struct scroller_state *scr_state, uint8_t scroll_character) {
	if(scroll_character == '^') {		// Scrolltext end
		scr_state->text_offset = 0;
		return scr_state->text[scr_state->text_offset++];
	}
	return scroll_character;
}


static void p8_init() {
	p8_scroll_1 = scroller_new(16, 16,  54, 2, p8_scroll_text_1, part1_small_font_data, p8_process_char, 0);
	p8_scroll_2 = scroller_new(16, 16,  86, 1, p8_scroll_text_2, part1_small_font_data, p8_process_char, 0);
	p8_scroll_3 = scroller_new(16, 16, 182, 1, p8_scroll_text_3, part1_small_font_data, p8_process_char, 0);
	p8_scroll_4 = scroller_new(16, 16, 214, 2, p8_scroll_text_4, part1_small_font_data, p8_process_char, 0);
}

static void p8_shutdown() {
	scroller_remove(p8_scroll_1);
	scroller_remove(p8_scroll_2);
	scroller_remove(p8_scroll_3);
	scroller_remove(p8_scroll_4);
}

static void p8_render_scroll_buffer(struct scroller_state *scr_state) {
	// PROFILE_FUNCTION();
	uint32_t *scroll_dest = buffer + scr_state->dest_offset_y * BUFFER_WIDTH;
	uint8_t *scroll_src = scr_state->buffer;
	uint32_t inner_color_count = ARRAYSIZE(p8_scroller_inner_colors);
	uint32_t border_color_count = ARRAYSIZE(p8_scroller_border_colors);

	uint32_t color_lookup[4];
	color_lookup[2] = 0xffffffff;

	for(size_t i = 0; i < scr_state->char_height; ++i) {
		color_lookup[1] = p8_scroller_inner_colors[(p8_real_inner_color_index + i) % inner_color_count];
		color_lookup[3] = p8_scroller_border_colors[(p8_real_border_color_index + i) % border_color_count];

		for(size_t j = 0; j < BUFFER_WIDTH; ++j) {
			size_t src_index = (scr_state->char_render_offset - 370 + j) & (SCROLL_BUFFER_WIDTH - 1);
			uint8_t color_index = scroll_src[src_index];
			color_lookup[0] = scroll_dest[j];
			scroll_dest[j] = color_lookup[color_index];
		}

		scroll_dest += BUFFER_WIDTH;
		scroll_src += SCROLL_BUFFER_WIDTH;
	}
}

static uint32_t p8_update()  {
	// PROFILE_NAMED("part8 all");

	scroller(p8_scroll_1);
	scroller(p8_scroll_2);
	scroller(p8_scroll_3);
	scroller(p8_scroll_4);
	p8_render_scroll_buffer(p8_scroll_1);
	p8_render_scroll_buffer(p8_scroll_2);
	p8_render_scroll_buffer(p8_scroll_3);
	p8_render_scroll_buffer(p8_scroll_4);
	p8_real_inner_color_index++;
	p8_real_border_color_index--;



	{
		// PROFILE_NAMED("part 8 end logo");
		struct ugg *p8_end_logo_entries[] = {
			part8_end_logo_1_data, part8_end_logo_2_data, part8_end_logo_3_data, part8_end_logo_4_data, part8_end_logo_5_data, part8_end_logo_6_data, part8_end_logo_7_data, part8_end_logo_8_data,
			part8_end_logo_9_data, part8_end_logo_8_data, part8_end_logo_7_data, part8_end_logo_6_data, part8_end_logo_5_data, part8_end_logo_4_data, part8_end_logo_3_data, part8_end_logo_2_data,
		};

	// mid screen THE END logo  144p to middle
		struct ugg *end_logo_entry = (struct ugg*)p8_end_logo_entries[p8_the_end_animation_index];
		uint32_t y_offset = 144 - (end_logo_entry->height >> 1);
		uint32_t x_offset = (BUFFER_WIDTH>>1) - (end_logo_entry->width >> 1);
		blit_full(end_logo_entry, x_offset, y_offset, 0);

		if((state.frame_number & 0x1) == 0) {
			p8_the_end_animation_index++;
			p8_the_end_animation_index %= ARRAYSIZE(p8_end_logo_entries);
		}
	}

	return mkfw_is_button_pressed(window, MOUSE_BUTTON_LEFT);
}




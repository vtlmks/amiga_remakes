/*
 * Part 6: Omega
 */

INCBIN_UGG(part6_large_logo_data, "data/p6_large_logo.ugg");
INCBIN_UGG(part6_scroll_font_data, "data/p6_large_scroll_font.ugg");

static uint8_t p6_scroll_text[] = {
	"             YEEH!!!!!  THIS IS \"#$%&' SPEAKING.... THIS "
	"IS MY FIRST DEMO ON THIS WONDERFUL MASCHINE.........       FIRS"
	"T SOME SPECIAL GREETINGS TO: DRAKE (GREAT MUSIC) AND TO MR JONE"
	"S (LOVE YOUR PICCY!!!). AND NOW OVER TO DRAKE............. HELL"
	"O ALL AMIGA FREAKS!!!  THIS IS DRAKE FROM VORTEX 42... RIGHT NO"
	"W OMEGA IS AT MY P(A)LACE...  FIRST SOME FUCKINGS TO DARKHELMET"
	" AND AXA 2002 .... IF YOU WANT SOME MUSIC FROM ME THEN CALL SWE"
	"DEN-(0)41822190 OR WRITE TO CARSTEN OHLSEN, PILANGSRUNDELN 17B,"
	" 26142 LANDSKRONA, SWEDEN .........    AND NOW OVER TO MR JONES"
	" FOR A LITTLE COMMENT......              YO! FUCK A DUCK... MR "
	"JONES IS HERE (THIS IS THE SECOND TIME I WRITE THIS, BLODDY GUR"
	"U) I HOPE YOU LIKE MY PICTURE. IF YOU WANT GRAPHICS DONE F"
	"OR YOUR DEMOS OR GAMES JUST CALL ME ON THE FOLLOWING NUMBER SWE"
	"DEN-(0)418/192 19 OR WRITE TO ME AT THIS ADRESS: MAGNUS PREDIKA"
	"KA, LOPARGATAN 32B, S-261 45  LANDSKRONA.     MY GREETINGS GOES"
	" TO: OMEGA (FOR ALLOWING ME TO DRAW THIS PICTURE FOR HIM), JAWS"
	", DRAKE AND THE REST OF VORTEX 42, ELECTRONIC POWER, NINE TOMMO"
	"ROW CREW, MR SHARK, MY GIRLFRIEND AND MY CRAZY CAT, MY CD PLAYE"
	"R, MY VIDEO, MY CAR SIGGE, MY AMIGA 1000, MY DISHWASHER (COULDE"
	"NT LIVE WITHOUT YOU), J SACH (YOU ARE THE GREATEST), EDDIE MURP"
	"HY, ZZ-TOP, ORUP, MY SELF, THE MAKERS OF THE AMAZING AMIGA, MAR"
	"CEL, GARFIELD, ALF (JUST KIDDING), BASIL, MANUEL, DR RUTH, EDGA"
	"R, BOND (JAMES BOND), SPACEBALLS, D SILVA, RICK SPRINGFIELD, MY"
	" TOASTER, MAX HEADROOM, THE HULKSTER, DONALD DUCK, KENNY EVERET"
	"T, BENNY HILL, THE HILL ANGELS, PAUL HOGAN, LADY ADA, WWF, BC, "
	"MAD, MORT WALKER, GORDON BESS, PHILIPS, BANG OLOFSEN, ONKYO, FO"
	"SS, STEVEN SPIELBERG, ZX 80, HELEN SPENCE, M 31, STAKEOUT, BRUC"
	"E WILLIS, ET, DUNE, ALIEN, DNA, I WANT YOUR SEX, EURYTHMICS, IS"
	"AAC ASIMOV, A C CLARK, MMMMMMMMMMMMMMMMM MARABOU, ILM, MTV, DP,"
	" ELECTRONIC ARTS, WAGA WAGA, SPEEDY GONZALES, 10010, BUT MOST O"
	"F ALL MY REMOTE CONTROLS.....    PS. GO OUT AND BUY THE GREATES"
	"T SAMPLER IN THE UNIVERSE, THE  OUTLAND SAMPLER!!!!!!!!!!!!!!!!"
	"!!!!!!!!, I WOULD ALSO WISH TO THA AXQO,U.+!/W/Z0.,., PUUHH, AT"
	" LAST I COULD TEAR HIM AWAY FROM  THE KEYBOARD. NOW FOR MY GREE"
	"TINGS: (ALL THOSE WHO ARE MENTIONED ABOVE), COPPERWIZARD, ALL M"
	"EMBERS OF NORTH STAR (ESPECIALLY ATOM FOR HIS HELP) AND TO EVER"
	"YBODY THAT LIKED THIS DEMO ........   (AOOOOOWW     BLOODY CAT "
	"(I HATE HIM)) AND THATS ALL FROM ME FOR THIS TIME. SEE YOU AGAI"
	"N VERY SOON.............       PS.     IF YOU WANT TO CONTACT M"
	"E (OR NORTH STAR) CALL: (ATOM) SWEDEN (0)42-153302  OR WRITE TO"
	": CHRISTOFFER NILSSON, JULIGATAN 7, 252 52 HBG.........        "
	"               ^"
};

static struct scroller_state *p6_scroll;
static uint32_t p6_offset;
static int32_t p6_offset_direction;

static uint8_t p6_process_char(struct scroller_state *scr_state, uint8_t scroll_character) {
	if(scroll_character == '^') {		// Scrolltext end
		scr_state->text_offset = 0;
		return scr_state->text[scr_state->text_offset++];
	}
	return scroll_character;
}

static void p6_init() {
	p6_scroll = scroller_new(32, 32, 183, 4, p6_scroll_text, part6_scroll_font_data, p6_process_char, 0);
}

static void p6_shutdown() {
	scroller_remove(p6_scroll);
}
#define P6_OFFSET_BOTTOM (400 - 164)

static void p6_bouncing_logo() {
	// PROFILE_FUNCTION();

	p6_offset += p6_offset_direction;
	p6_offset_direction = (p6_offset == 0) ? 1 : (p6_offset == P6_OFFSET_BOTTOM) ? -1 : p6_offset_direction;

	struct rect dest_rect = { 0, 18, BUFFER_WIDTH, 164 };
	uint32_t offset_x = (BUFFER_WIDTH - part6_large_logo_data->width) >> 1;

	blit_clipped(part6_large_logo_data, offset_x, 18-p6_offset, dest_rect, 0);
}

static void p6_render_scroll_buffer(struct scroller_state *dat) {
	// PROFILE_FUNCTION();
	uint32_t *scroll_dest = buffer + (dat->dest_offset_y * BUFFER_WIDTH) + ((BUFFER_WIDTH - 320) >> 1);
	uint8_t *scroll_src = dat->buffer;
	for(size_t i = 0; i < 32; ++i) {
		for(size_t j = 0; j < 320; ++j) {
			uint8_t color_index = scroll_src[(dat->char_render_offset - 370 + j) & (SCROLL_BUFFER_WIDTH - 1)];
			scroll_dest[j] = dat->font->palette[color_index];
		}
		scroll_dest += BUFFER_WIDTH;
		scroll_src += SCROLL_BUFFER_WIDTH;
	}
}

static uint32_t p6_update()  {
	// PROFILE_NAMED("part6 all");
	p6_bouncing_logo();
	scroller(p6_scroll);
	p6_render_scroll_buffer(p6_scroll);

	return mkfw_is_button_pressed(window, MOUSE_BUTTON_LEFT);
}

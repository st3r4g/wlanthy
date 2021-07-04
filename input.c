#include <anthy/anthy.h>
#include <anthy/input.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct anthy_input_context {
	anthy_context_t ctx;
	struct anthy_input_preedit *pe;
	int state;
};

static const char ini = 'a';
static const char fin = 'z';

static signed char map1[256] = {0};

static const char *map2[] = {
	"あ", "い", "う", "え", "お",
	"ぁ", "ぃ", "ぅ", "ぇ", "ぉ",
	"か", "き", "く", "け", "こ",
	"が", "ぎ", "ぐ", "げ", "ご",
	"さ", "し", "す", "せ", "そ",
	"ざ", "じ", "ず", "ぜ", "ぞ",
	"た", "ち", "つ", "て", "と",
	"だ", "ぢ", "づ", "で", "ど",
	"な", "に", "ぬ", "ね", "の",
	"は", "ひ", "ふ", "へ", "ほ",
	"ば", "び", "ぶ", "べ", "ぼ",
	"ぱ", "ぴ", "ぷ", "ぺ", "ぽ",
	"ま", "み", "む", "め", "も",
	"や",    0, "ゆ",    0, "よ",
	"ら", "り", "る", "れ", "ろ",
	"わ", "ゐ",    0, "ゑ", "を",
};

static char buffer[1024] = {0};
static char buffer_commit[1024] = {0};
static signed char state = 0;

int anthy_input_init(void) {
	#define AAA(c, n) map1[c] = n
	AAA('a', 0);
	AAA('i', 1);
	AAA('u', 2);
	AAA('e', 3);
	AAA('o', 4);

	AAA('x', -1);
	AAA('k', -2);
	AAA('g', -3);
	AAA('s', -4);
	AAA('z', -5);
	AAA('t', -6);
	AAA('d', -7);
	AAA('n', -8);
	AAA('h', -9);
	AAA('b', -10);
	AAA('p', -11);
	AAA('m', -12);
	AAA('y', -13);
	AAA('r', -14);
	AAA('w', -15);
	#undef AAA

    anthy_set_logger(NULL, 0);
    int r = anthy_init();
    assert(r == 0);

	return 0;
}

struct anthy_input_config* anthy_input_create_config(void) { return 0; }
void anthy_input_free_config(struct anthy_input_config* cfg) {}
struct anthy_input_context*
anthy_input_create_context(struct anthy_input_config* cfg) {
	struct anthy_input_context* ictx = calloc(1, sizeof(struct anthy_input_context));
    ictx->ctx = anthy_create_context();
    assert(ictx->ctx);
	anthy_context_set_encoding(ictx->ctx, ANTHY_UTF8_ENCODING);
	return ictx;
}
void anthy_input_free_context(struct anthy_input_context* ictx) {
    anthy_release_context(ictx->ctx);
	free(ictx);
}

int anthy_input_get_state(struct anthy_input_context* ictx) { return ictx->state; }
int anthy_input_get_selected_map(struct anthy_input_context* ictx) { return 0; }
void anthy_input_erase_prev(struct anthy_input_context* ictx) {}
void anthy_input_commit(struct anthy_input_context* ictx) {}
void anthy_input_move(struct anthy_input_context* ictx, int lr) {}
void anthy_input_resize(struct anthy_input_context* ictx, int lr) {}

void anthy_input_key(struct anthy_input_context* ictx, int c) {
		const char *d = "";
		if (c >= ini && c <= fin) {
			if (map1[c] >= 0) {
				if (state)
					buffer[strlen(buffer)-1] = '\0';
				d = map2[-state*5+map1[c]];
				state = 0;
				strcat(buffer, d);
			} else {
				// if state == 't' and c == 's', ignore
				if (state == -8) {
					buffer[strlen(buffer)-1] = '\0';
					d = "ん";
					strcat(buffer, d);
				} else if (state == map1[c]) {
					buffer[strlen(buffer)-1] = '\0';
					d = "っ";
					strcat(buffer, d);
				}
				state = map1[c];
				buffer[strlen(buffer)] = c;
				buffer[strlen(buffer)+1] = '\0';
			}
		}
		ictx->state = ANTHY_INPUT_SF_CURSOR;
}

void anthy_input_space(struct anthy_input_context* ictx) {
    anthy_set_string(ictx->ctx, buffer);
	ictx->state = ANTHY_INPUT_SF_EDITING;
}

int anthy_input_map_select(struct anthy_input_context* ictx, int map) { return 0; }

struct anthy_input_preedit*
anthy_input_get_preedit(struct anthy_input_context* ictx) {
	ictx->pe = calloc(1, sizeof(struct anthy_input_preedit));

	if (ictx->state == ANTHY_INPUT_SF_EDITING) {
		struct anthy_conv_stat stat;
		anthy_get_stat(ictx->ctx, &stat);
		ictx->pe->segment = calloc(stat.nr_segment, sizeof(struct anthy_input_segment));
		ictx->pe->cur_segment = ictx->pe->segment;
		for (int i=0; i<stat.nr_segment; i++) {
			struct anthy_segment_stat segstat;
			anthy_get_segment_stat(ictx->ctx, i, &segstat);
			//printf("%d %d\n", segstat.nr_candidate, segstat.seg_len);
			char *buf = malloc(256*sizeof(char));
			anthy_get_segment(ictx->ctx, i, 0, buf, 256);
			ictx->pe->segment[i].str = buf;
			ictx->pe->segment[i].next = (i == stat.nr_segment-1 ? NULL : ictx->pe->segment+i+1);
		}
		ictx->pe->segment[stat.nr_segment-1].next = NULL;
	} else {
		ictx->pe->segment = calloc(1, sizeof(struct anthy_input_segment));
		ictx->pe->segment->str = buffer;
	}

	return ictx->pe;
}

void anthy_input_free_preedit(struct anthy_input_preedit* pedit) {
	if (pedit->cur_segment) { // stupid workaround to avoid segfault
		for (struct anthy_input_segment* cur = pedit->segment; cur != NULL; cur = cur->next) {
				 free(cur->str);
		}
	}
	free(pedit->segment);
	free(pedit);
}

//anthy_quit(); where do I put this???

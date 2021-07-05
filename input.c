#include <anthy/anthy.h>
#include <anthy/input.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct anthy_input_context {
	anthy_context_t ctx;
	int cur;
	int *choices;
	int state;
	int tail;
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
	"や",   "", "ゆ",   "", "よ",
	"ら", "り", "る", "れ", "ろ",
	"わ", "ゐ",   "", "ゑ", "を",
	"ちゃ", "ち", "ちゅ", "ちぇ", "ちょ",
	"じゃ", "じ", "じゅ", "じぇ", "じょ",
};

static char buffer[1024] = {0};
static char buffer_commit[1024] = {0};
static char buffer_commit_[1024] = {0};
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
	AAA('c', -16);
	AAA('j', -17);
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
	ictx->state = ANTHY_INPUT_ST_NONE;
	return ictx;
}
void anthy_input_free_context(struct anthy_input_context* ictx) {
    anthy_release_context(ictx->ctx);
	free(ictx);
}

int anthy_input_get_state(struct anthy_input_context* ictx) { return ictx->state; }
int anthy_input_get_selected_map(struct anthy_input_context* ictx) { return 0; }
void anthy_input_erase_prev(struct anthy_input_context* ictx) {
	if (ictx->state == ANTHY_INPUT_ST_CONV) {
		ictx->state = ANTHY_INPUT_ST_EDIT;
	} else {
		if (ictx->tail > 0) {
			buffer[strlen(buffer)-1] = '\0';
			ictx->tail--;
			if (ictx->tail == 0)
				state = 0;
		} else {
			buffer[strlen(buffer)-1] = '\0';
			buffer[strlen(buffer)-1] = '\0';
			buffer[strlen(buffer)-1] = '\0';
		}
		//printf("tail %d\n", ictx->tail);
	}
}
void anthy_input_commit(struct anthy_input_context* ictx) {
		buffer_commit_[0] = '\0';

		struct anthy_conv_stat stat;
		anthy_get_stat(ictx->ctx, &stat);
		for (int i=0; i<stat.nr_segment; i++) {
			char buf[256];
			anthy_get_segment(ictx->ctx, i, ictx->choices[i], buf, 256);
			strcat(buffer_commit_, buf);
		}

		buffer[0] = '\0';
		ictx->state = ANTHY_INPUT_ST_NONE;
}
static void pbc(int *n, int inc, int period) {
	*n = (*n+inc) % period;
	if (*n < 0) *n += period;
}
void anthy_input_move(struct anthy_input_context* ictx, int lr) {
	struct anthy_conv_stat stat;
	anthy_get_stat(ictx->ctx, &stat);
	pbc(&ictx->cur, lr, stat.nr_segment);
}
void anthy_input_resize(struct anthy_input_context* ictx, int lr) {
	//anthy_resize_segment(ictx->ctx, )
}
void anthy_input_next_candidate(struct anthy_input_context* ictx) {
	struct anthy_segment_stat segstat;
	anthy_get_segment_stat(ictx->ctx, ictx->cur, &segstat);
	pbc(ictx->choices+ictx->cur, +1, segstat.nr_candidate);
}
void anthy_input_prev_candidate(struct anthy_input_context* ictx) {
	struct anthy_segment_stat segstat;
	anthy_get_segment_stat(ictx->ctx, ictx->cur, &segstat);
	pbc(ictx->choices+ictx->cur, -1, segstat.nr_candidate);
}

void anthy_input_key(struct anthy_input_context* ictx, int c) {
	if (ictx->state == ANTHY_INPUT_ST_CONV) {
		anthy_input_commit(ictx);
	}

		const char *d = "";
		if (c >= ini && c <= fin) {
			if (map1[c] >= 0) {
				while (ictx->tail > 0) {
					buffer[strlen(buffer)-1] = '\0';
					ictx->tail--;
				}
				d = map2[-state*5+map1[c]];
				state = 0;
				strcat(buffer, d);
			} else {
				if (state == -8) {
					buffer[strlen(buffer)-1] = '\0';
					ictx->tail--;
					d = "ん";
					strcat(buffer, d);
				} else if (state == map1[c]) {
					buffer[strlen(buffer)-1] = '\0';
					ictx->tail--;
					d = "っ";
					strcat(buffer, d);
				}
				if (!(state == -6 && map1[c] == -4) && !(state == -16 && map1[c] == -9))
					state = map1[c];
				int len = strlen(buffer);
				buffer[len] = c;
				buffer[len+1] = '\0';
				ictx->tail++;
			}
		}
		ictx->state = ANTHY_INPUT_ST_EDIT;
		//printf("tail %d\n", ictx->tail);
}

void anthy_input_space(struct anthy_input_context* ictx) {
    anthy_set_string(ictx->ctx, buffer);

	free(ictx->choices);
	struct anthy_conv_stat stat;
	anthy_get_stat(ictx->ctx, &stat);
	ictx->choices = calloc(stat.nr_segment, sizeof(int));

	ictx->cur = 0;

	ictx->state = ANTHY_INPUT_ST_CONV;
}

int anthy_input_map_select(struct anthy_input_context* ictx, int map) { return 0; }

struct anthy_input_preedit*
anthy_input_get_preedit(struct anthy_input_context* ictx) {
	struct anthy_input_preedit *pe = calloc(1, sizeof(struct anthy_input_preedit));

	if (ictx->state == ANTHY_INPUT_ST_CONV) {
		struct anthy_conv_stat stat;
		anthy_get_stat(ictx->ctx, &stat);
		pe->segment = calloc(stat.nr_segment, sizeof(struct anthy_input_segment));
		pe->cur_segment = pe->segment+ictx->cur;
		for (int i=0; i<stat.nr_segment; i++) {
			struct anthy_segment_stat segstat;
			anthy_get_segment_stat(ictx->ctx, i, &segstat);
			//printf("%d %d\n", segstat.nr_candidate, segstat.seg_len);
			char *buf = malloc(256*sizeof(char));
			anthy_get_segment(ictx->ctx, i, ictx->choices[i], buf, 256);
			pe->segment[i].str = buf;
			pe->segment[i].next = (i == stat.nr_segment-1 ? NULL : pe->segment+i+1);
		}
		pe->segment[stat.nr_segment-1].next = NULL;
	} else if (ictx->state == ANTHY_INPUT_ST_NONE) { // not correct, we might have commit in ST_CONV
		pe->commit = buffer_commit_;
	} else {
		pe->segment = calloc(1, sizeof(struct anthy_input_segment));
		pe->segment->str = buffer;
	}

	return pe;
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

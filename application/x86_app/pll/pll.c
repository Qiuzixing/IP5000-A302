/*
 * Copyright (c) 2004-2012 ASPEED Technology Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> //for floorf()
#include <getopt.h>

#include "pll.h"

#define USE_A0_REG  0

#if USE_A0_REG
#define CHIP_REV_STR      "A0"
#else
#define CHIP_REV_STR      "A1"
#endif

#define FIN         24000000 //24MHz
#define SSC_EN      1
#define MIN_M       10         //10 ~ 200
#define MAX_M       200        //10 ~ 200
#define TOTAL_M     (MAX_M - MIN_M + 1)
#define MIN_N       1          //1 ~ 32
#define MAX_N       32         //1 ~ 32
#define TOTAL_N     (MAX_N - MIN_N + 1)
#define MIN_P       1          //1 ~ 64
#define MAX_P       64         //1 ~ 64
#define TOTAL_P     (MAX_P - MIN_P + 1)
#define MIN_OD       1         // 1 ~ 8
#define MAX_OD       8         //1 // 1 ~ 8
#define TOTAL_OD     (MAX_OD - MIN_OD + 1)
#define M_RANGE_MIN(ssc_en)    (((ssc_en) == 1)?(25):(20))
#define M_RANGE_MAX(ssc_en)    (((ssc_en) == 1)?(100):(256))
#define FREF_RANGE_MIN(ssc_en) (((ssc_en) == 1)?(20*1000000):(5*1000000))
#define FREF_RANGE_MAX(ssc_en) (((ssc_en) == 1)?(80*1000000):(100*1000000))
#define FVCO_RANGE_MIN         (1000*1000000) //in Hz
#define FVCO_RANGE_MAX         (2000*1000000) //in Hz
#define FOUT_RANGE_MIN         (15625000)     //in Hz
#define FOUT_RANGE_MAX         (1000*1000000) //in Hz
#define TARGET_TOLERANCE_PPM   (5000) //VESA is 5000ppm, HDMI audio average is 100ppm, and under 2000ppm jitter.


#define SOLUTION_NUM  30
#define abs(v) (((v) < 0)?(-(v)):(v))

#define TABLE_SIZE   (SOLUTION_NUM)

struct question_t {
	long long Fin;
	unsigned int ssc_en;
	long long target_clk; //in 100Hz
	unsigned char *file;
	double ppm;
} question;

struct pll_table_t {
	long long Fout;      //integer mode Fout
	double Fout_frac; //fractional mode Fout
	unsigned int M;
	unsigned int N;
	unsigned int P;
	unsigned int OD;
	long long err_Hz;
	double err_ppm;

	double M_target;
	double M_fractional;
	double err_frac;
	unsigned int INIT;
} pll_table[TABLE_SIZE];

#define MAX_PLL_REG_TABLE_SIZE    500
struct pll_reg_t {
	unsigned int clock;                  //in 100Hz

	long long Fout;      //integer mode Fout
	double Fout_frac; //fractional mode Fout
	double M_target;
	double M_fractional;

	long long err_Hz;
	double err_ppm;
	double err_frac;

	union {
		unsigned int all;
		struct {
			unsigned int m:8;            //scu28[7:0];
			unsigned int n:5;            //scu28[12:8];
			unsigned int p:6;            //scu28[18:13];
			unsigned int od:3;           //scu28[21:19];
#if USE_A0_REG //For AST1520 A0, chip bug
			/* AST1520 A0 Define */
			unsigned int sip:5;          //scu28[26:22];
			unsigned int sic:5;          //scu28[31:27];
#else
			/* AST1520 A1 Define */
			unsigned int sic:5;          //scu28[26:22];
			unsigned int sip:5;          //scu28[31:27];
#endif
		} bits;
	} scu28; //or scu1c

	union {
		unsigned int all;
		struct {
			unsigned int pwrdn:1;        //scu130[0]     default = 1'b0
			unsigned int bypass:1;       //scu130[1]     default = 1'b0
			unsigned int reset:1;        //scu130[2]     default = 1'b0
			unsigned int ssc_en:1;       //scu130[3]     1: fractional/spread spectrum 0: integer
			unsigned int saw_en:1;       //scu130[4]     1: spread spectrum mode 0:fractional mode
			unsigned int lksel:2;        //scu130[6:5]   default = 2'b00
			unsigned int sgain:3;        //scu130[9:7]   default = 3'b011
			unsigned int slock:1;        //scu130[10]    default = 1'b1
			unsigned int pre_n:1;        //scu130[11]    default = 1'b0
			unsigned int sfc:2;          //scu130[13:12] default = 2'b00
			unsigned int :19;
		} bits;
	} scu130; //or scu13c

	union {
		unsigned int all;
		struct {
			unsigned int step:14;        //scu134[13:0]  ssc mode only
			unsigned int ud_cycle:14;    //scu134[27:14] ssc mode only
			unsigned int :3;
		} bits;
	} scu134; //or scu140

	union {
		unsigned int all;
		struct {
			unsigned int init:19;        //scu138[18:0]
			unsigned int :13;
		} bits;
	} scu138; //or scu144
} target_pll_reg[MAX_PLL_REG_TABLE_SIZE];



/*
** Return:
** 0: invalid
** unsigned long long: The valid Fout
*/
unsigned long long validate_Fvco(long long Fin, unsigned int ssc_en, unsigned int m, unsigned int n)
{
	unsigned long long Fvco, Fref;

	Fref = Fin / n;
	/*
	** SSC_EN == 1:criteria 80MHz >= FIN/N >= 20MHz
	** SSC_EN == 0:criteria 100MHz >= FIN/N >= 5MHz
	*/
	if (Fref > FREF_RANGE_MAX(ssc_en))
		return 0;
	if (Fref < FREF_RANGE_MIN(ssc_en))
		return 0;

	/*
	** Bruce150923. (m / n) is int operation which will remove floating parts.
	** We need the floating value for accurate result.
	** Fvco = (m / n) * Fin;
	*/
	Fvco = Fin * m / n;

	// criteria: 2000MHz >= Fvco >= 1000MHz
	if (Fvco > FVCO_RANGE_MAX)
		return 0;
	if (Fvco < FVCO_RANGE_MIN)
		return 0;

	return Fvco; //in Hz
}

void dump_pll_table_item(struct pll_table_t *t, unsigned int ssc_en)
{
	printf("Fout:%12lli, M:%3d, N:%2d, P:%2d, OD:%1d, err_Hz:%9lli, err_ppm:%6.0lf, ",
			t->Fout, t->M, t->N, t->P, t->OD, t->err_Hz, t->err_ppm);

	if (ssc_en == 1)
		printf("Fout_frac:%14.2lf, M_target:%8.10lf, M_fractional:%8.10lf, err_frac:% 4.4lfppm, INIT=0x%08X",
				t->Fout_frac, t->M_target, t->M_fractional, t->err_frac, t->INIT);

	printf("\n");
}

void dump_pll_table(struct pll_table_t *t, unsigned int size, unsigned int ssc_en)
{
	unsigned int i;

	//printf("Total:%d items\n", size);
	for (i = 0; i < size; i++, t++) {
		dump_pll_table_item(t, ssc_en);
	}
}

void swap_pll_table_item(struct pll_table_t *item_i, struct pll_table_t *item_j)
{
	struct pll_table_t t;

	t = *item_i;
	*item_i = *item_j;
	*item_j = t;
}

void calc_err(struct pll_table_t *p, long long target_clock)
{
	p->err_Hz = p->Fout - target_clock;
	if (abs(p->err_Hz) > ((target_clock * 100) / 1000)) {
		//Ignore the case over 100000 ppm
		p->err_ppm = 9999999;
	} else {
		p->err_ppm = (((double)(p->err_Hz) / (double)target_clock)) * 1000000;
	}
}

int calc_fractional_setting(struct pll_table_t *item, unsigned long long target_clock, long long Fin)
{
	double M_target, M_fractional;
	double b;
	unsigned int N = item->N, P = item->P, M_integer = item->M;
	unsigned int INIT = 0;
	int diff = 0;

	M_target = ((double)target_clock * P * N) / (double)Fin;
	//printf("target_clock=%lld, N=%d, P=%d, M_integer=%d, M_target=%f\n", target_clock, N, P, M_integer, M_target);

	/* This is fractional PLL limit. */
	if (abs(M_target - M_integer) > 3.99) {
		//printf("ERROR!! Out of fractional range\n");
		item->M_target = M_target;
		item->M_fractional = -1;
		item->err_frac = 99;
		item->INIT = 0;
		return -1;
	}
	b = (M_target - M_integer) * (1 << 16);
	if (b > 0) {
		//INIT = floorf(b);
		diff = (int)(b + 0.5);
		INIT = diff;
	} else if (b < 0) {
		//INIT = (1 << 19) + floorf(b) + 1;
		diff = (int)(b - 0.5);
		INIT = (1 << 19) + diff + 1;
	} else {
		//For the case that we don't need fractional value.
		INIT = 0;
	}

	M_fractional = M_integer + ((double)(diff) / (1 << 16));

	//item->err_frac = (M_fractional - M_target) * 100 / M_target;
	//printf("R:%10f\n", (((Fin * M_fractional / N / P) - (double)target_clock)/target_clock) * 1000000);
	item->Fout_frac = Fin * M_fractional / N / P;
	item->err_frac = (((item->Fout_frac) - (double)target_clock)/target_clock) * 1000000;
	//printf("M_fractional=%f, INT=%d (0x%08X), err_frac=%f%%\n", M_fractional, INIT, INIT, item->err_frac);

	item->M_target = M_target;
	item->M_fractional = M_fractional;
	item->INIT = INIT;

	return 0;
}

void find_pll_item(struct pll_table_t *table, unsigned int size, unsigned int m, unsigned int n, unsigned int p, unsigned int od, unsigned int ssc_en)
{
	unsigned int i;

	for (i = 0; i < size; i++, table++) {
		if (table->M != m)
			continue;
		if (table->N != n)
			continue;
		if (table->P != p)
			continue;
		if (table->OD != od)
			continue;
		printf("Found!!:");
		dump_pll_table_item(table, ssc_en);
	}
}

unsigned int M_to_SIC(unsigned int M, unsigned int ssc_en)
{
	if (ssc_en == 0) {
		if (M < 20) {
			printf("M_to_SIC fail?! %d\n", M);
			exit(-1);
		} else if (M < 30) {
			return 0x0C;
		} else if (M < 40) {
			return 0x0F;
		} else if (M < 50) {
			return 0x12;
		} else if (M < 100) {
			return 0x15;
		} else {
			return 0x18;
		}
		return -1;
	}

	//For ssc_en == 1
	if (M < 20) {
		printf("M_to_SIC fail?! %d\n", M);
		exit(-1);
	} else if (M < 30) {
		return 0x0C;
	} else if (M < 40) {
		return 0x0F;
	} else if (M < 50) {
		return 0x12;
	} else if (M < /*81*/ 101) {
		return 0x15;
	} else {
		printf("M_to_SIC fail?! %d\n", M);
		exit(-1);
	}

	return -1;
}

unsigned int M_to_SIP(unsigned int M, unsigned int ssc_en)
{
	if (ssc_en == 0) {
		if (M < 20) {
			printf("M_to_SIP fail?! %d\n", M);
			exit(-1);
		} else if (M < 30) {
			return 0x05;
		} else if (M < 40) {
			return 0x08;
		} else if (M < 50) {
			return 0x0B;
		} else if (M < 100) {
			return 0x0E;
		} else {
			return 0x11;
		}
		return -1;
	}

	//For ssc_en == 1
	if (M < 20) {
		printf("M_to_SIP fail?! %d\n", M);
		exit(-1);
	} else if (M < 30) {
		return 0x08;
	} else if (M < 40) {
		return 0x0B;
	} else if (M < 50) {
		return 0x0E;
	} else if (M < /*81*/ 101) {
		return 0x11;
	} else {
		printf("M_to_SIP fail?! %d\n", M);
		exit(-1);
	}

	return -1;
}

void dump_reg(struct pll_reg_t *r, unsigned int ssc_en)
{
	printf("{ %8d, 0x%08X, 0x%08X, 0x%08X, 0x%08X }, ", r->clock, r->scu28.all, r->scu130.all, r->scu134.all, r->scu138.all);

	printf("//Clock(%9d),M(%3d),N(%2d),P(%2d),OD(%2d),Fout(%9lld),err_int(%8lldHz,% 5.0lfppm)", r->clock * 100, r->scu28.bits.m + 1, r->scu28.bits.n + 1, r->scu28.bits.p + 1, r->scu28.bits.od + 1, r->Fout, r->err_Hz, r->err_ppm);
	if (ssc_en == 1)
		printf(",Fout_frac(%9.0lf),Frac(%6d[0x%08X]),err_frac(% 2.4lfppm)", r->Fout_frac, r->scu138.bits.init, r->scu138.bits.init, r->err_frac);

	printf("\n");
}

void dump_result(struct pll_reg_t *t, unsigned int size, unsigned int ssc_en)
{
	unsigned int i;

	printf("\n");
	printf("/*\n");
	printf("** Register Setting:\n");
	printf("{clk(100Hz), SCU28(%s),     SCU130,     SCU134,     SCU138 }\n", CHIP_REV_STR);
	printf("*/\n");

	for (i = 0; i < size; i++, t++) {
		dump_reg(t, ssc_en);
	}
	printf("\n");
}

void save_result(long long target_clock, struct pll_table_t *t, struct pll_reg_t *r, unsigned int ssc_en)
{
	r->clock = target_clock / 100;
	r->Fout = t->Fout;
	r->Fout_frac = t->Fout_frac;
	r->M_target = t->M_target;
	r->M_fractional = t->M_fractional;
	r->err_Hz = t->err_Hz;
	r->err_ppm = t->err_ppm;
	r->err_frac = t->err_frac;

	r->scu28.all = 0;
	r->scu28.bits.m = t->M - 1;
	r->scu28.bits.n = t->N - 1;
	r->scu28.bits.p = t->P - 1;
	r->scu28.bits.od = t->OD - 1;
	r->scu28.bits.sic = M_to_SIC(t->M, ssc_en);
	r->scu28.bits.sip = M_to_SIP(t->M, ssc_en);

	r->scu130.all = 0;
	r->scu130.bits.pwrdn = 0;            //scu130[0]     default = 1'b0
	r->scu130.bits.bypass = 0;           //scu130[1]     default = 1'b0
	r->scu130.bits.reset = 0;            //scu130[2]     default = 1'b0
	r->scu130.bits.ssc_en = ssc_en;      //scu130[3]     1: fractional/spread spectrum 0: integer
	r->scu130.bits.saw_en = 0;           //scu130[4]     1: spread spectrum mode 0:fractional mode
	r->scu130.bits.lksel = 0;            //scu130[6:5]   default = 2'b00
	r->scu130.bits.sgain = 0x3;          //scu130[9:7]   default = 3'b011
	r->scu130.bits.slock = 1;            //scu130[10]    default = 1'b1
	r->scu130.bits.pre_n = 0;            //scu130[11]    default = 1'b0
	r->scu130.bits.sfc = 0;              //scu130[13:12] default = 2'b00

	r->scu134.all = 0;                   //ssc mode only

	r->scu138.all = 0;
	r->scu138.bits.init = t->INIT;       //scu138[18:0]

	printf("Clock(%10lld), M(%4d), N(%2d), P(%2d), OD(%2d), err_int(%8lldHz, %6.0lfppm)", target_clock, r->scu28.bits.m + 1, r->scu28.bits.n + 1, r->scu28.bits.p + 1, r->scu28.bits.od + 1, r->err_Hz, r->err_ppm);
	if (ssc_en == 1) {
		printf(", Frac(%6d[0x%08X]), M_target(%2.10lf), M_frac(%2.10lf), err_frac(% 2.4lfppm)", r->scu138.bits.init, r->scu138.bits.init, t->M_target, t->M_fractional, r->err_frac);
	}

	printf("\n");
}


#define is_empty_entry(item_j) ((item_j->M == 0) && (item_j->N == 0) && (item_j->P == 0) && (item_j->OD == 0))
#define is_bad_odd(v) (((v) > 1) && ((v) & 0x1))
#define is_bad_div(item_j) (is_bad_odd((item_j)->P) && !is_bad_odd((item_j)->OD))
#define is_very_bad_div(item_j) (is_bad_odd((item_j)->P) && is_bad_odd((item_j)->OD))
#define is_ng_div(item_j) (is_bad_div(item_j) || is_very_bad_div(item_j))
#define is_good_div(item_j) (!is_bad_odd((item_j)->P) && !is_bad_odd((item_j)->OD))

void save_n_sort_pll_table(struct pll_table_t *table, unsigned int size, struct pll_table_t i, struct question_t *q)
{
	unsigned int j;
	struct pll_table_t *item_j;
	struct pll_table_t *item_i = &i;
	unsigned int ssc_en = q->ssc_en;

	for (j = 0; j < size; j++) {
		item_j = table + j;

#if 0
		if (item_i->err_ppm == 0)
			dump_pll_table_item(item_i, ssc_en);
#endif

		if (is_empty_entry(item_j))
			goto swap_n_return;

		/*
		** Under valid ppm, we pick dividers which won't cause jitter or bad duty.
		** Odd div is not good. Two odd div is the worst, causes big jitter.
		** If there must have odd value, put it to the last div.
		*/
		if ((abs(item_i->err_ppm) < q->ppm) && (abs(item_j->err_ppm) < q->ppm)) {
			if (ssc_en == 1) { //fractional
				/*
				** Fractional mode can use frac to get precise ppm, so we choose
				** only even dividers.
				*/
				if (!is_good_div(item_j) && is_good_div(item_i))
					goto swap;
				if (is_good_div(item_j) && !is_good_div(item_i))
					continue;
			} else {
				/*
				** Avoid two odd div under integer mode.
				*/
				if (is_very_bad_div(item_j) && !is_very_bad_div(item_i))
					goto swap;
				if (!is_very_bad_div(item_j) && is_very_bad_div(item_i))
					continue;
			}
		}

		//Check err_Hz
		if (abs(item_i->err_Hz) < abs(item_j->err_Hz))
			goto swap;
		if (abs(item_i->err_Hz) > abs(item_j->err_Hz))
			continue;

		//err_Hz are abs equal, now check N and P
		//Smaller N is better
		if (item_i->N < item_j->N)
			goto swap;
		if (item_i->N > item_j->N)
			continue;

		/*
		** From HJ, If odd div number is used, put it to the last div.
		*/
		//if (item_i->N == item_j->N), check P value
		if ((item_i->P & 0x1) && !(item_j->P & 0x1))
			continue; //i is odd, but j is even. Even is better
		if (!(item_i->P & 0x1) && (item_j->P & 0x1))
			goto swap; //i is even, but j is odd. Even is better

		/*
		** check OD value
		*/
		//It will be better if we don't need OD. (OD == 1)
		if ((item_i->OD == 1) && (item_j->OD != 1))
			goto swap;
		if ((item_i->OD != 1) && (item_j->OD == 1))
			continue;
		//Otherwise, even value is better
		if ((item_i->OD & 0x1) && !(item_j->OD & 0x1))
			continue; //i is odd, but j is even. Even is better
		if (!(item_i->OD & 0x1) && (item_j->OD & 0x1))
			goto swap; //i is even, but j is odd. Even is better

		//err_Hz are abs equal, pick the Nagitive one. (Bruce preferred slower pixel clock).
		if (item_i->err_Hz < item_j->err_Hz)
			goto swap;
		if (item_i->err_Hz > item_j->err_Hz)
			continue;

		//Pick smallest OD.
		if (item_i->OD < item_j->OD)
			goto swap;
		if (item_i->OD > item_j->OD)
			continue;

		//Both i and j are odd/even, pick smaller P.
		if (item_i->P < item_j->P)
			goto swap;
		if (item_i->P > item_j->P)
			continue;

		//Finally, we check M. Smaller is better?
		if (item_i->M < item_j->M)
			goto swap;
		if (item_i->M > item_j->M)
			continue;

		//Here means: err_Hz the same, N the same, P the same, M the same, OD the same. This can't happen!!
		printf("ERROR!! Unexpected PLL item?!\n");
		dump_pll_table_item(item_i, ssc_en);
		dump_pll_table_item(item_j, ssc_en);
		exit(-1);
swap:
		swap_pll_table_item(item_i , item_j);
	}

	return;

swap_n_return:
	swap_pll_table_item(item_i , item_j);
}

void pll_walker_restart(unsigned int ssc_en, struct pll_table_t *p)
{
	p->M = M_RANGE_MIN(ssc_en);
	p->N = MIN_N;
	p->P = MIN_P;
	p->OD = MIN_OD;
}

int pll_walker_next(long long Fin, unsigned int ssc_en, struct pll_table_t *p)
{
	//Create M,N,P table. Contains all M, N, P combination.
	for (; p->M <= M_RANGE_MAX(ssc_en); p->M += 1) {
		for (; p->N <= MAX_N; p->N += 1) {
			unsigned long long Fvco;

			Fvco = validate_Fvco(Fin, ssc_en, p->M, p->N);
			if (Fvco == 0)
				continue;

			// A valid Fvco found
			for (; p->P <= MAX_P; p->P += 1) {
				for (/*p->OD += 1*/; p->OD <= MAX_OD; p->OD += 1) {
					p->Fout = Fvco / (p->P) / (p->OD);
					return 1;
				}
				p->OD = MIN_OD;
			}
			p->P = MIN_P;
		}
		p->N = MIN_N;
	}

	p->M = M_RANGE_MIN(ssc_en);
	return 0;
}

void pll_walker(struct question_t *q, long long target_clock)
{
	struct pll_table_t p;
	unsigned int ssc_en = q->ssc_en;

	memset(pll_table, 0, sizeof(struct pll_table_t)*TABLE_SIZE);

	pll_walker_restart(ssc_en, &p);

	while (pll_walker_next(q->Fin, ssc_en, &p)) {
		if (ssc_en == 1)
			if (calc_fractional_setting(&p, target_clock * p.OD, q->Fin))
				goto next;

		calc_err(&p, target_clock);

		//Pass p as pointer kills swap_pll_table_item() inside save_n_sort_pll_table(). Don't know why?
		save_n_sort_pll_table(pll_table, TABLE_SIZE, p, q);
next:
		p.OD += 1;
	}
}

void _main(struct question_t *q)
{
	struct pll_reg_t *r = target_pll_reg;
	unsigned int reg_table_size = 0;
	long long target_clk = q->target_clk;
	unsigned int ssc_en = q->ssc_en;

	memset(target_pll_reg, 0, sizeof(struct pll_reg_t)*MAX_PLL_REG_TABLE_SIZE);

	if (target_clk) { //From parameter
		pll_walker(q, target_clk * 100);
		printf("================================================================\n");
		printf("Possible Solution for %12lld Hz:\n", target_clk * 100);
		dump_pll_table(pll_table, TABLE_SIZE, ssc_en);
		printf("Best Solution:\n");
		// Best result is in pll_table[0]
		save_result(target_clk * 100, pll_table, r, ssc_en);
		printf("================================================================\n");
		r++;
	} else if (q->file) { //From File
		FILE *f;
		char *line = NULL;
		size_t len = 0;
		ssize_t read;

		f = fopen(q->file, "r");
		if (f == NULL) {
			printf("Failed to open file:%s\n", q->file);
			exit(-1);
		}
		while ((read = getline(&line, &len, f)) != -1) {
			if (read == 0)
				continue;

			target_clk = atoi(line);
			if (!target_clk)
				continue;

			pll_walker(q, target_clk * 100);
			// Best result is in pll_table[0]
			save_result(target_clk * 100, pll_table, r, ssc_en);
			r++;
		}

		free(line);
		fclose(f);
	} else { //From built-in table
		int i;
		for (i = 0; i < target_pll_size; i++) {
			pll_walker(q, target_pll[i] * 100);
			// Best result is in pll_table[0]
			save_result(target_pll[i] * 100, pll_table, r, ssc_en);
			r++;
		}
	}
	reg_table_size = (r - target_pll_reg);
	dump_result(target_pll_reg, reg_table_size, ssc_en);

}

static const struct option longopts[] = {
	{"fin",	required_argument,	NULL, 'i'},
	{"target",	required_argument,	NULL, 't'},
	{"ssc_en", required_argument,	NULL, 's'},
	{"pll_file", required_argument,	NULL, 'f'},
	{"ppm", required_argument,	NULL, 'p'},
	{"help", no_argument,	NULL, 'h'},
	{NULL,		0,		NULL,  0}
};


void help(void)
{
	fprintf(stderr,"\
 Usage: \n\
  pll [-i <Fin>] [-s <en>] [-p <ppm>] [-t <target clock>] [-f <file name>]\n\
\n\
    -i <Fin>            System clock input in Hz (24000000 or 25000000). Default:24000000 \n\
    -s <en>             Enable fractional mode (1 or 0). Default:1 \n\
    -p <ppm>            Target tolerance in PPM. Default:5000 \n\
    -t <target clock>   Desired output clock in 100Hz (ex: 27MHz == 270000). Default: built-in table \n\
    -f <file name>      Desired output clock in 100Hz list in file format. See pll.txt for example. \n\
\n\
");
	return;
}

int main (int argc, char **argv)
{
	struct question_t *q = &question;

	q->Fin = FIN;
	q->ssc_en = SSC_EN;
	q->target_clk = 0;
	q->file = NULL;
	q->ppm = TARGET_TOLERANCE_PPM;

	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "i:t:s:f:p:h", longopts, &index);

		if (c == -1)
			break;

		switch (c) {
			case 'i':
				q->Fin = atoi(optarg);
				break;
			case 't':
				q->target_clk = atoi(optarg);
				break;
			case 's':
				q->ssc_en = atoi(optarg);
				break;
			case 'f':
				q->file = optarg;
				break;
			case 'p':
				q->ppm = (double)atoi(optarg);
				break;
			case 'h':
				//passthrough
			default:
				help();
				exit(0);
		}
	}

	printf("Fin=%lldHz, Target=%lldHz, SSC_EN=%d, file=%s, ppm=%.0f\n", q->Fin, q->target_clk*100, q->ssc_en, q->file, q->ppm);
	_main(q);

	return 0;
}

/* The MIT License

   Copyright (c) 2008 Genome Research Ltd (GRL).

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

/* Contact: Heng Li <lh3@sanger.ac.uk> */

#ifndef BWA_BWT_H
#define BWA_BWT_H

#include <stdint.h>
#include <stddef.h>

// requirement: (OCC_INTERVAL%16 == 0); please DO NOT change this line because some part of the code assume OCC_INTERVAL=0x80
#define OCC_INTV_SHIFT 7
#define OCC_INTERVAL   (1LL<<OCC_INTV_SHIFT)
#define OCC_INTV_MASK  (OCC_INTERVAL - 1)

#ifndef BWA_UBYTE
#define BWA_UBYTE
typedef unsigned char ubyte_t;
#endif

typedef uint64_t bwtint_t;

typedef struct {
	bwtint_t primary; // S^{-1}(0), or the primary index of BWT
	bwtint_t L2[5]; // C(), cumulative count
	bwtint_t seq_len; // sequence length
	bwtint_t bwt_size; // size of bwt, about seq_len/4
	uint32_t *bwt; // at begin act as BWT string, last act as compressed occurance array.
  uint32_t cnt_table[256];
  int sa_intv;
  bwtint_t n_sa;
        bwtint_t *sa;
} bwt_t;
typedef struct{//one sample line of every 128 entry of Occurrance array
	uint32_t base[16];//base[0]--AA, base[1]--CA, base[2]--GA,...,base[4]--AC,..,base[8]--AG,...,base[12]--AT,...
    uint64_t offset[8];//offset[0]--B1[0:63] high bit, offset[1]--B1[64:127] high bit, offset[2]--B1[0:63] low bit, offset[3]--B1[64:127] low bit, offset[4]--B[0:63] high bit,... string B for BWT matrix last column, string B1 for BWT matrix the second last column.
} Occline;
typedef struct{
    uint64_t refLen;
    Occline *occArray;//must force the occArray align to 256 boundary
    uint32_t *sa_low32;//the length of sa is 2*refLen
    uint32_t *sa_high2;//for more extension ability
    uint64_t c1Array[5];//the cumulative for FDM-index extend one step
    uint64_t c2Array[16];//the cumulative for FDM-index extend two step
} lbwt_t;//lambert_bwt_t

/* typedef struct{//FDM-index type */
/*     uint64_t fs; */
/*     uint64_t rs; */
/*     uint64_t len; */
/*     uint32_t readBegin; */
/*     uint32_t readEnd; */
/* } fdm_t; */
typedef struct {
  //bwtint_t x[3], info;
  uint64_t fs;
  uint64_t rs;
  uint64_t len;
  uint32_t readBegin;
  uint32_t readEnd;
} bwtintv_t;

typedef struct { size_t n, m; bwtintv_t *a; } bwtintv_v;//n space occupied, m space allocated

/* For general OCC_INTERVAL, the following is correct:
#define bwt_bwt(b, k) ((b)->bwt[(k)/OCC_INTERVAL * (OCC_INTERVAL/(sizeof(uint32_t)*8/2) + sizeof(bwtint_t)/4*4) + sizeof(bwtint_t)/4*4 + (k)%OCC_INTERVAL/16])
#define bwt_occ_intv(b, k) ((b)->bwt + (k)/OCC_INTERVAL * (OCC_INTERVAL/(sizeof(uint32_t)*8/2) + sizeof(bwtint_t)/4*4)
*/

// The following two lines are ONLY correct when OCC_INTERVAL==0x80
#define bwt_bwt(b, k) ((b)->bwt[((k)>>7<<4) + sizeof(bwtint_t) + (((k)&0x7f)>>4)])
#define bwt_occ_intv(b, k) ((b)->bwt + ((k)>>7<<4))

/* retrieve a character from the $-removed BWT string. Note that
 * bwt_t::bwt is not exactly the BWT string and therefore this macro is
 * called bwt_B0 instead of bwt_B */
#define bwt_B0(b, k) (bwt_bwt(b, k)>>((~(k)&0xf)<<1)&3)

#define bwt_set_intv(bwt, c, ik) ((ik).x[0] = (bwt)->L2[(int)(c)]+1, (ik).x[2] = (bwt)->L2[(int)(c)+1]-(bwt)->L2[(int)(c)], (ik).x[1] = (bwt)->L2[3-(c)]+1, (ik).info = 0)

#ifdef __cplusplus
extern "C" {
#endif

	void bwt_dump_bwt(const char *fn, const bwt_t *bwt);
	void bwt_dump_sa(const char *fn, const bwt_t *bwt);
  void lbwt_dump_lbwt(const char *fn,const lbwt_t *lbwt);
  void bwt_dump_sa_lambert(const char *fn, const bwt_t *bwt);

	bwt_t *bwt_restore_bwt(const char *fn);
  lbwt_t *lbwt_restore_lbwt(const char *fn);
	void bwt_restore_sa(const char *fn, bwt_t *bwt);
  void bwt_restore_sa_lambert(const char *fn,lbwt_t *lbwt);

	void bwt_destroy(bwt_t *bwt);
  void lbwt_destroy(lbwt_t *bwt);

	void bwt_cal_sa(bwt_t *bwt, int intv);
  void bwt_cal_sa_and_sample(bwt_t *bwt);
  void constructOccArray(lbwt_t *lbwt, char *pac, bwt_t *bwt);

	void bwt_bwtupdate_core(bwt_t *bwt);
  void bwt_update_bwt_lambert(bwt_t *bwt);

	bwtint_t bwt_occ(const bwt_t *bwt, bwtint_t k, ubyte_t c);
	void bwt_occ4(const bwt_t *bwt, bwtint_t k, bwtint_t cnt[4]);
	bwtint_t bwt_sa(const bwt_t *bwt, bwtint_t k);

	// more efficient version of bwt_occ/bwt_occ4 for retrieving two close Occ values
	void bwt_gen_cnt_table(bwt_t *bwt);
	void bwt_2occ(const bwt_t *bwt, bwtint_t k, bwtint_t l, ubyte_t c, bwtint_t *ok, bwtint_t *ol);
	void bwt_2occ4(const bwt_t *bwt, bwtint_t k, bwtint_t l, bwtint_t cntk[4], bwtint_t cntl[4]);
  // inline void bwt_occ4_lambert(const bwt_t *bwt, bwtint_t k, bwtint_t cntk[4]);

	int bwt_match_exact(const bwt_t *bwt, int len, const ubyte_t *str, bwtint_t *sa_begin, bwtint_t *sa_end);
	int bwt_match_exact_alt(const bwt_t *bwt, int len, const ubyte_t *str, bwtint_t *k0, bwtint_t *l0);

	/**
	 * Extend bi-SA-interval _ik_
	 */
	void bwt_extend(const bwt_t *bwt, const bwtintv_t *ik, bwtintv_t ok[4], int is_back);

	/**
	 * Given a query _q_, collect potential SMEMs covering position _x_ and store them in _mem_.
	 * Return the end of the longest exact match starting from _x_.
	 */
	int bwt_smem1(const lbwt_t *lbwt, int len, const uint8_t *q, int x, int min_intv, bwtintv_v *mem, bwtintv_v *tmpvec[2]);
	int bwt_smem1a(const lbwt_t *lbwt, int len, const uint8_t *q, int x, int min_intv, bwtintv_v *mem, bwtintv_v *tmpvec[2]);

	int bwt_seed_strategy1(const lbwt_t *lbwt, int len, const uint8_t *q, int x, int min_len, int max_intv, bwtintv_t *mem);

#ifdef __cplusplus
}
#endif

#endif

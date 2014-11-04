/*
	CI.H
	----
*/

#ifndef CI_H_
#define CI_H_

#include <stdint.h>
#include <string.h>
#include "heap.h"

/*
	class CI_VOCAB
	--------------
*/
class CI_vocab
{
public:
	const char *term;
	void (*method)(void);
	uint64_t cf;
	uint64_t df;
public:
	static int compare(const void *a, const void *b) { return strcmp(((CI_vocab*)a)->term, ((CI_vocab*)b)->term);}
	static int compare_string(const void *a, const void *b) { return strcmp((char *)a, ((CI_vocab*)b)->term);}
};

extern uint32_t CI_unique_terms;
extern uint32_t CI_unique_documents;
extern CI_vocab CI_dictionary[];
extern const char *CI_doclist[];
extern uint16_t *CI_accumulators;
extern uint16_t **CI_accumulator_pointers;
extern uint32_t CI_top_k;
extern uint32_t CI_results_list_length;

void top_k_qsort(uint16_t **a, long long n, long long top_k);

/*
	struct ADD_RSV_COMPARE
	----------------------
	compares on value and if they are the same then compares on address - i.e. tie break on docid
*/
struct add_rsv_compare
{
int operator() (uint16_t *a, uint16_t *b) const { return *a > *b ? 1 : *a < *b ? -1 : (a > b ? 1 : a < b ? -1 : 0); }
};

extern ANT_heap<uint16_t *, add_rsv_compare> *CI_heap;

/*
	ADD_RSV()
	---------
*/
inline void add_rsv(uint32_t docid, uint16_t score)
{
uint16_t old_value;
uint16_t *which = CI_accumulators + docid;
add_rsv_compare cmp;

/*
	CI_top_k search so we maintain a heap
*/
if (CI_results_list_length < CI_top_k)
	{
	/*
		We haven't got enough to worry about the heap yet, so just plonk it in
	*/
	old_value = *which;
	*which += score;

	if (old_value == 0)
		CI_accumulator_pointers[CI_results_list_length++] = which;

	if (CI_results_list_length == CI_top_k)
		CI_heap->build_min_heap();
	}
else if (cmp(which, CI_accumulator_pointers[0]) >= 0)
	{
	/*
		We were already in the heap, so update
	*/
	*which +=score;
	CI_heap->min_update(which);
	}
else
	{
	/*
		We weren't in the heap, but we could get put there
	*/
	*which += score;
	if (cmp(which, CI_accumulator_pointers[0]) > 0)
		CI_heap->min_insert(which);
	}
}


#endif

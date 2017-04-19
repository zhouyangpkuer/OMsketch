#ifndef _OMASKETCH_STAND_H
#define _OMASKETCH_STAND_H

//FP sketch

#include "params.h"
#include "BOBHash64.h"
#include <string.h>


using namespace std;

typedef unsigned long long int uint64;

class OMASketch
{
private:
	int w_low, d_low;
	int w_high, d_high;
	int word_num_low, word_num_high;

	int MAX_CNT_LOW, MAX_CNT_HIGH;
	int word_index_size, counter_index_size;

	uint64 *word_low;
	uint64 *word_high;

	BOBHash64 * bobhash[MAX_HASH_NUM];
	
public:
	int overflow_cnt_query;
	int overflow_cnt_insert;

	OMASketch(int _w_low, int _w_high);
	void Insert(const char * str);
	int Query(const char *str);

	~OMASketch();

};

OMASketch::OMASketch(int _w_low, int _w_high)
{
	overflow_cnt_query = 0;
	overflow_cnt_insert = 0;

	//w_low is the number of counters (16 * #words) in the lower layer.
	//_w_low is the number of words in the lower layer.
	w_low = (_w_low << 4);
	word_num_low = _w_low;
	
	//w_high is the numeber of counters in the higher layer.
	w_high = (_w_high << 2);
	word_num_high = _w_high;

	d_low = 4;
	d_high = 4;
	
	//for the low and the high;
	word_index_size = 18;
	counter_index_size = 4;
	
	word_low = new uint64[word_num_low];
	word_high = new uint64[word_num_high];
	memset(word_low, 0, sizeof(uint64) * word_num_low);
	memset(word_high, 0, sizeof(uint64) * word_num_high);

	MAX_CNT_LOW = (1 << LOW_COUNTER_SIZE) - 1;
	MAX_CNT_HIGH = (1 << HIGH_COUNTER_SIZE) - 1;

	for(int i = 0; i < d_low + d_high; i++)
	{
		bobhash[i] = new BOBHash64(i + 1000);
	}
}

void OMASketch::Insert(const char * str)
{
	int high_offset[MAX_HASH_NUM];
	int low_offset[MAX_HASH_NUM];
	
	uint64 hash_value;
	int word_index[MAX_HASH_NUM];

	int min_value = 1 << 30;
	uint64 temp, temp2;

	hash_value = (bobhash[0]->run(str, strlen(str)));
	
	word_index[0] = (hash_value & 0x3FFFF) % word_num_low;
	hash_value >>= 18;
	
	low_offset[0] = (hash_value & 0xF) % 14;
	hash_value >>= 4;
	low_offset[1] = (hash_value & 0xF) % 14;
	hash_value >>= 4;
	low_offset[2] = (hash_value & 0xF) % 14;
	hash_value >>= 4;
	low_offset[3] = (hash_value & 0xF) % 14;
	hash_value >>= 4;

	// word_index[0] = (hash_value & ((1 << word_index_size) - 1)) % word_num_low;
	// hash_value >>= word_index_size;
	// for(int i = 0; i < d_low; i++)
	// {
	// 	low_offset[i] = (hash_value & 0xF) % 14;
	// 	hash_value >>= 4;
	// }

	for(int i = 0; i < d_low; i++)
	{
		temp = (word_low[word_index[0]] >> (low_offset[i] << 2)) & 0xF;
		min_value = temp < min_value ? temp : min_value;
	}
	
	if(min_value != MAX_CNT_LOW)
	{
		for(int i = 0; i < d_low; i++)
		{
			temp = (word_low[word_index[0]] >> (low_offset[i] << 2)) & 0xF;
			if(temp == min_value)
			{
				word_low[word_index[0]] += ((uint64)1 << (low_offset[i] << 2));	
			}
		}
		return;
	}




	/*********************carry into the high counters!****************************/

	temp = (low_offset[0] ^ low_offset[1]) | ((low_offset[2] ^ low_offset[3]) << 4);
	uint64 finger = (word_low[word_index[0]] >> 56) & 0x7F;

	if(finger != 0 && (finger & (temp & 0x7F)) != finger)
		word_low[word_index[0]] |= ((uint64)1 << 63);
	
	word_low[word_index[0]] |= ((temp & 0x7F) << 56);

	overflow_cnt_insert++;

	for(int i = 0; i < d_low; i++)
		word_low[word_index[0]] &= (~((uint64)0xF << (low_offset[i] << 2)));
	

	min_value = 1 << 30;

	hash_value = (bobhash[d_low]->run(str, strlen(str)));

	word_index[0] = word_index[1] = (hash_value & 0x3FFFF) % word_num_high;
	hash_value >>= 18;
	
	high_offset[0] = (hash_value & 3) % 4;
	hash_value >>= 2;
	high_offset[1] = (hash_value & 3) % 4;
	hash_value >>= 2;

	word_index[2] = word_index[3] = (hash_value & 0x3FFFF) % word_num_high;
	hash_value >>= 18;
	
	high_offset[2] = (hash_value & 3) % 4;
	hash_value >>= 2;
	high_offset[3] = (hash_value & 3) % 4;
	hash_value >>= 2;

	// word_index[0] = (hash_value & ((1 << word_index_size) - 1)) % word_num_high;
	// word_index[1] = word_index[0];

	// hash_value >>= word_index_size;
	// for(int i = 0; i < 2; i++)
	// {
	// 	high_offset[i] = (hash_value & 3) % 4;
	// 	hash_value >>= 2;
	// }

	// hash_value = (bobhash[d_low + 1]->run(str, strlen(str)));
	// word_index[2] = (hash_value & ((1 << word_index_size) - 1)) % word_num_high;
	// word_index[3] = word_index[2];
	// hash_value >>= word_index_size;
	// for(int i = 2; i < d_high; i++)
	// {
	// 	high_offset[i] = (hash_value & 3) % 4;
	// 	hash_value >>= 2;
	// }



	for(int i = 0; i < d_high; i++)
	{
		temp = (word_high[word_index[i]] >> (high_offset[i] << 4)) & 0xFFFF;
		min_value = temp < min_value ? temp : min_value;
	}
	
	for(int i = 0; i < d_high; i++)
	{
		temp = (word_high[word_index[i]] >> (high_offset[i] << 4)) & 0xFFFF;
		if(temp == min_value)
		{
			word_high[word_index[i]] += ((uint64)1 << (high_offset[i] << 4));	
		}
	}
	return;
}

int OMASketch::Query(const char *str)
{
	int high_offset[MAX_HASH_NUM];
	int low_offset[MAX_HASH_NUM];
	
	uint64 hash_value;
	int word_index[MAX_HASH_NUM];
	uint64 temp, temp2;


	int min_value = 1 << 30;
	
	hash_value = (bobhash[0]->run(str, strlen(str)));
	word_index[0] = (hash_value & 0x3FFFF) % word_num_low;
	hash_value >>= 18;
	
	low_offset[0] = (hash_value & 0xF) % 14;
	hash_value >>= 4;
	low_offset[1] = (hash_value & 0xF) % 14;
	hash_value >>= 4;
	low_offset[2] = (hash_value & 0xF) % 14;
	hash_value >>= 4;
	low_offset[3] = (hash_value & 0xF) % 14;
	hash_value >>= 4;

	// word_index[0] = (hash_value & ((1 << word_index_size) - 1)) % word_num_low;
	// hash_value >>= word_index_size;
	// for(int i = 0; i < d_low; i++)
	// {
	// 	low_offset[i] = (hash_value & 0xF) % 14;
	// 	hash_value >>= 4;
	// }

	for(int i = 0; i < d_low; i++)
	{
		temp = (word_low[word_index[0]] >> (low_offset[i] << 2)) & 0xF;

		min_value = temp < min_value ? temp : min_value;
	}


	uint64 finger = (word_low[word_index[0]] >> 56) & 0x7F;
	int flag = word_low[word_index[0]] >> 63;

	temp = (low_offset[0] ^ low_offset[1]) | ((low_offset[2] ^ low_offset[3]) << 4);

	if(flag == 1)
	{
		if((finger & (temp & 0x7F)) != finger)
			return min_value;
	}
	else
	{
		if(finger != (temp & 0x7F))
			return min_value;
	}
	

	
	overflow_cnt_query++;

	int backup = min_value;
	min_value = 1 << 30;

	
	hash_value = (bobhash[d_low]->run(str, strlen(str)));

	word_index[0] = word_index[1] = (hash_value & 0x3FFFF) % word_num_high;
	hash_value >>= 18;
	
	high_offset[0] = (hash_value & 3) % 4;
	hash_value >>= 2;
	high_offset[1] = (hash_value & 3) % 4;
	hash_value >>= 2;

	word_index[2] = word_index[3] = (hash_value & 0x3FFFF) % word_num_high;
	hash_value >>= 18;
	
	high_offset[2] = (hash_value & 3) % 4;
	hash_value >>= 2;
	high_offset[3] = (hash_value & 3) % 4;
	hash_value >>= 2;

	// word_index[0] = (hash_value & ((1 << word_index_size) - 1)) % word_num_high;
	// word_index[1] = word_index[0];
	// // word_index[2] = word_index[1];
	// hash_value >>= word_index_size;
	// for(int i = 0; i < 2; i++)
	// {
	// 	high_offset[i] = (hash_value & 3) % 4;
	// 	hash_value >>= 2;
	// }

	// hash_value = (bobhash[d_low + 1]->run(str, strlen(str)));
	// word_index[2] = (hash_value & ((1 << word_index_size) - 1)) % word_num_high;
	// word_index[3] = word_index[2];
	// hash_value >>= word_index_size;
	// for(int i = 2; i < d_high; i++)
	// {
	// 	high_offset[i] = (hash_value & 3) % 4;
	// 	hash_value >>= 2;
	// }



	for(int i = 0; i < d_high; i++)
	{
		temp2 = (word_high[word_index[i]] >> (high_offset[i] << 4)) & 0xFFFF;
		min_value = temp2 < min_value ? temp2 : min_value;
	}
	return min_value * (MAX_CNT_LOW + 1) + backup;
}

#endif //_OMASKETCH_STAND_H
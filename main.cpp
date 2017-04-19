#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <string.h>
#include <ctime>
#include <time.h>
#include <iterator>
#include <math.h>
#include <vector>

#include "CMSketch.h"
#include "CUSketch.h"
#include "CSketch.h"

#include "OMASketch_stand.h"

using namespace std;


const char * filename_FlowTraffic = "stream.dat";

char insert[N_INSERT + N_INSERT / 5][200];
char **query;

unordered_map<string, int> unmp;

#define LOW_PROPORTION 67

#define testcycles 10


int main(int argc, char** argv)
{
    if(argc == 2)
    {
        filename_FlowTraffic = argv[1];
    }

	unmp.clear();
	int val;


    double memory = 0.1;

    double coef = LOW_PROPORTION * 1.0 / 100;



    double low_memory = coef * memory;
    double high_memory = (1 - coef) * memory;

    int w = memory * 1024 * 1024 * 8.0 / COUNTER_SIZE;
    int w_low = low_memory * 1024 * 1024 * 8.0 / WORD_SIZE;
    int w_high = high_memory * 1024 * 1024 * 8.0 / WORD_SIZE;



    printf("\nlow_memory = %lf, high_memory = %lf\n", low_memory, high_memory);
    printf("w = %d, w_low = %d, w_high = %d\n", w, w_low, w_high);



    printf("\n******************************************************************************\n");
    printf("Evaluation starts!\n\n");


    CMSketch *cmsketch;
    CUSketch *cusketch;
    CSketch *csketch;

    OMASketch * omasketch;


    int packageNum = 0;

 	FILE *file_FlowTraffic = fopen(filename_FlowTraffic, "r");

    while(fgets(insert[packageNum], 200, file_FlowTraffic) != NULL)
    {
        unmp[string(insert[packageNum])]++;

        packageNum++;

        if(packageNum == N_INSERT)
            break;
    }
    fclose(file_FlowTraffic);



    query = new char*[unmp.size()];
    for(int i = 0; i < unmp.size(); ++i)
        query[i] = new char[200];


    int k = 0, max_freq = 0;
    for(unordered_map<string, int>::iterator it = unmp.begin(); it != unmp.end(); it++, k++)
    {
        strcpy(query[k], (it->first).c_str());

        int tmpVal = it->second;
        max_freq = max_freq > tmpVal ? max_freq : tmpVal;
    }
    printf("max_freq = %d\n", max_freq);



    printf("\n*************************************\n\n");



/******************************* insert ********************************/

    timespec time1, time2;
    long long resns;
   

    double throughput_cm, throughput_cu, throughput_c, throughput_oma;


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        cmsketch = new CMSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
        for(int i = 0; i < packageNum; i++)
        {
            cmsketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_cm = (double)1000.0 * testcycles * packageNum / resns;
    printf("throughput of CM (insert): %.6lf MIPS\n", throughput_cm);




    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        cusketch = new CUSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
        for(int i = 0; i < packageNum; i++)
        {
            cusketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_cu = (double)1000.0 * testcycles * packageNum / resns;
    printf("throughput of CU (insert): %.6lf MIPS\n", throughput_cu);




    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        csketch = new CSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
        for(int i = 0; i < packageNum; i++)
        {
            csketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_c = (double)1000.0 * testcycles * packageNum / resns;
    printf("throughput of C (insert): %.6lf MIPS\n", throughput_c);




    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        omasketch = new OMASketch(w_low, w_high);
        for(int i = 0; i < packageNum; i++)
        {
            omasketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_oma = (double)1000.0 * testcycles * packageNum / resns;
    printf("throughput of OM (insert): %.6lf MIPS\n", throughput_oma);


    printf("*************************************\n\n");
/***********************************************************************/


/******************************* query *********************************/    

    int res_tmp=0;

    int flow_num = unmp.size();

    int sum = 0;

    int i;

  
    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(i = 0; i < flow_num; ++i)
        {
            res_tmp = cmsketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_cm = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of CM (query): %.6lf MIPS\n", throughput_cm);
    sum += res_tmp;



    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(i = 0; i < flow_num; ++i)
        {
            res_tmp = cusketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_cu = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of CU (query): %.6lf MIPS\n", throughput_cu);
    sum += res_tmp;


 
    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(i = 0; i < flow_num; ++i)
        {
            res_tmp = csketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_c = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of C (query): %.6lf MIPS\n", throughput_c);
    sum += res_tmp;



    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(i = 0; i < flow_num; ++i)
        {
            res_tmp = omasketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_oma = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of OM (query): %.6lf MIPS\n", throughput_oma);
    sum += res_tmp;



    //avoid the over-optimize of the compiler! 
    if(sum == (1 << 30))
        return 0;

        printf("*************************************\n\n");
/***********************************************************************/



/***************************** accuracy ********************************/ 

    double re_cm = 0.0, re_cu = 0.0, re_oma = 0.0, re_a = 0.0;
    double re_cm_sum = 0.0, re_cu_sum = 0.0, re_oma_sum = 0.0, re_a_sum = 0.0;

    double ae_cm = 0.0, ae_cu = 0.0, ae_oma = 0.0, ae_a = 0.0;
    double ae_cm_sum = 0.0, ae_cu_sum = 0.0, ae_oma_sum = 0.0, ae_a_sum = 0.0;

	int val_cm = 0, val_cu = 0, val_oma = 0, val_a = 0;
    


    int cm_cor_num = 0, cu_cor_num = 0, c_cor_num = 0, oma_cor_num = 0;

    char temp[200];


    omasketch->overflow_cnt_query = 0;

    for(unordered_map<string, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
    	strcpy(temp, (it->first).c_str());
    	val = it->second;


		val_cm = cmsketch->Query(temp);
        val_cu = cusketch->Query(temp);
		val_a = csketch->Query(temp);
        val_oma = omasketch->Query(temp);
 	    

        re_cm = fabs(val_cm - val) / (val * 1.0);
        re_cu = fabs(val_cu - val) / (val * 1.0);
        re_a = fabs(val_a - val) / (val * 1.0);
		re_oma = fabs(val_oma - val) / (val * 1.0);

        ae_cm = fabs(val_cm - val);
        ae_cu = fabs(val_cu - val);
        ae_a = fabs(val_a - val);
        ae_oma = fabs(val_oma - val);


        re_cm_sum += re_cm;
        re_cu_sum += re_cu;
        re_a_sum += re_a;
        re_oma_sum += re_oma;

        ae_cm_sum += ae_cm;
        ae_cu_sum += ae_cu;
        ae_a_sum += ae_a;
        ae_oma_sum += ae_oma;


        if(val == val_cm)
            cm_cor_num++;
        if(val == val_cu)
            cu_cor_num++;
        if(val == val_a)
            c_cor_num++;
        if(val == val_oma)
            oma_cor_num++;

    }

    
    double a = unmp.size() * 1.0;
    double b = packageNum;

    printf("cm\tcorrect rate = %lf\n", cm_cor_num * 1.0 / a);
    printf("cu\tcorrect rate = %lf\n", cu_cor_num * 1.0 / a);
    printf("c\tcorrect rate = %lf\n", c_cor_num * 1.0 / a);
    printf("om\tcorrect rate = %lf\n", oma_cor_num * 1.0 / a);
    printf("-------------------------------------\n\n");



    printf("are_cm = %lf\n", re_cm_sum / a);
    printf("are_cu = %lf\n", re_cu_sum / a);
    printf("are_c = %lf\n", re_a_sum / a);
    printf("are_om = %lf\n", re_oma_sum / a);
    printf("-------------------------------------\n\n");



    printf("aae_cm = %lf\n", ae_cm_sum / a);
    printf("aae_cu = %lf\n", ae_cu_sum / a);
    printf("aae_c = %lf\n", ae_a_sum / a);
    printf("aae_om = %lf\n", ae_oma_sum / a);
    printf("-------------------------------------\n\n");



    printf("om:\t(insert) average number of memory accesses is %lf\n", 1 + omasketch->overflow_cnt_insert * 1.0 / packageNum * 2);
    printf("om:\t(query) average number of memory accesses is %lf\n", 1 + omasketch->overflow_cnt_query * 1.0 / a * 2);
    printf("-------------------------------------\n\n");




    printf("Evaluation Ends!\n");
    printf("******************************************************************************\n\n");

    return 0;
}
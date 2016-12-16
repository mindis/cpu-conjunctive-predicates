#####input#####
#run the code with the command:::::make bit_width=BITWIDTH
############### -g

CC=icc
OPT=  -O3 -pthread -fPIC -fopenmp -std=c++11 -lrt
#-std=c++0x  -std=gnu++11 
COMMONTOOL = thread_tool.o rand_tool.o file_tool.o cpu_mapping.o huge_page.o memory_tool.o libpcm_2_11.a


all: impact_of_prefetcher_tlb.x column_compare_with_literal_test.x 2_column_compare_with_literal_test.x 3_column_compare_with_literal_test.x 4_column_compare_with_literal_test.x 5_column_compare_with_literal_test.x
 
#####################common functions##########################################
rand_tool.o: rand_tool.cpp   
	$(CC) $(OPT) -c rand_tool.cpp
thread_tool.o: thread_tool.cpp avx-utility.h types.h  
	$(CC) $(OPT) -c thread_tool.cpp
file_tool.o: file_tool.cpp   
	$(CC) $(OPT) -c file_tool.cpp
huge_page.o: huge_page.cpp 
	$(CC) $(OPT) -c huge_page.cpp
cpu_mapping.o: cpu_mapping.cpp 
	$(CC) $(OPT) -c cpu_mapping.cpp
#perf_counters.o: perf_counters.cpp   
#	$(CC) $(OPT) -c perf_counters.cpp
memory_tool.o: memory_tool.cpp   
	$(CC) $(OPT) -c memory_tool.cpp
#########################end of common functions################################


# check the impact of hardware prefetcher, branch and PCM. Also the one column case.
impact_of_prefetcher_tlb.x: impact_of_prefetcher_tlb.cpp avx-utility.h types.h $(COMMONTOOL) common_tool.h 
	$(CC) $(OPT) impact_of_prefetcher_tlb.cpp -o impact_of_prefetcher_tlb.x $(COMMONTOOL)
#####################end of motivations##########################################	
 
# check the case with two columns. 2_column_compare_with_literal.cpp
column_compare_with_literal.o: column_compare_with_literal.cpp types.h avx-utility.h 
	$(CC) $(OPT) -c column_compare_with_literal.cpp
column_compare_with_literal_test.x: column_compare_with_literal_test.cpp avx-utility.h types.h $(COMMONTOOL) common_tool.h column_compare_with_literal.o 
	$(CC) $(OPT) column_compare_with_literal_test.cpp -o column_compare_with_literal_test.x $(COMMONTOOL) column_compare_with_literal.o
	

# check the case with two columns. 2_column_compare_with_literal.cpp
2_column_compare_with_literal.o: 2_column_compare_with_literal.cpp types.h avx-utility.h 
	$(CC) $(OPT) -c 2_column_compare_with_literal.cpp
2_column_compare_with_literal_test.x: 2_column_compare_with_literal_test.cpp avx-utility.h types.h $(COMMONTOOL) common_tool.h 2_column_compare_with_literal.o 
	$(CC) $(OPT) 2_column_compare_with_literal_test.cpp -o 2_column_compare_with_literal_test.x $(COMMONTOOL) 2_column_compare_with_literal.o


# check the case with three columns. 3_column_compare_with_literal.cpp
3_column_compare_with_literal.o: 3_column_compare_with_literal.cpp types.h avx-utility.h 
	$(CC) $(OPT) -c 3_column_compare_with_literal.cpp
3_column_compare_with_literal_test.x: 3_column_compare_with_literal_test.cpp avx-utility-128.h types_simd.h $(COMMONTOOL) common_tool.h 3_column_compare_with_literal.o 
	$(CC) $(OPT) 3_column_compare_with_literal_test.cpp -o 3_column_compare_with_literal_test.x $(COMMONTOOL) 3_column_compare_with_literal.o
#####################end of motivations##########################################	


# check the case with four columns. 4_column_compare_with_literal.cpp
4_column_compare_with_literal.o: 4_column_compare_with_literal.cpp types.h avx-utility.h 
	$(CC) $(OPT) -c 4_column_compare_with_literal.cpp
4_column_compare_with_literal_test.x: 4_column_compare_with_literal_test.cpp avx-utility-128.h types_simd.h $(COMMONTOOL) common_tool.h 4_column_compare_with_literal.o 
	$(CC) $(OPT) 4_column_compare_with_literal_test.cpp -o 4_column_compare_with_literal_test.x $(COMMONTOOL) 4_column_compare_with_literal.o
###############################################################	

# check the case with four columns. 5_column_compare_with_literal.cpp
5_column_compare_with_literal.o: 5_column_compare_with_literal.cpp types.h avx-utility.h 
	$(CC) $(OPT) -c 5_column_compare_with_literal.cpp
5_column_compare_with_literal_test.x: 5_column_compare_with_literal_test.cpp avx-utility.h types_simd.h $(COMMONTOOL) common_tool.h 5_column_compare_with_literal.o 
	$(CC) $(OPT) 5_column_compare_with_literal_test.cpp -o 5_column_compare_with_literal_test.x $(COMMONTOOL) 5_column_compare_with_literal.o
###############################################################	

	
clean_test:
	rm -rf *.o	 
	
clean:
	rm -rf *.o *.x
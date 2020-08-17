/*
Pledge : I have abided by the Wheaton College Honor Code in this work.
Name   : Timothy McCormack
Summary: This program simulates an American election, it uses the POSIX library
to execute the random number generation in parallel. The executable(./out) takes
no arguments, and the output of the program can be found in stdout. 
The election results consists of:
	1. The breakdown of the election per state (winner, popular vote)
	2. The final winner of the election and the final electoral and popular vote
	3. The total cost (dollars) of the election
	4. The amound of time (days) of the election


*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>  //POSIX API
#include <sys/time.h> //includes timing structures
#include "elect.h"
#define NUMTHREADS 1






int main(int argc, char ** argv){
	int numberOfStates = 51; //50 states + DC
	int agency = 9;			 //9 == ID for IOWA	
	float turn_out = 0.5;    //how likely the registered voters will vote
	struct timeval start, end; //for timing

	//filenames that are used:
	char* filename = "Resources/electorals.txt";
	char* dataFile = "Resources/data.csv";
	char* zoneFile = "Resources/zone.txt";

	//file pointers ...
	FILE * fp_elect;
	FILE * fp_data;
	FILE *fp_zone;





	//create a Country structure to aggregate the data at the end
	struct Country USA;
	struct Country usaPtr;

	//seed random time generator
	srand(time(NULL));

	//allocate an array of fifty one states using malloc (will be freed)
	struct State* states;
	states = (struct State*) malloc(numberOfStates*sizeof(struct State));

	//set the attributes for the country structure
	usaPtr = USA;
	strcpy(usaPtr.countryName, "Unites States of America");

	//assign each state a unique identifier (ID)
	assignID(states, numberOfStates);

	//parse the electorals.txt file
    parseElectorals(fp_elect, numberOfStates, filename, states);

    //parse the data.csv file
	csvParser(fp_data, numberOfStates, states, dataFile);

	//parse the zones.txt file
	zoneParser(fp_zone, numberOfStates, states, zoneFile);
	
	
	//START TIMER
	gettimeofday(&start, NULL); //start of measurement

	
	//also calculate the turn out and the zoning distance
	actualVotersCalculator(states, turn_out, numberOfStates, agency);

	//calculate the mailing costs with mail().
	for(short q = 0; q < numberOfStates; q++){
		mail(states, q);
	}


	//now it is time to simulate voting
	//headers for stdout
	printf("\n---------- STATE ELECTIONS ----------\n");
	printf("State: winner (popular votes)\n");

	//thread id's for all threads
	pthread_t tid[NUMTHREADS];

	//each thread has struct for params
	struct threadData data[NUMTHREADS];
	int i;	//iterator

	//work per thread
    int taskPerThread = (numberOfStates + NUMTHREADS -1)/NUMTHREADS;

    //preparing parameters for each thread
    for(int i = 0; i < NUMTHREADS; i++){
        data[i].start = i*taskPerThread;
        data[i].stop = (i+1) *taskPerThread;
        data[i].stateArray = states;
    }
    //the last stop value will run over the array without the line below
    data[NUMTHREADS-1].stop = numberOfStates;

	//Launch the threads    
    for(i = 0; i < NUMTHREADS; i++){
        pthread_create(&tid[i], NULL, vote, &data[i]);
    }
    //Wait for the threads to execute
    for(i = 0; i < NUMTHREADS; i++){
        pthread_join(tid[i], NULL);
    }
	

	int red = 0; 			//red electoral votes
	int blue = 0;			//blue electoral votes
	int popVoteRed = 0;	    //popular votes for red
	int popVoteBlue = 0;    //popular votes for blue
	//ensure that the total cost is reset
	usaPtr.totalCost = 0;

	//aggregate all metrics (electorals, popular votes, time , cost) and declare final winner
	for(short s = 0; s < numberOfStates; s++){
		aggregateVotes(states,s, usaPtr, &red, &blue, numberOfStates, &usaPtr.totalTime, 
			&usaPtr.totalCost, &popVoteRed, &popVoteBlue);
	}

	//END
	gettimeofday(&end, NULL); //end

	//every malloc needs a free()
	free(states);

	//time calculation
  	unsigned long secs = end.tv_sec - start.tv_sec;
  	unsigned long usecs = end.tv_usec - start.tv_usec;
  	long total = ((((long)(secs))*1000000) + ((long)(usecs)))/1000; /*total time in milliseconds*/
  	printf("Runtime: %ld ms\n", total);


	return 0;

}
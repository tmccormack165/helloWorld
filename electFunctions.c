#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>  //POSIX API
#include <sys/time.h> //includes timing structures
#include "elect.h"



void mail(struct State* states, int index){
	if(states[index].distanceFromCenter == 0){
		states[index].mailTime = 3;
		states[index].mailCost = 5;
	}
	else if(states[index].distanceFromCenter == 1){
		states[index].mailTime = 4;
		states[index].mailCost = 7;
	}
	else if(states[index].distanceFromCenter == 2){
		states[index].mailTime = 5;
		states[index].mailCost = 10;
	}
	else if(states[index].distanceFromCenter == 3){
		states[index].mailTime = 7;
		states[index].mailCost = 12;
	}
	else if(states[index].distanceFromCenter == 4){
		states[index].mailTime = 8;
		states[index].mailCost = 15;
	}
	else if(states[index].distanceFromCenter == 5){
		states[index].mailTime = 9;
		states[index].mailCost = 20;
	}
	else if(states[index].distanceFromCenter >= 6){
		states[index].mailTime = 10;
		states[index].mailCost = 40;
	}

}

//PRE: Country *usaPtr.winner == NULL (the final winner has not been calculated)
//POST: Country *usaPtr.winner == 1 || Country *usaPtr.winner == 0
void aggregateVotes(struct State*states, int index, struct Country usaPtr, 
	int *rdPtr, int *bluPtr,int numberOfStates, int *totalTime, long *totalCost, int* redPop, int* bluePop){

	if(states[index].winner == 'r'){
		*rdPtr += states[index].electoralVotes; //collect electoral votes
	}
	else{
		*bluPtr += states[index].electoralVotes; //collect electoral votes
	}

	//calculate the incoming mail cost
	*totalCost += states[index].mailCost;

	//account for outgoing mail (constant intrazone rate)
	*totalCost += 5*states[index].amountRegistered;	//amountRegistered NOT actual voters

	//update the popular vote
	*redPop += states[index].red;
	*bluePop += states[index].blue; 

	//output results to stdout
	if(index == (numberOfStates - 1)){
		usaPtr.redElectorals = *rdPtr;
		usaPtr.blueElectorals = *bluPtr;
		if(usaPtr.redElectorals >= usaPtr.blueElectorals){
			usaPtr.winner = 'r';
		}
		if(usaPtr.redElectorals < usaPtr.blueElectorals){
			usaPtr.winner ='b';
		}
		
		//account for outgoing mail (intrazone)
		printf("\n\n\nTOTAL COST: $%ld.00 \nTOTAL TIME: 11 days\n", *totalCost);
		if(usaPtr.winner == 'r'){
			printf("WINNER: Republican (RED)\n");
		}
		else{
			printf("WINNER: Democrat (BLUE)\n");
		}
		printf("\n---------- ELECTORAL VOTE ----------\n");
		printf("%d RED, %d BLUE\n\n", usaPtr.redElectorals, usaPtr.blueElectorals);
		printf("---------- POPULAR VOTE  -----------\n");
		printf("%d RED, %d BLUE\n",*redPop, *bluePop);


	}



}

void* vote(void* td){
	//cast the void pointer to a threadData pointer
	struct threadData* data = (struct threadData *) td;
	int start = data -> start;			
	int stop = data -> stop;
	struct State* stateArrayT = data->stateArray;
	int cap;	//cap == the amount of actual voters in a state

	//each thread collects the votes for seven threads (except NTHREADS-1)
	for(short i = start; i < stop; i++ ){
		cap = stateArrayT[i].actualVoters;
		int vote;
		for(int j = 0; j < cap; j++){
			vote = rand() % 2;			//generate the vote
			if(vote == 0)
				stateArrayT[i].red++;	//update red
			else
				stateArrayT[i].blue++;	//update blue
		}
		//outout stateName, popular vote to stdout, and calculate the winner in each state
		if(stateArrayT[i].red >= stateArrayT[i].blue){
			stateArrayT[i].winner = 'r';
			printf("%s: red (red = %d, blue = %d)\n", stateArrayT[i].stateName, stateArrayT[i].red,stateArrayT[i].blue);
		}
		else{
			stateArrayT[i].winner = 'w';
			printf("%s: blue  (red = %d, blue = %d)\n", stateArrayT[i].stateName, stateArrayT[i].red,stateArrayT[i].blue);

		}
	}
	
	//remember the thread functions return void pointers...
	return NULL;
}

void assignID(struct State* states,int numberOfStates){
	for(short i = 0; i < numberOfStates; i++)
		states[i].stateID = i;
}

void parseElectorals(FILE* fp, int numberOfStates, char* filename[25], struct State* states){
	char delimeter;	//store delimeter value (throwaway)
	char n[2];	//store electoral votes before typecast

	fp = fopen(filename, "r");
	if(fp == NULL){
		perror("ERROR: 'electorals.txt' missing in directory \n");
		//return 1;
	}

	for(short j = 0; j < numberOfStates; j++){
		fscanf(fp, "%s", n);
		states[j].electoralVotes = atoi(&n);
	}

}

void csvParser(FILE *fp_data, int numberOfStates, struct State* states, char* filename[25]){
	char c; // c = fgetc() reusable
	int k,l; //k = iterator per word, l = iterator per step

	fp_data = fopen(filename, "r");

	if(fp_data == NULL){
		perror("ERROR: 'data.csv' missing in directory \n");
		//r/eturn 1;
	}

	char *throw = (char*) malloc(100 *sizeof(char));
	//TODO dynamically allocate the throw string
	//throw away colunmn headers
	fscanf(fp_data, "%s", throw);
	free(throw);


	//csv parser
	for(short l = 0; l < numberOfStates; l++){
		k = 0;
		while(c != ','){
			c = fgetc(fp_data);
			
			if(c != ','){
				states[l].stateName[k] = c;
				k++;
			}
		}

		k = 0;
		do{
			c = fgetc(fp_data);
			if(c != ','){
				//tmp[k] = c;
				states[l].practiceRegister[k] = c;
				k++;
			}

		}while(c != ',');

		k = 0;

		do{
			c = fgetc(fp_data);
			if(c != ','){

				states[l].practicePerc[k] = c;
				
				k++;
			}
		}while(c != ',');

		k = 0;
		do{
			c = fgetc(fp_data);
			if(c != ',' && c != '\n'){
				states[l].practicePop[k] = c;			
				k++;
			}
		}while(c != '\n');
		//atoi/atof conversions because the .csv file is read as an ascii file
		states[l].amountRegistered = atoi(&states[l].practiceRegister);
		states[l].registeredPerc = atof(&states[l].practicePerc);
		states[l].population = atoi(states[l].practicePop);		

	}


}
void zoneParser(FILE* fp_zone, int  numberOfStates, struct State* states, char* zoneFile[25]){
	//final file to open contains the zoning data for each state
	int m;
	char z[2]; //zone char
	fp_zone = fopen(zoneFile, "r");
	if(fp_zone == NULL){
		printf("ERROR: zone.txt is nonexistent in directory");
	}

	for(m = 0; m < numberOfStates; m++){
		fscanf(fp_zone, "%s", z);
		states[m].zone = atoi(&z);

	}
}

void actualVotersCalculator(struct State* states, float turn_out, int numberOfStates, int agency){
	
	for(short o = 0; o < numberOfStates; o++){
		states[o].isCenter = 0;
		if(o== agency){
			states[o].isCenter = 1;
			printf("%s IS THE AGENCY \n", states[o].stateName);
		}
		float result = turn_out * states[o].amountRegistered;
		states[o].actualVoters = (int)result;
		printf("%d actual voters in %s \n", states[o].actualVoters, states[o].stateName);
	}
	//calculate the zoning difference from the agency for each state
	for(short p = 0; p < numberOfStates; p++){
		states[p].distanceFromCenter = abs(states[p].zone - states[agency].zone);
	}

}
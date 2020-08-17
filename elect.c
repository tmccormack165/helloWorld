#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct State{
	int stateID;
	int electoralVotes;
	int zone;
	int distanceFromCenter;
	int isCenter;
	long population;
	long amountRegistered;
	float registeredPerc;
	char stateName[20];
	int actualVoters;

	char practiceRegister[20]; //used for ascii conversion
	char practicePerc[20];     //used for ascii conversion   
	char practicePop[20];      //used for ascii conversion

	int mailCost;
	int mailTime;

	int red;
	int blue;

	char winner;
};

struct Country{
	long totalCost;
	int totalTime;

	int redElectorals;
	int blueElectorals;

	int redPopVote;
	int bluePopVote;

	char countryName[50];

	char winner;
};

//posix only allows passing of one argument in pthread create
struct threadData{
	int start, stop;
	int* stateArray;

};

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

void vote(struct State* states, int index){
	//int n; // used 
	int cap = states[index].actualVoters; 
	int vote;
	struct timeval start, end; //for timing


	//printf("%d actual voters \n", states[index].actualVoters);
	//allocate ten thousand 1's or 0's
	for(int j = 0; j < cap; j++){
		vote = rand() % 2;
		if(vote == 0){
			states[index].red++;
			//printf("%d\n", states[index].red);
		}
		else{
			states[index].blue++;
		}
	}

	if(states[index].red >= states[index].blue){
		states[index].winner = 'r';
	}
	else{
		states[index].winner = 'b';
	}
}

void aggregateVotes(struct State*states, int index, struct Country usaPtr, 
	int *rdPtr, int *bluPtr,int numberOfStates, int *totalTime, int *totalCost, int* redPop, int* bluePop){

	if(states[index].winner == 'r'){
		*rdPtr += states[index].electoralVotes;
	}
	else{
		*bluPtr += states[index].electoralVotes;
	}

	//incoming mail
	//*totalTime += states[index].mailTime;
	*totalCost += states[index].mailCost;

	//account for outgoing mail
	*totalCost += 5*states[index].amountRegistered;

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

int main(int argc, char ** argv){
	//in our simulation D.C. has electoral votes
	int numberOfStates = 51;
	int agency = 9;
	int intraZoneTime = 153; //3 dollars X registered Voters
	int intraZoneCost = 255;  
	float turn_out = 0.5;
	struct timeval start, end; //for timing


	srand(time(NULL));

	//in my simulation the turn-out rate is proportional to the amount of registered voters compared
	//to the state population


	//allocate an array of fifty states...
	struct State* states;
	states = (struct State*) malloc(numberOfStates*sizeof(struct State));

	struct Country USA;
	struct Country usaPtr;

	usaPtr = USA;
	strcpy(usaPtr.countryName, "Unites States of America");

	//assign each state a unique identifier
	for(short i = 0; i < numberOfStates; i++){
		states[i].stateID = i;
		//printf("%d \n", states[i].stateID);
	}

	//open electorals.txt
	//State: Number of votes
	FILE * fp_elect;
	fp_elect = fopen("Resources/electorals.txt", "r");

	if(fp_elect == NULL){
		printf("ERROR: 'electorals.txt' missing in directory \n");
		return 1;
	}
	char delimeter;	//store delimeter value (throwaway)
	char n[2];	//store electoral votes before typecast

	for(short j = 0; j < numberOfStates; j++){
		fscanf(fp_elect, "%s", n);
		states[j].electoralVotes = atoi(&n);
	}

	//open up data.csv (State, totalRegistered, registeredPerc, Pop)
	FILE * fp_data;
	fp_data = fopen("Resources/data.csv", "r");

	if(fp_elect == NULL){
		printf("ERROR: 'data.csv' missing in directory \n");
		return 1;
	}
	char *throw = (char*) malloc(100 *sizeof(char));
	//TODO dynamically allocate the throw string
	//throw away colunmn headers
	fscanf(fp_data, "%s", throw);
	free(throw);

	//state
	char c; // c = fgetc() reusable
	int k,l; //k = iterator per word, l = iterator per step

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
		
		states[l].amountRegistered = atoi(&states[l].practiceRegister);
		states[l].registeredPerc = atof(&states[l].practicePerc);
		states[l].population = atoi(states[l].practicePop);		

	}


	//final file to open contains the zoning data
	int m;
	char z[2]; //zone char
	FILE *fp_zone;
	fp_zone = fopen("Resources/zone.txt", "r");
	if(fp_zone == NULL){
		printf("ERROR: zone.txt is nonexistent in directory");
	}

	gettimeofday(&start, NULL); //start of measurement
	for(m = 0; m < numberOfStates; m++){
		fscanf(fp_zone, "%s", z);
		states[m].zone = atoi(&z);

	}


	//now we want one state to be the voting agency, that state is Iowa in zone 3
	//also calculate actual voters
	for(short o = 0; o < numberOfStates; o++){
		states[o].isCenter = 0;
		if(o== agency)
			states[o].isCenter = 1;
		float result = turn_out * states[o].amountRegistered;
		states[o].actualVoters = (long)result;
	}


	//calculate the zoning difference from the agency
	for(short p = 0; p < numberOfStates; p++){
		states[p].distanceFromCenter = abs(states[p].zone - states[agency].zone);
	}

	//printf("%s has a distance of %d \n", states[40].stateName, states[40].distanceFromCenter);



	//now it is time to calculate the mailing costs ...
	for(short q = 0; q < numberOfStates; q++){
		mail(states, q);
	}
	//now it is time to simulate voting
	printf("\n---------- STATE ELECTIONS ----------\n");
	printf("Stae: winner (popular votes)\n");
	for(short r = 0; r <numberOfStates; r++){
		vote(states,r);
		//printf("Winner in %s: %c \n", states[r].stateName, states[r].winner);
		if(states[r].winner == 'r'){
			printf("%s: red (red = %d, blue = %d)\n", states[r].stateName, states[r].red,states[r].blue);
		}
		else{
			printf("%s: blue (red = %d, blue = %d)\n", states[r].stateName, states[r].red,states[r].blue);
		}
	}

	int red = 0;
	int blue = 0;
	int popVoteRed = 0;
	int popVoteBlue = 0;
	//ensure that the total cost is reset
	usaPtr.totalCost = 0;

	for(short s = 0; s < numberOfStates; s++){
		aggregateVotes(states,s, usaPtr, &red, &blue, numberOfStates, &usaPtr.totalTime, 
			&usaPtr.totalCost, &popVoteRed, &popVoteBlue);
	}
	

	gettimeofday(&end, NULL); //end

	free(states);

	//time calculation
  	unsigned long secs = end.tv_sec - start.tv_sec;
  	unsigned long usecs = end.tv_usec - start.tv_usec;
  	long total = ((((long)(secs))*1000000) + ((long)(usecs)))/1000; /*total time in milliseconds*/
  	printf("Runtime: %ld ms\n", total);


	return 0;

}
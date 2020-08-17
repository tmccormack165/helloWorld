
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>  //POSIX API
#include <sys/time.h> //includes timing structures


//structures
struct State{
	int stateID;			//unique ID
	int electoralVotes;		//amount electoral votes
	int zone;				//geographic zone
	int distanceFromCenter; //distance from agency (Iowa: zone 3)
	int isCenter;			//is this state the central agency?
	long population;		//state population
	long amountRegistered;	//registered voters in the state
	float registeredPerc;	//percent of population registered
	char stateName[20];		//name of state
	int actualVoters;		//how many people actually voted

	char practiceRegister[20]; //used for ascii conversion
	char practicePerc[20];     //used for ascii conversion   
	char practicePop[20];      //used for ascii conversion

	int mailCost;				//cost for the mail coming from Iowa
	int mailTime;				//the time it takes to get mail from Iowa

	int red;					//the number of red votes
	int blue;					//the number of blue votes

	char winner;				//the winning party
};

//a structure to keep track of aggregating metrics
struct Country{
	long totalCost;		//the total cost of the election
	int totalTime;		//the total time for the election (MAX(mailTime) + 3)

	int redElectorals;	//the number of electoral votes for red (republican)
	int blueElectorals; //the number of electoral votes for blue (democrat)

	int redPopVote;		//the popular votes for red (republican)
	int bluePopVote;    //the popular votes for blue (democrat)
	char countryName[50];//the name of the country

	char winner;		 //the final winner
};

//a thread to keep track of data for each thread
struct threadData{
	int start, stop;			//the partitioning values for which states belong
	struct State* stateArray;	//a pointer to gain access to the state data structure

};
//PRE: states[index].mailTime == NULL && states[index].mailCost == NULL
//POST: states[index].mailTime == {3,4,5,7,8,9} && states[index].mailCost == {5,7,10,12,15,20,40}
void mail(struct State* states, int index);

void aggregateVotes(struct State*states, int index, struct Country usaPtr, 
	int *rdPtr, int *bluPtr,int numberOfStates, int *totalTime, long *totalCost, int* redPop, int* bluePop);

void* vote(void* td);

void assignID(struct State* states,int numberOfStates);

void parseElectorals(FILE* fp, int numberOfStates, char* filename[25], struct State* states);

void csvParser(FILE *fp_data, int numberOfStates, struct State* states, char* filename[25]);

void zoneParser(FILE* fp_zone, int  numberOfStates, struct State* states, char* zoneFile[25]);


void actualVotersCalculator(struct State* states, float turn_out, int numberOfStates, int agency);
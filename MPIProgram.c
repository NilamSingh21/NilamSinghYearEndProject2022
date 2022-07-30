/*
 * MpiProgram.c
 *
 *  Created on: 06-Apr-2022
 *      Author: Nilam Singh
 *
 *  MPI defines an environment for managing distributed computations on
 *  processors with independent memory connected by networking mechanisms.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "mpi.h"

//allow constant values to be declared for use throughout the code.
//It's safe to presume that no text or pattern file will be larger than 30 MB.
#define MaxTxtPattern 30
#define BufferSize 100000
#define MASTER 0
#define JOBSIZE 768

//read files

void readFile(FILE *file, char **data, int *length)
{
	int chr ,allocatedLength=0,resultLength = 0;
	char *result=NULL;

	chr = fgetc(file);
	while (chr >= 0)
	{
		resultLength++;
		if (resultLength > allocatedLength)
		{
			allocatedLength += 10000;
			result = (char *)realloc(result, sizeof(char) * allocatedLength);
			if (result == NULL)
				{
						fprintf (stderr, "Error - unable to allocate required memory\n");  //used to print the error on the output screen.
						exit (0);
				}
		}
		result[resultLength-1] = chr;
		chr = fgetc(file);		//taking input single character at a time
	}
	*data = result;
	*length = resultLength;
}

int readTextFile(char **txtData, int *txtLength)
{
	for (int i = 0; i < MaxTxtPattern; i++)
	{
		FILE *file;
		char fileName[1000];

		sprintf (fileName, "TextFile%d.txt",i); //store output on char fileName

		file = fopen(fileName, "r");
		if (file == NULL)
			return i;
		readFile (file, &txtData[i], &txtLength[i]);
		fclose (file);
	}
	return MaxTxtPattern;
}

int readPatternFile(char **patternData, int *patternLength)
{
	for (int i = 0; i < MaxTxtPattern; i++)
	{
		FILE *file;
		char fileName[1000];

		sprintf (fileName, "PatternFile%d.txt",i);

		file = fopen (fileName, "r");
		if (file == NULL)
			return i;
		readFile (file, &patternData[i], &patternLength[i]);
		fclose (file);
	}
	return MaxTxtPattern;
}

int readInputFile(int inputData[][3])
{
	FILE *file;
	char fileName[1000];

	//A InputFile file, which specifies the mode, text to use, and pattern to use,
	//specifies the pattern and text combinations to be tested.

	sprintf (fileName, "Master.txt");

	file = fopen (fileName, "r");
	if (file == NULL)
		return 0;

	int c,i = 0;

	while ((c = fscanf(file, "%d %d %d", &inputData[i][0], &inputData[i][1], &inputData[i][2])) != EOF)
	{
			i++;
	}

	fclose (file);
	return i;
}

void OutputFile(char buffer[])
{
	FILE *file;
	char fileName[1000];

	sprintf (fileName, "OutputFile_MPI.txt");

	file = fopen(fileName, "a+");  //Opens a file for reading and appending
	if (file == NULL)
		return;
	fprintf(file,"%s", buffer);
	fclose(file);

	buffer[0] = '\0';
}

// This should only be called by the master.
void writeBuffer(char buffer[], int txtIndex, int patternIndex, int matchPoint)
{
	if (strlen(buffer) > BufferSize - 30)
		OutputFile(buffer);

	sprintf(buffer+strlen(buffer), "%d %d %d\n", txtIndex, patternIndex, matchPoint);
}

// For serachLength, find a pattern at the provided text position and the positions after it.
int searchForPatternAtPosition(char txt[], char pattern[], int txtIndex, int textLength, int patternLength, int serachLength, int result[JOBSIZE])
{
	int i,patternFound = 0;

	for (i = 0; i < serachLength; i++)
	{
		if (textLength - (txtIndex + i) < patternLength)
			break;
		int j = txtIndex + i;
		int k = 0;

		while (txt[j] == pattern[k] && k < patternLength)
		{
			j++;
			k++;
		}

		if (k == patternLength)
		{
			result[patternFound++] = txtIndex + i;
		}
	}

	if (patternFound < JOBSIZE)
		result[patternFound] = -1; // This is used to tell master where the array results terminate.

	return patternFound;
}

void masterModel(int numberOfTask, int numberOfTests, int inputData[][3], int* txtLength, int* patternLength, char* buffer)
{
	// Loop through each test
	for (int testCountNo = 0; testCountNo < numberOfTests; testCountNo++)
	{
		int mode = inputData[testCountNo][0];
		int txtIndex = inputData[testCountNo][1];
		int patternIndex = inputData[testCountNo][2];

		// It won't be found if the text is shorter than the pattern.
		if (txtLength[txtIndex] < patternLength[patternIndex])
		{
			sprintf(buffer+strlen(buffer), "%d %d %d\n", txtIndex, patternIndex, -1);
			continue;
		}
		int found = -1;
		int activeThreads = 0;

		int searchData[4] = {txtIndex, patternIndex, 0, JOBSIZE}; // {text#, pattern#, text index to check, length of text to check}

		// Send a job to all the threads to start off with
		//Blocking send.A message containing count elements of the specified data type starting at address buffer is to be sent to the process with the identifier test.
		//The message is associated with an envelope comprising the specified tag, the source process’s identifier and the specified communicator.
		for (int i = 1; i < numberOfTask; i++)
		{
			if (searchData[2] <= txtLength[txtIndex] - patternLength[patternIndex]) // Unless the text is shorter than the number of threads
			{
				//MPI_COMM_WORLD is a communicator
				MPI_Send(&searchData, 4, MPI_INT, i, 0, MPI_COMM_WORLD); // 1
				searchData[2]+=searchData[3];
				activeThreads++;
			}
			else
				break;
		}
		int complete = 0;

		// Loop over each character in the text until the test is complete
		while (!complete && activeThreads > 0)
		{
			int results[JOBSIZE];
			MPI_Status s;
			MPI_Recv(&results, JOBSIZE, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &s); // 2
			activeThreads--;

			// If a thread discover a pattern occurrence
			if (results[0] != -1)
			{
				// Mode 0 - find first instance
				if (mode == 0)
				{
					found = results[0];
					// Wait for remaining active threads to return, in case they find an earlier result
					while (activeThreads > 0)
					{
						MPI_Recv(&results, JOBSIZE, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &s); // 2

						if (results[0] != -1 )
						{
							int minVal = results[0];
							if (minVal < found)
								found = minVal;
						}
						activeThreads--;
					}

					// save the outcome,then end this test by setting done
					sprintf(buffer+strlen(buffer), "%d %d %d\n", txtIndex, patternIndex, found);
					complete = 1;
				}
				// Mode 1 - find all instances
				else
				{
					// save the outcome and set found
					found = results[0];
					int i = 0;
					while (i < JOBSIZE && results[i] != -1)
						sprintf(buffer+strlen(buffer), "%d %d %d\n", txtIndex, patternIndex, results[i++]);
				}
			}
			// Thread failed to detect a pattern in mode 0 or there is still text that needs to be analysed in mode 0/1
			if (!complete && searchData[2] <= txtLength[txtIndex] - patternLength[patternIndex])
			{
				MPI_Send(&searchData, 4, MPI_INT, s.MPI_SOURCE, 0, MPI_COMM_WORLD); // 1
				searchData[2] += JOBSIZE;
				activeThreads++;
			}

		}
		// A pattern is not present if no records are identified
		if (found == -1)
		{
			sprintf(buffer+strlen(buffer), "%d %d %d\n", txtIndex, patternIndex, -1);
		}
	}

	// When all of the tests are finished, instruct the slaves to come to a halt.
	int complete = 1;
	MPI_Request request;
	for (int j = 1; j < numberOfTask; j++)
	{
		MPI_Isend(&complete, 1, MPI_INT, j, 1, MPI_COMM_WORLD, &request); // 3
	}
}

void slaveModel(int rank, char** txtData, char** patternData, int* txtLength, int* patternLength)
{
	// create the receive for the Master flag indicating the end
	int complete = 0,completeFlag = 0;
	MPI_Request request;
	MPI_Status status;
	MPI_Irecv(&complete, 1, MPI_INT, MASTER, 1, MPI_COMM_WORLD, &request); // 3

	while (!completeFlag)
	{
		// wait for new work from the master
		MPI_Request r;
		MPI_Status s;
		int newWorkFlag = 0,checkData[4];
		MPI_Irecv(&checkData, 4, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &r); // 1
		while(!newWorkFlag)
		{
			MPI_Test(&request, &completeFlag, &status); // 3
			if (completeFlag)
				return;

			MPI_Test(&r, &newWorkFlag, &s); // 1
		}

		int results[JOBSIZE];

		MPI_Send(&results, JOBSIZE, MPI_INT, MASTER, 0, MPI_COMM_WORLD); // 2

		// verify whether master indicates we are finished
		MPI_Test(&request, &completeFlag, &status); // 3
	}
}

int main(int argc, char **argv)
{
	// For each thread, read in the text pattern and input (aim of keeping bcasts etc to as few as possible)
	char *txtData[MaxTxtPattern];
	int txtLength[MaxTxtPattern];


	char *patternData[MaxTxtPattern];
	int patternLength[MaxTxtPattern];

	int inputData[800][3];

	// to store the execution time of code
		double time_spent = 0.0;
		clock_t begin = clock();

	int no_Of_Tests = readInputFile(inputData);
	readTextFile(txtData, txtLength);
	readPatternFile(patternData, patternLength);

	int rank,numtasks;

	char buffer[BufferSize];
	sprintf(buffer, "");

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == MASTER)
	{
		masterModel(numtasks, no_Of_Tests, inputData, txtLength, patternLength, buffer);
		OutputFile(buffer);
	}
	else
	{
		slaveModel(rank, txtData, patternData, txtLength, patternLength);
	}

		MPI_Finalize();
		 clock_t end = clock();

			    // calculate elapsed time by finding difference (end - begin) and
			    // dividing the difference by CLOCKS_PER_SEC to convert to seconds
			    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
			    printf("The elapsed time is %f seconds", time_spent);

			    return 0;
}

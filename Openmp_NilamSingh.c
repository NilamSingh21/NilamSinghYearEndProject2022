/*
* Open Multi-Processing (OpenMP) API for parallel programming.
* The pattern-finding procedure is simply put into a parallel loop.

* There are two modes that must be used:
  mode 0, which just requires finding the first instance of the pattern,
  and mode 1, which requires finding all instances of the pattern in the text.

* If the solution has already been determined and the system is in mode 0, the check is skipped.
  There are critical parts in place to ensure that only one thread can change the 'InstanceFound' flag at the same time.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>

//allow constant values to be declared for use throughout the code.
//It's safe to presume that no text or pattern file will be larger than 30 MB.
#define MaxTxtPattern 30
#define BufferSize 100000

char *txtData[MaxTxtPattern],*patternData[MaxTxtPattern];
int txtLength[MaxTxtPattern],patternLength[MaxTxtPattern];
int txtCnt,patternCnt;
int InputData[800][3];

void readFile(FILE *file, char **data, int *length)
{
	int chr,allocatedLength=0,resultLength = 0;
	char *result = NULL;;

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

int readTextFile()
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

int readPatternFile()
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

int readInputFile()
{
	FILE *file;
	char fileName[1000];

	//A master file, which specifies the mode, text to use, and pattern to use,
	//specifies the pattern and text combinations to be tested.

	sprintf (fileName, "Master.txt");

	file = fopen (fileName, "r");
	if (file == NULL)
		return 0;

	int c, i = 0;

	while ((c = fscanf(file, "%d %d %d", &InputData[i][0], &InputData[i][1], &InputData[i][2])) != EOF)
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

	sprintf (fileName, "OutputFile_OpenMP.txt");

	file = fopen(fileName, "a+");  //Opens a file for reading and appending
	if (file == NULL)
		return;
	fprintf(file,"%s", buffer);
	fclose(file);

	buffer[0] = '\0';
}

void writeBuffer(char buffer[], int txtIndex, int patternIndex, int matchPoint)
{
	if (strlen(buffer) > BufferSize - 30)
		OutputFile(buffer);

	sprintf(buffer+strlen(buffer), "%d %d %d\n", txtIndex, patternIndex, matchPoint);
}

// Mode 0 - find the first occurrence of the pattern
int searchFirstInstance(int txtIndex, int patternIndex)
{
	int i=0,j=0,k=0, lastInstance;
	lastInstance = txtLength[txtIndex] - patternLength[patternIndex];
	int InstanceFound = txtLength[txtIndex] + 1;

	//parallel Region
	#pragma omp parallel for default(none) shared(InstanceFound) private(j, k) firstprivate(lastInstance, txtData, patternData, patternLength, txtIndex, patternIndex) schedule(static) num_threads(8)

	for (i = 0; i <= lastInstance; i++)
		{
			if (i < InstanceFound)
			{
				k=i;
				j=0;

				while( txtData[txtIndex][k] == patternData[patternIndex][j] && j < patternLength[patternIndex] )
				{
					k++;
					j++;
				}

				if (j == patternLength[patternIndex])
				{
					#pragma omp critical(check)
					{
						if (InstanceFound > i)
							InstanceFound = i;
					}
				}

			}
		}
		if (InstanceFound < txtLength[txtIndex] + 1)
			return InstanceFound;
		else
			return -1;
	}

// Mode 1 - find all occurrences of the pattern
void searchAllInstances(int txtIndex, int patternIndex, char buffer[])
{
	int i,j=0,k,numThreads=8, lastInstance,InstanceFound=0;
	lastInstance = txtLength[txtIndex] - patternLength[patternIndex];

//parallel Region
	#pragma omp parallel for default(none) shared(InstanceFound, buffer) private(j, k) firstprivate(lastInstance, txtData, patternData, patternLength, txtIndex, patternIndex) schedule(static) num_threads(8)

	for (i = 0; i <= lastInstance; i++)
	{
		k=i;
		j=0;

		while( txtData[txtIndex][k] == patternData[patternIndex][j] && j < patternLength[patternIndex] )
		{
			k++;
			j++;
		}
		if (j == patternLength[patternIndex])
		{
			#pragma omp critical(check)
			{
				InstanceFound = 1;
				writeBuffer(buffer, txtIndex, patternIndex, i);
			}
		}

	}
	if (!InstanceFound)
		writeBuffer(buffer, txtIndex, patternIndex, -1);
}

// Call the correct function based on the mode
void dataProcess(int mode, int txtIndex, int patternIndex, char buffer[])
{
	// If patternLength > textLength, don't continue with the methods.
		if (patternLength[patternIndex] > txtLength[txtIndex])
		{
			writeBuffer(buffer, txtIndex, patternIndex, -1);
			return;
		}
		// else call the relevant function for the specified mode
		if (mode == 0)
			writeBuffer(buffer, txtIndex, patternIndex, searchFirstInstance(txtIndex, patternIndex));
		else
			searchAllInstances(txtIndex, patternIndex, buffer);
}

int main(int argc, char **argv)
{
	// to store the execution time of code
	double time_spent = 0.0;
	clock_t begin = clock();

	int no_Of_Tests = readInputFile();
	txtCnt=readTextFile();
	patternCnt=readPatternFile();

	char buffer[BufferSize];
	sprintf(buffer, "");

	int i;
	for (i = 0; i < no_Of_Tests; i++)
		dataProcess(InputData[i][0], InputData[i][1], InputData[i][2], buffer);

	OutputFile(buffer);
	clock_t end = clock();

		    // calculate elapsed time by finding difference (end - begin) and
		    // dividing the difference by CLOCKS_PER_SEC to convert to seconds
		    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
		    printf("Number of Threads : 8");
		    printf("\nThe elapsed time is %f seconds", time_spent);
		    return 0;
}


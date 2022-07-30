/*  BaseAlgorithmProgram.c
 *  Created on: 10-Feb-2022
 *  Author: Nilam Singh
 */

/*
 * wrote a base algorithm solution to generate correct result files for solving well known problems such as string pattern matching
 * The pattern is searched from left to right.
 * There are two modes:
 * 1. mode 0, which just requires finding the first instance of the pattern,
 * 2. mode 1, which requires finding all instances of the pattern in the text.
 * Files Detail:
 * Master File: Each Master File line   have three integers that correspond to a single search.
              first integer : 0 means only find the first occurrence, while a 1 means find all instances occurrence .
              second integer : Using the second integer will tell us which text to use.
              third integer : The third integer will indicates 	which pattern to use.
 * program output should be saved to a single file.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

// allow constant values to be declared for use throughout the code.
#define MaxTxtPattern 30
#define BufferSize 100000

int InputData[1000][3];
char *txtData[MaxTxtPattern],*patternData[MaxTxtPattern];
int txtLength[MaxTxtPattern],patternLength[MaxTxtPattern];

void readFile(FILE *file, char **data, int *length)
{
	char *result = NULL;
	int chr,resultLength = 0, assignLength = 0;

	chr = fgetc(file);
	while (chr >= 0)
	{
		resultLength++;
		if (resultLength > assignLength)
		{
			assignLength=assignLength + 10000;
			result = (char *)realloc(result, sizeof(char) * assignLength);
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

		sprintf (fileName,"PatternFile%d.txt",i);

		file = fopen (fileName, "r");
		if (file == NULL)
			return i;
		readFile (file, &patternData[i], &patternLength[i]);
		fclose (file);
	}
	return MaxTxtPattern;
}

int readMasterFile()
{
	FILE *file;
	char fileName[1000];

	//A Master file, which specifies the mode, text to use, and pattern to use,
	//specifies the pattern and text combinations to be tested.

	sprintf (fileName, "Master.txt");

	file = fopen (fileName, "r");
	if (file == NULL)
		return 0;

	int c,i = 0;

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

	sprintf (fileName, "OutputFile_BaseAlgorithm.txt");

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

	while (i<=lastInstance && j<patternLength[patternIndex])
	{
		if (txtData[txtIndex][k] == patternData[patternIndex][j])
		{
			k++;
			j++;
		}
		else
		{
			i++;
			k=i;
			j=0;
		}
	}
	if (j == patternLength[patternIndex])
		return i;
	else
		return -1;
}

// Mode 1 - find all occurrences of the pattern
void searchAllInstances(int txtIndex, int patternIndex, char buffer[])
{
	int i=0,j=0,k=0,InstanceFound = 0, lastInstance;
	lastInstance = txtLength[txtIndex] - patternLength[patternIndex];

	while (i<=lastInstance)
	{
		if (txtData[txtIndex][k] == patternData[patternIndex][j])
		{
			k++;
			j++;
			if (j == patternLength[patternIndex])
			{
				InstanceFound = 1;
				writeBuffer(buffer, txtIndex, patternIndex, i);
				i++;
				k=i;
				j=0;
			}
		}
		else
		{
			i++;
			k=i;
			j=0;
		}
	}
	if (!InstanceFound)
		writeBuffer(buffer, txtIndex, patternIndex, -1);
}

// Call the correct function based on the mode
void dataProcess(int mode, int txtIndex, int patternIndex, char buffer[])
{
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

	int countTests = readMasterFile();
	readTextFile();
	readPatternFile();

	char buffer[BufferSize];
	sprintf(buffer,"");
	for (int i = 0; i < countTests; i++)
		dataProcess(InputData[i][0], InputData[i][1], InputData[i][2], buffer);

	OutputFile(buffer);
	clock_t end = clock();

	    // calculate elapsed time by finding difference (end - begin) and
	    // dividing the difference by CLOCKS_PER_SEC to convert to seconds
	    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
	    printf("The elapsed time is %f seconds", time_spent);
	    return 0;
}




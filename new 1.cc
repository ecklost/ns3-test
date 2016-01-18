/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
* http://turnyournameintoaface.com/
* Author:    Eric C. Klostermeyer <Eric.Klostermeyer@rockets.utoledo.edu>
* Class:     EECS 3540 - Operating Systems and Systems Programming
* Professor: Dr. Gerald Heuring
* Project:   Collatz Sequence
* Due Date:  02 February 2016
*
* User will input the range of values like so: <thing here>
* Not complete as of yet.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

using namespace std;

int
main (int argc, char* argv[]) 
{
  // Defaults
  uint32_t nFirst = 1;
  uint32_t nSecond = 10;
 
  if (argc != 2) 
    {
      // Need two starting values 
	  fputs ("Specify a range of only two values"); // Input explanation here
	  exit (1);
    }
  
  // Command line values -- see initial block paragraph
  CommandLine cmd;
  cmd.AddValue ("nFirst", "Insert a value to start at.", nFirst);
  cmd.AddValue ("nSecond", "Insert a value to end at.", nSecond);
  
  cmd.parse (argc, argv);
  
  if (nSecond < nFirst)
    {
	  // If second value is larger than first, swap values
	  uint32_t nTemp = nSecond;
	  nSecond = nFirst;
	  nFirst = nTemp;
    }
  
  // Make arrays (multi-dimensional -- first is number range, second is frequency, third is time)
}

int[][][]
Ptr<collatz> (int* freqArray[][][])
{
  // Collatz one at a time.
  // Collatz, output to array, and repeat until no more in range
}
// Output table should have the # and how many times it took to get to 1
// Finish
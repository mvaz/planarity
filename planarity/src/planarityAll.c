/*
Planarity-Related Graph Algorithms Project
Copyright (c) 1997-2010, John M. Boyer
All rights reserved. Includes a reference implementation of the following:

* John M. Boyer. "Simplified O(n) Algorithms for Planar Graph Embedding,
  Kuratowski Subgraph Isolation, and Related Problems". Ph.D. Dissertation,
  University of Victoria, 2001.

* John M. Boyer and Wendy J. Myrvold. "On the Cutting Edge: Simplified O(n)
  Planarity by Edge Addition". Journal of Graph Algorithms and Applications,
  Vol. 8, No. 3, pp. 241-273, 2004.

* John M. Boyer. "A New Method for Efficiently Generating Planar Graph
  Visibility Representations". In P. Eades and P. Healy, editors,
  Proceedings of the 13th International Conference on Graph Drawing 2005,
  Lecture Notes Comput. Sci., Volume 3843, pp. 508-511, Springer-Verlag, 2006.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

* Neither the name of the Planarity-Related Graph Algorithms Project nor the names
  of its contributors may be used to endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "planarity.h"

void GetNumberIfZero(int *pNum, char *prompt, int min, int max);
void ReinitializeGraph(graphP *pGraph, int ReuseGraphs, char command);
graphP MakeGraph(int Size, char command);

/****************************************************************************
 RandomGraphs()
 Top-level method to randomly generate graphs to test the algorithm given by
 the command parameter.
 The number of graphs to generate, and the number of vertices for each graph,
 can be sent as the second and third params.  For each that is sent as zero,
 this method will prompt the user for a value.
 ****************************************************************************/

#define NUM_MINORS  9

int  RandomGraphs(char command, int NumGraphs, int SizeOfGraphs)
{
char theFileName[256];
int  K, countUpdateFreq;
int Result=OK, MainStatistic=0;
int  ObstructionMinorFreqs[NUM_MINORS];
graphP theGraph=NULL, origGraph=NULL;
platform_time start, end;
int embedFlags = GetEmbedFlags(command);
int ReuseGraphs = TRUE;

     GetNumberIfZero(&NumGraphs, "Enter number of graphs to generate:", 1, 1000000000);
     GetNumberIfZero(&SizeOfGraphs, "Enter size of graphs:", 1, 10000);

   	 theGraph = MakeGraph(SizeOfGraphs, command);
   	 origGraph = MakeGraph(SizeOfGraphs, command);
   	 if (theGraph == NULL || origGraph == NULL)
   	 {
   		 gp_Free(&theGraph);
   		 return NOTOK;
   	 }

     // Initialize a secondary statistics array
     for (K=0; K < NUM_MINORS; K++)
          ObstructionMinorFreqs[K] = 0;

   	 // Seed the random number generator with "now". Do it after any prompting
   	 // to tie randomness to human process of answering the prompt.
   	 srand(time(NULL));

   	 // Select a counter update frequency that updates more frequently with larger graphs
   	 // and which is relatively prime with 10 so that all digits of the count will change
   	 // even though we aren't showing the count value on every iteration
   	 countUpdateFreq = 3579 / SizeOfGraphs;
   	 countUpdateFreq = countUpdateFreq < 1 ? 1 : countUpdateFreq;
   	 countUpdateFreq = countUpdateFreq % 2 == 0 ? countUpdateFreq+1 : countUpdateFreq;
   	 countUpdateFreq = countUpdateFreq % 5 == 0 ? countUpdateFreq+2 : countUpdateFreq;

   	 // Start the count
     fprintf(stdout, "0\r");
     fflush(stdout);

     // Start the timer
     platform_GetTime(start);

     // Generate and process the number of graphs requested
     for (K=0; K < NumGraphs; K++)
     {
          if ((Result = gp_CreateRandomGraph(theGraph)) == OK)
          {
              if (tolower(OrigOut)=='y')
              {
                  sprintf(theFileName, "random\\%d.txt", K%10);
                  gp_Write(theGraph, theFileName, WRITE_ADJLIST);
              }

              gp_CopyGraph(origGraph, theGraph);

              if (strchr("pdo234", command))
              {
                  Result = gp_Embed(theGraph, embedFlags);

                  if (gp_TestEmbedResultIntegrity(theGraph, origGraph, Result) != Result)
                      Result = NOTOK;

                  if (Result == OK)
                  {
                       MainStatistic++;

                       if (tolower(EmbeddableOut) == 'y')
                       {
                           sprintf(theFileName, "embedded\\%d.txt", K%10);
                           gp_Write(theGraph, theFileName, WRITE_ADJMATRIX);
                       }

                       if (tolower(AdjListsForEmbeddingsOut) == 'y')
                       {
                           sprintf(theFileName, "adjlist\\%d.txt", K%10);
                           gp_Write(theGraph, theFileName, WRITE_ADJLIST);
                       }
                  }
                  else if (Result == NONEMBEDDABLE)
                  {
                       if (embedFlags == EMBEDFLAGS_PLANAR || embedFlags == EMBEDFLAGS_OUTERPLANAR)
                       {
                           if (theGraph->IC.minorType & MINORTYPE_A)
                                ObstructionMinorFreqs[0] ++;
                           else if (theGraph->IC.minorType & MINORTYPE_B)
                                ObstructionMinorFreqs[1] ++;
                           else if (theGraph->IC.minorType & MINORTYPE_C)
                                ObstructionMinorFreqs[2] ++;
                           else if (theGraph->IC.minorType & MINORTYPE_D)
                                ObstructionMinorFreqs[3] ++;
                           else if (theGraph->IC.minorType & MINORTYPE_E)
                                ObstructionMinorFreqs[4] ++;

                           if (theGraph->IC.minorType & MINORTYPE_E1)
                                ObstructionMinorFreqs[5] ++;
                           else if (theGraph->IC.minorType & MINORTYPE_E2)
                                ObstructionMinorFreqs[6] ++;
                           else if (theGraph->IC.minorType & MINORTYPE_E3)
                                ObstructionMinorFreqs[7] ++;
                           else if (theGraph->IC.minorType & MINORTYPE_E4)
                                ObstructionMinorFreqs[8] ++;

                           if (tolower(ObstructedOut) == 'y')
                           {
                               sprintf(theFileName, "obstructed\\%d.txt", K%10);
                               gp_Write(theGraph, theFileName, WRITE_ADJMATRIX);
                           }
                       }
                  }
              }
              else if (command == 'c')
              {
      			if ((Result = gp_ColorVertices(theGraph)) == OK)
      				 Result = gp_ColorVerticesIntegrityCheck(theGraph, origGraph);
				if (Result == OK && gp_GetNumColorsUsed(theGraph) <= 5)
					MainStatistic++;
              }

              // If there is an error in processing, then write the file for debugging
              if (Result != OK && Result != NONEMBEDDABLE)
              {
                   sprintf(theFileName, "error\\%d.txt", K%10);
                   gp_Write(origGraph, theFileName, WRITE_ADJLIST);
              }
          }

          // Reinitialize or recreate graphs for next iteration
          ReinitializeGraph(&theGraph, ReuseGraphs, command);
          ReinitializeGraph(&origGraph, ReuseGraphs, command);

          // Show progress, but not so often that it bogs down progress
          if (quietMode == 'n' && (K+1) % countUpdateFreq == 0)
          {
              fprintf(stdout, "%d\r", K+1);
              fflush(stdout);
          }

          // Terminate loop on error
          if (Result != OK && Result != NONEMBEDDABLE)
          {
        	  ErrorMessage("\nError found\n");
              Result = NOTOK;
              break;
          }
     }

     // Stop the timer
     platform_GetTime(end);

     // Finish the count
     fprintf(stdout, "%d\n", NumGraphs);
     fflush(stdout);

     // Free the graph structures created before the loop
     gp_Free(&theGraph);
     gp_Free(&origGraph);

     // Print some demographic results
     if (Result == OK || Result == NONEMBEDDABLE)
         Message("\nNo Errors Found.");
     sprintf(Line, "\nDone (%.3lf seconds).\n", platform_GetDuration(start,end));
     Message(Line);

     // Report statistics for planar or outerplanar embedding
     if (embedFlags == EMBEDFLAGS_PLANAR || embedFlags == EMBEDFLAGS_OUTERPLANAR)
     {
         sprintf(Line, "Num Embedded=%d.\n", MainStatistic);
         Message(Line);

         for (K=0; K<5; K++)
         {
        	  // Outerplanarity does not produces minors C and D
        	  if (embedFlags == EMBEDFLAGS_OUTERPLANAR && (K==2 || K==3))
        		  continue;

              sprintf(Line, "Minor %c = %d\n", K+'A', ObstructionMinorFreqs[K]);
              Message(Line);
         }

         if (!(embedFlags & ~EMBEDFLAGS_PLANAR))
         {
             sprintf(Line, "\nNote: E1 are added to C, E2 are added to A, and E=E3+E4+K5 homeomorphs.\n");
             Message(Line);

             for (K=5; K<NUM_MINORS; K++)
             {
                  sprintf(Line, "Minor E%d = %d\n", K-4, ObstructionMinorFreqs[K]);
                  Message(Line);
             }
         }
     }

     // Report statistics for graph drawing
     else if (embedFlags == EMBEDFLAGS_DRAWPLANAR)
     {
         sprintf(Line, "Num Graphs Embedded and Drawn=%d.\n", MainStatistic);
         Message(Line);
     }

     // Report statistics for subgraph homeomorphism algorithms
     else if (embedFlags == EMBEDFLAGS_SEARCHFORK23)
     {
         sprintf(Line, "Of the generated graphs, %d did not contain a K_{2,3} homeomorph as a subgraph.\n", MainStatistic);
         Message(Line);
     }
     else if (embedFlags == EMBEDFLAGS_SEARCHFORK33)
     {
         sprintf(Line, "Of the generated graphs, %d did not contain a K_{3,3} homeomorph as a subgraph.\n", MainStatistic);
         Message(Line);
     }
     else if (embedFlags == EMBEDFLAGS_SEARCHFORK4)
     {
         sprintf(Line, "Of the generated graphs, %d did not contain a K_4 homeomorph as a subgraph.\n", MainStatistic);
         Message(Line);
     }

     // Report statistics for vertex coloring
     else if (command == 'c')
     {
         sprintf(Line, "Num Graphs colored with 5 or fewer colors=%d.\n", MainStatistic);
         Message(Line);
     }

     FlushConsole(stdout);

     return Result==OK || Result==NONEMBEDDABLE ? OK : NOTOK;
}

/****************************************************************************
 GetNumberIfZero()
 Internal function that gets a number if the given *pNum is zero.
 The prompt is displayed if the number must be obtained from the user.
 Whether the given number is used or obtained from the user, the function
 ensures it is in the range [min, max] and assigns the midpoint value if
 it is not.
 ****************************************************************************/

void GetNumberIfZero(int *pNum, char *prompt, int min, int max)
{
	if (*pNum == 0)
	{
	    Prompt(prompt);
	    scanf(" %d", pNum);
	}

	if (min < 1) min = 1;
	if (max < min) max = min;

	if (*pNum < min || *pNum > max)
	{
		*pNum = (max + min) / 2;
        sprintf(Line, "Number out of range [%d, %d]; changed to %d\n", min, max, *pNum);
        ErrorMessage(Line);
	}
}

/****************************************************************************
 MakeGraph()
 Internal function that makes a new graph, initializes it, and attaches an
 algorithm to it based on the command.
 ****************************************************************************/

graphP MakeGraph(int Size, char command)
{
	graphP theGraph;
    if ((theGraph = gp_New()) == NULL || gp_InitGraph(theGraph, Size) != OK)
    {
    	ErrorMessage("Error creating space for a graph of the given size.\n");
    	gp_Free(&theGraph);
    	return NULL;
    }

// Enable the appropriate feature. Although the same code appears in SpecificGraph,
// it is deliberately not separated to a common utility because SpecificGraph is
// used as a self-contained tutorial.  It is not that hard to update both locations
// when new algorithms are added.

	switch (command)
	{
		case 'd' : gp_AttachDrawPlanar(theGraph); break;
		case '2' : gp_AttachK23Search(theGraph); break;
		case '3' : gp_AttachK33Search(theGraph); break;
		case '4' : gp_AttachK4Search(theGraph); break;
		case 'c' : gp_AttachColorVertices(theGraph); break;
	}

	return theGraph;
}

/****************************************************************************
 ReinitializeGraph()
 Internal function that will either reinitialize the given graph or free it
 and make a new one just like it.
 ****************************************************************************/

void ReinitializeGraph(graphP *pGraph, int ReuseGraphs, char command)
{
	if (ReuseGraphs)
		gp_ReinitializeGraph(*pGraph);
	else
	{
		graphP newGraph = MakeGraph((*pGraph)->N, command);
		gp_Free(pGraph);
		*pGraph = newGraph;
	}
}

/****************************************************************************
 Creates a random maximal planar graph, then adds 'extraEdges' edges to it.
 ****************************************************************************/

int RandomGraph(char command, int extraEdges, int numVertices, char *outfileName, char *outfile2Name)
{
int  Result;
platform_time start, end;
graphP theGraph=NULL, origGraph;
int embedFlags = GetEmbedFlags(command);
char saveEdgeListFormat;

     GetNumberIfZero(&numVertices, "Enter number of vertices:", 1, 1000000);
     if ((theGraph = MakeGraph(numVertices, command)) == NULL)
    	 return NOTOK;

     srand(time(NULL));

     Message("Creating the random graph...\n");
     platform_GetTime(start);
     if (gp_CreateRandomGraphEx(theGraph, 3*numVertices-6+extraEdges) != OK)
     {
         ErrorMessage("gp_CreateRandomGraphEx() failed\n");
         return NOTOK;
     }
     platform_GetTime(end);

     sprintf(Line, "Created random graph with %d edges in %.3lf seconds. ", theGraph->M, platform_GetDuration(start,end));
     Message(Line);
     FlushConsole(stdout);

     // The user may have requested a copy of the random graph before processing
     if (outfile2Name != NULL)
     {
         gp_Write(theGraph, outfile2Name, WRITE_ADJLIST);
     }

     origGraph = gp_DupGraph(theGraph);

     // Do the requested algorithm on the randomly generated graph
     Message("Now processing\n");
     FlushConsole(stdout);

     if (strchr("pdo234", command))
     {
         platform_GetTime(start);
         Result = gp_Embed(theGraph, embedFlags);
         platform_GetTime(end);

    	 gp_SortVertices(theGraph);

         if (gp_TestEmbedResultIntegrity(theGraph, origGraph, Result) != Result)
             Result = NOTOK;
     }
     else if (command == 'c')
     {
         platform_GetTime(start);
    	 Result = gp_ColorVertices(theGraph);
         platform_GetTime(end);
     }
     else
    	 Result = NOTOK;

     // Write what the algorithm determined and how long it took
     WriteAlgorithmResults(theGraph, Result, command, start, end, NULL);

     // On successful algorithm result, write the output file and see if the
     // user wants the edge list formatted file.
     if (Result == OK || Result == NONEMBEDDABLE)
     {
    	 if (outfileName != NULL)
    		 gp_Write(theGraph, outfileName, WRITE_ADJLIST);

         Prompt("Do you want to save the generated graph in edge list format (y/n)? ");
         fflush(stdin);
         scanf(" %c", &saveEdgeListFormat);
         if (tolower(saveEdgeListFormat) == 'y')
         {
        	 char *fileName = "maxPlanarEdgeList.txt";
             if (extraEdges > 0)
            	 fileName = "nonPlanarEdgeList.txt";

             SaveAsciiGraph(theGraph, fileName);
             sprintf(Line, "Edge list format saved to '%s'\n", fileName);
        	 Message(Line);
         }
     }
     else ErrorMessage("Failure occurred");

     gp_Free(&theGraph);
     gp_Free(&origGraph);

     FlushConsole(stdout);
     return Result;
}



/****************************************************************************
 SpecificGraph()
 ****************************************************************************/

int SpecificGraph(char command, char *infileName, char *outfileName, char *outfile2Name)
{
graphP theGraph, origGraph;
platform_time start, end;
int Result;

    // Get the filename of the graph to test
    if ((infileName = ConstructInputFilename(infileName)) == NULL)
      return NOTOK;

    // Create the graph and, if needed, attach the correct algorithm to it
    theGraph = gp_New();

  switch (command)
  {
    case 'd' : gp_AttachDrawPlanar(theGraph); break;
    case '2' : gp_AttachK23Search(theGraph); break;
    case '3' : gp_AttachK33Search(theGraph); break;
    case '4' : gp_AttachK4Search(theGraph); break;
    case 'c' : gp_AttachColorVertices(theGraph); break;
  }

    // Read the graph into memory
  Result = gp_Read(theGraph, infileName);
  if (Result == NONEMBEDDABLE)
  {
    Message("The graph contains too many edges.\n");
    // Some of the algorithms will still run correctly with some edges removed.
    if (strchr("pdo234", command))
    {
      Message("Some edges were removed, but the algorithm will still run correctly.\n");
      Result = OK;
    }
  }

  // If there was an unrecoverable error, report it
  if (Result != OK)
    ErrorMessage("Failed to read graph\n");

  // Otherwise, call the correct algorithm on it
  else
  {
    // Copy the graph for integrity checking
        origGraph = gp_DupGraph(theGraph);

        // Run the algorithm
        if (strchr("pdo234", command))
        {
        int embedFlags = GetEmbedFlags(command);
          platform_GetTime(start);

//          gp_CreateDFSTree(theGraph);
//          gp_SortVertices(theGraph);
//      gp_Write(theGraph, "debug.before.txt", WRITE_DEBUGINFO);
//          gp_SortVertices(theGraph);

      Result = gp_Embed(theGraph, embedFlags);
          platform_GetTime(end);
          Result = gp_TestEmbedResultIntegrity(theGraph, origGraph, Result);
        }
        else
        {
          platform_GetTime(start);
          if (command == 'c')
          {
          if ((Result = gp_ColorVertices(theGraph)) == OK)
             Result = gp_ColorVerticesIntegrityCheck(theGraph, origGraph);
          }
          else
          Result = NOTOK;
            platform_GetTime(end);
        }

        // Write what the algorithm determined and how long it took
        WriteAlgorithmResults(theGraph, Result, command, start, end, infileName);

        // Free the graph obtained for integrity checking.
        gp_Free(&origGraph);
  }

  // Report an error, if there was one, free the graph, and return
  if (Result != OK && Result != NONEMBEDDABLE)
  {
    ErrorMessage("AN ERROR HAS BEEN DETECTED\n");
    Result = NOTOK;
//    gp_Write(theGraph, "debug.after.txt", WRITE_DEBUGINFO);
  }

  // Provide the output file(s)
  else
  {
        // Restore the vertex ordering of the original graph (undo DFS numbering)
        if (strchr("pdo234", command))
            gp_SortVertices(theGraph);

        // Determine the name of the primary output file
        outfileName = ConstructPrimaryOutputFilename(infileName, outfileName, command);

        // For some algorithms, the primary output file is not always written
        if ((strchr("pdo", command) && Result == NONEMBEDDABLE) ||
          (strchr("234", command) && Result == OK))
          ;

        // Write the primary output file, if appropriate to do so
        else
      gp_Write(theGraph, outfileName, WRITE_ADJLIST);

        // NOW WE WANT TO WRITE THE SECONDARY OUTPUT FILE

    // When called from the menu system, we want to write the planar or outerplanar
    // obstruction, if one exists. For planar graph drawing, we want the character
        // art rendition.  An empty but non-NULL string is passed to indicate the necessity
        // of selecting a default name for the second output file.
    if (outfile2Name != NULL)
    {
      if ((command == 'p' || command == 'o') && Result == NONEMBEDDABLE)
      {
        // By default, use the same name as the primary output filename
        if (strlen(outfile2Name) == 0)
            outfile2Name = outfileName;
        gp_Write(theGraph, outfile2Name, WRITE_ADJLIST);
      }
      else if (command == 'd' && Result == OK)
      {
        // By default, add ".render.txt" to the primary output filename
        if (strlen(outfile2Name) == 0)
              strcat((outfile2Name = outfileName), ".render.txt");
        gp_DrawPlanar_RenderToFile(theGraph, outfile2Name);
      }
    }
  }

  // Free the graph
  gp_Free(&theGraph);

  // Flush any remaining message content to the user, and return the result
    FlushConsole(stdout);
  return Result;
}

/****************************************************************************
 WriteAlgorithmResults()
 ****************************************************************************/

void WriteAlgorithmResults(graphP theGraph, int Result, char command, platform_time start, platform_time end, char *infileName)
{
  if (infileName)
     sprintf(Line, "The graph '%s' ", infileName);
  else sprintf(Line, "The graph ");
  Message(Line);

  switch (command)
  {
    case 'p' : sprintf(Line, "is%s planar.\n", Result==OK ? "" : " not"); break;
    case 'd' : sprintf(Line, "is%s planar.\n", Result==OK ? "" : " not"); break;
    case 'o' : sprintf(Line, "is%s outerplanar.\n", Result==OK ? "" : " not"); break;
    case '2' : sprintf(Line, "has %s subgraph homeomorphic to K_{2,3}.\n", Result==OK ? "no" : "a"); break;
    case '3' : sprintf(Line, "has %s subgraph homeomorphic to K_{3,3}.\n", Result==OK ? "no" : "a"); break;
    case '4' : sprintf(Line, "has %s subgraph homeomorphic to K_4.\n", Result==OK ? "no" : "a"); break;
    case 'c' : sprintf(Line, "has been %d-colored.\n", gp_GetNumColorsUsed(theGraph)); break;
    default  : sprintf(Line, "nas not been processed due to unrecognized command.\n"); break;
  }
  Message(Line);

  sprintf(Line, "Algorithm '%s' executed in %.3lf seconds.\n",
      GetAlgorithmName(command), platform_GetDuration(start,end));
  Message(Line);
}



/****************************************************************************
 Configuration
 ****************************************************************************/

char Mode='r',
     OrigOut='n',
     EmbeddableOut='n',
     ObstructedOut='n',
     AdjListsForEmbeddingsOut='n',
     quietMode='n';

void Reconfigure()
{
     fflush(stdin);

     Prompt("\nDo you want to \n"
        "  Randomly generate graphs (r),\n"
        "  Specify a graph (s),\n"
        "  Randomly generate a maximal planar graph (m), or\n"
        "  Randomly generate a non-planar graph (n)?");
     scanf(" %c", &Mode);

     Mode = tolower(Mode);
     if (!strchr("rsmn", Mode))
       Mode = 's';

     if (Mode == 'r')
     {
        Message("\nNOTE: The directories for the graphs you want must exist.\n\n");

        Prompt("Do you want original graphs in directory 'random' (last 10 max)?");
        scanf(" %c", &OrigOut);

        Prompt("Do you want adj. matrix of embeddable graphs in directory 'embedded' (last 10 max))?");
        scanf(" %c", &EmbeddableOut);

        Prompt("Do you want adj. matrix of obstructed graphs in directory 'obstructed' (last 10 max)?");
        scanf(" %c", &ObstructedOut);

        Prompt("Do you want adjacency list format of embeddings in directory 'adjlist' (last 10 max)?");
        scanf(" %c", &AdjListsForEmbeddingsOut);
     }

     FlushConsole(stdout);
}

/****************************************************************************
 MESSAGE - prints a string, but when debugging adds \n and flushes stdout
 ****************************************************************************/

#define MAXLINE 1024
char Line[MAXLINE];

void Message(char *message)
{
  if (quietMode == 'n')
  {
      fprintf(stdout, "%s", message);

#ifdef DEBUG
//      fprintf(stdout, "\n");
      fflush(stdout);
#endif
  }
}

void ErrorMessage(char *message)
{
  if (quietMode == 'n')
  {
    fprintf(stderr, "%s", message);

#ifdef DEBUG
    fprintf(stderr, "\n");
    fflush(stderr);
#endif
  }
}

void FlushConsole(FILE *f)
{
#ifdef DEBUG
      // Certain debuggers only flush completed lines of output to the console
      fprintf(f, "\n");
#endif
      fflush(f);
}

void Prompt(char *message)
{
  Message(message);
  FlushConsole(stdout);
}

/****************************************************************************
 ****************************************************************************/

void SaveAsciiGraph(graphP theGraph, char *filename)
{
  int  e, EsizeOccupied;
  FILE *outfile = fopen(filename, "wt");
  fprintf(outfile, "%s\n", filename);

  EsizeOccupied = gp_EdgeInUseIndexBound(theGraph);
  for (e = gp_GetFirstEdge(theGraph); e < EsizeOccupied; e+=2)
  {
    // Skip the edge holes
    if (gp_EdgeInUse(theGraph, e))
    {
      fprintf(outfile, "%d %d\n", gp_GetNeighbor(theGraph, e)+1, gp_GetNeighbor(theGraph, e+1)+1);
    }
  }

  fprintf(outfile, "0 0\n");

  fclose(outfile);
}

/****************************************************************************
 ****************************************************************************/

int  FilesEqual(char *file1Name, char *file2Name)
{
  FILE *infile1 = NULL, *infile2 = NULL;
  int Result = TRUE;

  infile1 = fopen(file1Name, "r");
  infile2 = fopen(file2Name, "r");

  if (infile1 == NULL || infile2 == NULL)
    Result = FALSE;
  else
  {
    int c1=0, c2=0;

    // Read the first file to the end
    while ((c1 = fgetc(infile1)) != EOF)
    {
      // If we got a char from the first file, but not from the second
      // then the second file is shorter, so files are not equal
      if ((c2 = fgetc(infile2)) == EOF)
      {
        Result = FALSE;
        break;
      }

      // If we got a char from second file, but not equal to char from
      // first file, then files are not equal
      if (c1 != c2)
      {
        Result = FALSE;
        break;
      }
    }

    // If we got to the end of the first file without breaking the loop...
    if (c1 == EOF)
    {
      // Then attempt to read from the second file to ensure it also ends.
      if (fgetc(infile2) != EOF)
        Result = FALSE;
    }
  }

  if (infile1 != NULL) fclose(infile1);
  if (infile2 != NULL) fclose(infile2);
  return Result;
}

/****************************************************************************
 ****************************************************************************/

int GetEmbedFlags(char command)
{
  int embedFlags = 0;

  switch (command)
  {
    case 'o' : embedFlags = EMBEDFLAGS_OUTERPLANAR; break;
    case 'p' : embedFlags = EMBEDFLAGS_PLANAR; break;
    case 'd' : embedFlags = EMBEDFLAGS_DRAWPLANAR; break;
    case '2' : embedFlags = EMBEDFLAGS_SEARCHFORK23; break;
    case '3' : embedFlags = EMBEDFLAGS_SEARCHFORK33; break;
    case '4' : embedFlags = EMBEDFLAGS_SEARCHFORK4; break;
  }

  return embedFlags;
}

/****************************************************************************
 ****************************************************************************/

char *GetAlgorithmName(char command)
{
  char *algorithmName = "UnsupportedAlgorithm";

  switch (command)
  {
    case 'p' : algorithmName = "PlanarEmbed"; break;
    case 'd' : algorithmName = DRAWPLANAR_NAME; break;
    case 'o' : algorithmName = "OuterplanarEmbed"; break;
    case '2' : algorithmName = K23SEARCH_NAME; break;
    case '3' : algorithmName = K33SEARCH_NAME; break;
    case '4' : algorithmName = K4SEARCH_NAME; break;
    case 'c' : algorithmName = COLORVERTICES_NAME; break;
  }

  return algorithmName;
}

/****************************************************************************
 ****************************************************************************/

void AttachAlgorithm(graphP theGraph, char command)
{
  switch (command)
  {
    case 'd' : gp_AttachDrawPlanar(theGraph); break;
    case '2' : gp_AttachK23Search(theGraph); break;
    case '3' : gp_AttachK33Search(theGraph); break;
    case '4' : gp_AttachK4Search(theGraph); break;
    case 'c' : gp_AttachColorVertices(theGraph); break;
  }
}

/****************************************************************************
 A string used to construct input and output filenames.

 The SUFFIXMAXLENGTH is 32 to accommodate ".out.txt" + ".render.txt" + ".test.txt"
 ****************************************************************************/

#define FILENAMEMAXLENGTH 128
#define ALGORITHMNAMEMAXLENGTH 32
#define SUFFIXMAXLENGTH 32

char theFileName[FILENAMEMAXLENGTH+1+ALGORITHMNAMEMAXLENGTH+1+SUFFIXMAXLENGTH+1];

/****************************************************************************
 ConstructInputFilename()
 Returns a string not owned by the caller (do not free string).
 String contains infileName content if infileName is non-NULL.
 If infileName is NULL, then the user is asked to supply a name.
 Returns NULL on error, or a non-NULL string on success.
 ****************************************************************************/

char *ConstructInputFilename(char *infileName)
{
  if (infileName == NULL)
  {
    Prompt("Enter graph file name: ");
    fflush(stdin);
    scanf(" %s", theFileName);

    if (!strchr(theFileName, '.'))
      strcat(theFileName, ".txt");
  }
  else
  {
    if (strlen(infileName) > FILENAMEMAXLENGTH)
    {
      ErrorMessage("Filename is too long");
      return NULL;
    }
    strcpy(theFileName, infileName);
  }

  return theFileName;
}

/****************************************************************************
 ConstructPrimaryOutputFilename()
 Returns a string not owned by the caller (do not free string).
 Reuses the same memory space as ConstructinputFilename().
 If outfileName is non-NULL, then the result string contains its content.
 If outfileName is NULL, then the infileName and the command's algorithm name
 are used to construct a string.
 Returns non-NULL string
 ****************************************************************************/

char *ConstructPrimaryOutputFilename(char *infileName, char *outfileName, char command)
{
  char *algorithmName = GetAlgorithmName(command);

  if (outfileName == NULL)
  {
    // The output filename is based on the input filename
    if (theFileName != infileName)
        strcpy(theFileName, infileName);

    // If the primary output filename has not been given, then we use
    // the input filename + the algorithm name + a simple suffix
    if (strlen(algorithmName) <= ALGORITHMNAMEMAXLENGTH)
    {
      strcat(theFileName, ".");
      strcat(theFileName, algorithmName);
    }
    else
      ErrorMessage("Algorithm Name is too long, so it will not be used in output filename.");

      strcat(theFileName, ".out.txt");
  }
  else
  {
    if (strlen(outfileName) > FILENAMEMAXLENGTH)
    {
      // The output filename is based on the input filename
      if (theFileName != infileName)
          strcpy(theFileName, infileName);

        if (strlen(algorithmName) <= ALGORITHMNAMEMAXLENGTH)
        {
          strcat(theFileName, ".");
          strcat(theFileName, algorithmName);
        }
          strcat(theFileName, ".out.txt");
      sprintf(Line, "Outfile filename is too long. Result placed in %s", theFileName);
      ErrorMessage(Line);
    }
    else
    {
      if (theFileName != outfileName)
          strcpy(theFileName, outfileName);
    }
  }

  return theFileName;
}

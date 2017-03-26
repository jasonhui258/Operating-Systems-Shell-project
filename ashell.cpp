//Tevin Youn
//Jason Hui 913362314
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <sstream>
//check if we cna use cstring
#include <cstring>
#include <ncurses.h>
#include <sys/wait.h>
using namespace std;

int callLS(string str);
vector<string> split(string str, char delimiter);
int ffFunction(string file, string directory);
char** split(vector<string> arrayVector);
vector< vector<string> > splitPipe(vector<string> arrayVector);
void runSource(int fd[], vector<string> inp, int size, int itr, bool isBegin);
void runDest(int fd[], vector<string> inp, int size, int itr, bool isBegin); 

// GLOBAL VARIABLES:
struct termios SavedTermAttributes;

/******* TODO
1.############################################## DONE Fix the problem where the user can delete past the % in the terminal
2. #################################################MOSTLY DONE Add up arrow/ history
3. implement ff
4. Able to Execute PROGRAMS like a.out
5. able to redirect things, like txt1.txt > txt2.txt > txt3.txt so 
    so whatevers in txt1 overwrites txt2, and then creates new file for txt3.txt maybe?? piping involved
    
6.###############################MOSTLY DONE Make the BELL SOUND \a also if you backspace till the end, make noise 
7.@@####################################### Makefile DONE
8.########################################MOSTLY DONE backspace and prevent backspace
9. > < 
10. | piping 
11. FORK in 
**/
/*

ashell.out : ashell.o
        g++ -Wall -g -o ashell.out ashell.o

ashell.o : ashell.cpp
        g++ -Wall -g -c ashell.cpp

clean:
        rm -f ashell.out ashell.o
*/
/**************
 * REFERENCES:
 * http://www.dreamincode.net/forums/topic/59943-accessing-directories-in-cc-part-i/
 * http://www.qnx.com/developers/docs/660/index.jsp?topic=%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fg%2Fgetcwd.html
 * http://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
 * http://code.runnable.com/VHb0hWMZp-ws1gAr/splitting-a-string-into-a-vector-for-c%2B%2B
 * Jason cite list: 
 *http://linux.die.net/man/3/chdir
 * http://stackoverflow.com/questions/12862739/convert-string-to-char 
 * http://www.linuxquestions.org/questions/programming-9/detecting-arrow-key-presses-in-c-c-166055/
 */
//first one is chdir
//second one is c_str()
//third one is for arrow keys detect 



void ResetCanonicalMode(int fd, struct termios *savedattributes)
{
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    char *name;
    
    // Make sure stdin is a terminal.
    if(!isatty(fd)){
        fprintf (stderr, "Not a terminal.\n");
        exit(0);
    }
    
    // Save the terminal attributes so we can restore them later.
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes.
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO.
    TermAttributes.c_cc[VMIN] = 10;
    TermAttributes.c_cc[VTIME] = 5;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}

void directprint()
{    //START WRITING THE DIRECTORY ########## COFFEE BLACK
    // Print out the directory onces before the input.
    char cwd[1024];
       //cout<<"count cwd is:" <<countcwd<<endl;
   
    if( getcwd(cwd, sizeof(cwd)) != NULL) {
       //  cout << "In direct print! The cwd is: " << cwd << endl;
        int counthit=0;
        while(cwd[counthit]!='\0')
        {
            counthit++;
        }

        if(counthit > 16)
        {
            int sneeze=1;
            int leftovers=0;
             while(cwd[sneeze]!='/')
            {
                char temp[1];
                temp[0] =cwd[sneeze];
                leftovers++;
                sneeze++;
            }
              write(1,"/.../",5);
            int backwardscount = counthit;
            while(cwd[backwardscount]!='/')
            {
                backwardscount--;
            }
            backwardscount++;
            
            while(cwd[backwardscount]!='\0')
            {
                char temp[1];
                temp[0] =cwd[backwardscount];
                write(1, temp,1);
                backwardscount++;
            }
        write(1, "% ",2);
        }//end if look if above 16
        else
        {
            write(1, cwd,strlen(cwd));
            write(1, "% ",2);
        }
    }
}//directprint() end

void getoutofhere()
{  
  exit(3); 
}

void printpwd()
{
    char cwd[1024];
    if( getcwd(cwd, sizeof(cwd)) != NULL) {
        write(1, cwd,strlen(cwd));
        write(1,"\n",1); 
    }
}

void cdthese(string dir)
{ 
   int work; // If exists, 0, if doesn't, -1.
   const char *direct = dir.c_str();
   work= chdir(direct);
}

/**********************************************
 * 
 * MAIN
 * 
 * **********************************************/
int main(int argc, char *argv[])
{

    char RXChar; // Maybe the char being read in
    string input;
    
    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    
    // Print out the directory onces before the input.
    directprint();

    vector<string> stringhistory;
   // int stringarraycount=0;
    int stringmove=0;
    int firstup=0;
    int backlimit=0; 
    int directprinting=0; 
    int curHistoryLocation = -1;
    bool isUpArrow = false;
    while(1)
    {
        //keypad implementation end 
        read(STDIN_FILENO, &RXChar, 1);
        
         // START keystroke implementation
        int keystroke, two, three; 
        keystroke = RXChar;
       // cout<<"keystroke:   "<<keystroke<<endl; 
        // This part is just for backspace:
        // 127 is backspace. 27 is escape/start of arrow keys
        if(keystroke !=27 && keystroke != 127)
        {
            backlimit++; 
        }
        else if(keystroke==127)
        { 
            if(backlimit == 0) // If input is size 0.
            {
                write(1,"\a ",2);
            }
            else if(backlimit>=0)
            {    
				//cout<<"gethereee"<<endl; 
                //write(1,"backspace",9);
                backlimit--; 
                write(1,"\b ", 2);
                
                //input.at(input.length()-1)='';
                string temp; 
                for(int i=0; i<backlimit; i++)
                {
				  temp += input.at(i);
				 // cout<<"temp is going   "<<temp<<" "<<i<<endl; 
				   	
			    }
               // cout<<"temp is  "<<temp<<endl;
                input=""; 
                input=temp;  
               // cout<<"input is "<<input<<endl; 
               temp=""; 
              // cout<<"temp is gone?  "<<temp<<endl<<endl;
            }
        }
        ///////////////////////////////////////////////
        else if(keystroke ==27)
        {  //cout<<"27"; 
            two = getchar();
            if (two == 91)
            {
                three = getchar();
                
                //cout<<"three is "<<three<<endl; 
                if (three == 65) 
                { // Arrow key pressed
                   // cout << "Arrow Location " << curHistoryLocation << endl;
                    if (stringhistory.size() > 0) 
                    
                    { // Make sure the history is not empty
						//cout<<"history size is big"<<endl; 
                        if (firstup==0)
                        { // Up arrow pressed once
                            
							//cout<<"get here?"<<endl; 
                            curHistoryLocation = 0;
                           write(1, stringhistory[0].c_str(), stringhistory[0].length() );
                         //  cout<<"ssssssssssssssssssssssssssssssssssssssssssstring is tor"<<stringhistory[0].length()<<endl; 
                     backlimit = stringhistory[0].length(); 
                     input=stringhistory[curHistoryLocation]; 
                             firstup=1; 
 
                           // cout<<"firstup"<<endl; 
                        }
                        else if(firstup==1)
                        { // Up arrow pressed more than once
							
							//cout<<"  secone up"<<curHistoryLocation; 
							//cout<<"   sstringhistory"<<stringhistory.size()<<endl; 
                            if (curHistoryLocation <stringhistory.size()-1)
                            {///if 
                            
                          // cout<<"__________back limit is "<<backlimit<<endl; 
                          
                                for(int i =0; i < stringhistory[curHistoryLocation].length(); i++) // Delete the previous history
        			        	{
									write(1,"\b",1); 
								} 
									for(int i =0; i < stringhistory[curHistoryLocation].length(); i++) // Delete the previous history
        			        	{

									write(1," ",1); 
									} 
									for(int i =0; i < stringhistory[curHistoryLocation].length(); i++) // Delete the previous history
										{

											write(1,"\b",1);  
											} 

        			         	curHistoryLocation++;
                                // Access the next location
         
                               write(1, stringhistory[curHistoryLocation].c_str() , stringhistory[curHistoryLocation].length() );
                               
                               input=stringhistory[curHistoryLocation]; 
                                backlimit = stringhistory[curHistoryLocation].length(); 
                            }
                            else
                            {
								write(1, "\a",1); 
						    }
                        }// Up arrow pressed more than once ENDING 
                    }
                }
            }
        }
        
        ////////////////////////////////////
        // FOO is char by char input
         if(RXChar!=27)
       { 
           string chacha = "";
        chacha += RXChar;
        // Print out the char being typed
        write(1,chacha.c_str(),chacha.length());
       }
        if(0x04== RXChar)
        { // C-d
            break;
        }
        else
        {
            if (0x0A == RXChar) // If enter is pressed, execute.
            { 
                // History stuff:
               if (curHistoryLocation == -1) 
                {
                    backlimit = 0; 
              	    stringhistory.insert(stringhistory.begin(), input);
                }
                else {
                    input = stringhistory[curHistoryLocation];
                    curHistoryLocation = -1 ;
                    stringhistory.insert(stringhistory.begin(), input);
                   backlimit=0; 
                   firstup=0; 
                }
                
                // Split input into different vectors
                vector<string> inp = split(input,' ');
                vector< vector<string> > inpPipe = splitPipe(inp); 

                /*********************************************************
                 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                 * This is where all the FORKING and PIPING *magic* happens:
                 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                 * ********************************************************/
                 
                pid_t pid;
                
                
                /////////////////JASON ATTEMPT >
                
                
                vector< vector<string> > inpRight =splitRedirectRight(inp); 
                int statusR;
                int fdR[inpRight.size() *2]; 
                int numRight =inpRight.size()-1;
                bool isEndR=false;  
                if(0<numRight)
                { 
					
					ofstream firstfile;
					firstfile.open(inpRight[0], ofstream::out | ofstream ::app); 
					firstfile<<"HUHH?"<<endl; 
				  //ffstreaminpRight[0] 
				}
                
                /////////////////JASON ATTEMPt >
                
                
                
                
                
                int status;
                int fd[inpPipe.size() * 2];//Tevin Youn
//Jason Hui 913362314
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <sstream>
//check if we cna use cstring
#include <cstring>
#include <ncurses.h>
#include <sys/wait.h>
using namespace std;

int callLS(string str);
vector<string> split(string str, char delimiter);
int ffFunction(string file, string directory);
char** split(vector<string> arrayVector);
vector< vector<string> > splitPipe(vector<string> arrayVector);
void runSource(int fd[], vector<string> inp, int size, int itr, bool isBegin);
void runDest(int fd[], vector<string> inp, int size, int itr, bool isBegin); 

// GLOBAL VARIABLES:
struct termios SavedTermAttributes;

/******* TODO
1.############################################## DONE Fix the problem where the user can delete past the % in the terminal
2. #################################################MOSTLY DONE Add up arrow/ history
3. implement ff
4. Able to Execute PROGRAMS like a.out
5. able to redirect things, like txt1.txt > txt2.txt > txt3.txt so 
    so whatevers in txt1 overwrites txt2, and then creates new file for txt3.txt maybe?? piping involved
    
6.###############################MOSTLY DONE Make the BELL SOUND \a also if you backspace till the end, make noise 
7.@@####################################### Makefile DONE
8.########################################MOSTLY DONE backspace and prevent backspace
9. > < 
10. | piping 
11. FORK in 
**/
/*

ashell.out : ashell.o
        g++ -Wall -g -o ashell.out ashell.o

ashell.o : ashell.cpp
        g++ -Wall -g -c ashell.cpp

clean:
        rm -f ashell.out ashell.o
*/
/**************
 * REFERENCES:
 * http://www.dreamincode.net/forums/topic/59943-accessing-directories-in-cc-part-i/
 * http://www.qnx.com/developers/docs/660/index.jsp?topic=%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fg%2Fgetcwd.html
 * http://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
 * http://code.runnable.com/VHb0hWMZp-ws1gAr/splitting-a-string-into-a-vector-for-c%2B%2B
 * Jason cite list: 
 *http://linux.die.net/man/3/chdir
 * http://stackoverflow.com/questions/12862739/convert-string-to-char 
 * http://www.linuxquestions.org/questions/programming-9/detecting-arrow-key-presses-in-c-c-166055/
 * http://www.cs.loyola.edu/~jglenn/702/S2005/Examples/dup2.html
 
 *http://www.cplusplus.com/reference/fstream/ofstream/open/
 */
//first one is chdir
//second one is c_str()
//third one is for arrow keys detect 



void ResetCanonicalMode(int fd, struct termios *savedattributes)
{
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    char *name;
    
    // Make sure stdin is a terminal.
    if(!isatty(fd)){
        fprintf (stderr, "Not a terminal.\n");
        exit(0);
    }
    
    // Save the terminal attributes so we can restore them later.
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes.
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO.
    TermAttributes.c_cc[VMIN] = 10;
    TermAttributes.c_cc[VTIME] = 5;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}

void directprint()
{    //START WRITING THE DIRECTORY ########## COFFEE BLACK
    // Print out the directory onces before the input.
    char cwd[1024];
       //cout<<"count cwd is:" <<countcwd<<endl;
   
    if( getcwd(cwd, sizeof(cwd)) != NULL) {
       //  cout << "In direct print! The cwd is: " << cwd << endl;
        int counthit=0;
        while(cwd[counthit]!='\0')
        {
            counthit++;
        }

        if(counthit > 16)
        {
            int sneeze=1;
            int leftovers=0;
             while(cwd[sneeze]!='/')
            {
                char temp[1];
                temp[0] =cwd[sneeze];
                leftovers++;
                sneeze++;
            }
              write(1,"/.../",5);
            int backwardscount = counthit;
            while(cwd[backwardscount]!='/')
            {
                backwardscount--;
            }
            backwardscount++;
            
            while(cwd[backwardscount]!='\0')
            {
                char temp[1];
                temp[0] =cwd[backwardscount];
                write(1, temp,1);
                backwardscount++;
            }
        write(1, "% ",2);
        }//end if look if above 16
        else
        {
            write(1, cwd,strlen(cwd));
            write(1, "% ",2);
        }
    }
}//directprint() end

void getoutofhere()
{  
  exit(3); 
}

void printpwd()
{
    char cwd[1024];
    if( getcwd(cwd, sizeof(cwd)) != NULL) {
        write(1, cwd,strlen(cwd));
        write(1,"\n",1); 
    }
}

void cdthese(string dir)
{ 
   int work; // If exists, 0, if doesn't, -1.
   const char *direct = dir.c_str();
   work= chdir(direct);
}

/**********************************************
 * 
 * MAIN
 * 
 * **********************************************/
int main(int argc, char *argv[])
{

    char RXChar; // Maybe the char being read in
    string input;
    
    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    
    // Print out the directory onces before the input.
    directprint();

    vector<string> stringhistory;
   // int stringarraycount=0;
    int stringmove=0;
    int firstup=0;
    int backlimit=0; 
    int directprinting=0; 
    int curHistoryLocation = -1;
    bool isUpArrow = false;
    while(1)
    {
        //keypad implementation end 
        read(STDIN_FILENO, &RXChar, 1);
        
         // START keystroke implementation
        int keystroke, two, three; 
        keystroke = RXChar;
       // cout<<"keystroke:   "<<keystroke<<endl; 
        // This part is just for backspace:
        // 127 is backspace. 27 is escape/start of arrow keys
        if(keystroke !=27 && keystroke != 127)
        {
            backlimit++; 
        }
        else if(keystroke==127)
        { 
            if(backlimit == 0) // If input is size 0.
            {
                write(1,"\a ",2);
            }
            else if(backlimit>=0)
            {
                 //write(1,"backspace",9);
                backlimit--; 
                write(1,"\b ", 2);
                
                //input.at(input.length()-1)='';
                string temp; 
                for(int i=0; i<backlimit; i++)
                {
				  temp += input.at(i);
				 // cout<<"temp is going   "<<temp<<" "<<i<<endl; 
				   	
			    }
               // cout<<"temp is  "<<temp<<endl;
                input=""; 
                input=temp;  
               // cout<<"input is "<<input<<endl; 
               temp=""; 
              // cout<<"temp is gone?  "<<temp<<endl<<endl;
                
                
            }
        }
        ///////////////////////////////////////////////
        else if(keystroke ==27)
        {  //cout<<"27"; 
            two = getchar();
            if (two == 91)
            {
                three = getchar();
                
                //cout<<"three is "<<three<<endl; 
                if (three == 65) 
                { // Arrow key pressed
                   // cout << "Arrow Location " << curHistoryLocation << endl;
                    if (stringhistory.size() > 0) 
                    
                    { // Make sure the history is not empty
						//cout<<"history size is big"<<endl; 
                        if (firstup==0)
                        { // Up arrow pressed once
                            
							//cout<<"get here?"<<endl; 
                            curHistoryLocation = 0;
                           write(1, stringhistory[0].c_str(), stringhistory[0].length() );
      backlimit = stringhistory[0].length(); 
                     input=stringhistory[curHistoryLocation];
                             firstup=1; 
 
                            
                        }
                        else if(firstup==1)
                        { // Up arrow pressed more than once
							
							//cout<<"  secone up"<<curHistoryLocation; 
							//cout<<"   sstringhistory"<<stringhistory.size()<<endl; 
                            if (curHistoryLocation <stringhistory.size()-1)
                            {///if 
                            
                          // cout<<"__________back limit is "<<backlimit<<endl; 
                          
                                for(int i =0; i < stringhistory[curHistoryLocation].length(); i++) // Delete the previous history
        			        	{
									write(1,"\b",1); 
								} 
									for(int i =0; i < stringhistory[curHistoryLocation].length(); i++) // Delete the previous history
        			        	{

									write(1," ",1); 
									} 
									for(int i =0; i < stringhistory[curHistoryLocation].length(); i++) // Delete the previous history
										{

											write(1,"\b",1);  
											} 

        			         	curHistoryLocation++;
                                // Access the next location
         
                               write(1, stringhistory[curHistoryLocation].c_str() , stringhistory[curHistoryLocation].length() );
                                  backlimit = stringhistory[0].length(); 
                                  input=stringhistory[curHistoryLocation];
                            }
                            else
                            {
								write(1, "\a",1); 
						    }
                        }// Up arrow pressed more than once ENDING 
                    }
                }
            }
        }
        
        ////////////////////////////////////
        // FOO is char by char input
         if(RXChar!=27)
       { 
           string chacha = "";
        chacha += RXChar;
        // Print out the char being typed
        write(1,chacha.c_str(),chacha.length());
       }
        if(0x04== RXChar)
        { // C-d
            break;
        }
        else
        {
            if (0x0A == RXChar) // If enter is pressed, execute.
            { 
                // History stuff:
               if (curHistoryLocation == -1) 
                {
                    backlimit = 0; 
              	    stringhistory.insert(stringhistory.begin(), input);
                }
                else {
                    input = stringhistory[curHistoryLocation];
                    curHistoryLocation = -1 ;
                    stringhistory.insert(stringhistory.begin(), input);
                   backlimit=0; 
                   firstup=0; 
                }
                
                // Split input into different vectors
                vector<string> inp = split(input,' ');
                vector< vector<string> > inpPipe = splitPipe(inp); 

                /*********************************************************
                 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                 * This is where all the FORKING and PIPING *magic* happens:
                 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                 * ********************************************************/
                 
                pid_t pid;
                
                int status;
                int fd[inpPipe.size() * 2];
                bool isEnd = false;
                int numPipes = inpPipe.size()-1;
                
                if (0 < numPipes) { // THERE ARE PIPES
                    if (numPipes > 1) {
                        isEnd = true;
                    }
                    for (int i = 0 ; i < numPipes; i++) { 
                        pipe(fd + i*2);
                    }
                    cout << "STARTING" << endl;
                    
                    for (int i = 0 ; i < numPipes; i++) {
                        if(i == 0 && numPipes < 2) {
                            //cout << "OPTION 1" << endl;
                            runSource(fd, inpPipe[i], inpPipe.size() * 2, i, true);
                            runDest(fd, inpPipe[i+1], inpPipe.size() * 2, i, true);
                        }
                        else if(i == 0 && numPipes >= 2) {
                           // cout << "OPTION 2" << endl;
                            runSource(fd, inpPipe[i], inpPipe.size() * 2, i, true);
                            runDest(fd, inpPipe[i+1], inpPipe.size() * 2, i, false);
                        }
                        else if (i == numPipes - 1) {
                           // cout << "OPTION 3" << endl;
                            runDest(fd, inpPipe[i+1], inpPipe.size() * 2, i, true);
                        }
                        else {
                            //cout << "OPTION 4" << endl;
                            runDest(fd, inpPipe[i+1], inpPipe.size() * 2, i, false);
                        }
                    }
                    for (int i = 0 ; i < inpPipe.size(); i++) {
                        close(fd[i]);
                        close(fd[i+1]);
                    }
                    
                    for (int i = 0; i < numPipes + 1; i++) {
                        wait(&status);
                    }
                    directprint();
                    input = "";
                   // cout << "IM OUT!" << endl;
                }
                else if (0 == numPipes) {
                    char** inpchar = split(inp);
                    string fileToLook = "";
                    // EXECUTRE:
                    // *********************** LS ******************************
                    if (!inp[0].compare("ls"))
                    {
                        // Check if there is a specified directory after the ls
                        for(int i = 1; i < inp.size(); i++)
                        {
                            if(inp[i] != "")
                            {
                                fileToLook = inp[i];
                                break;
                            }
                        }
                        callLS(fileToLook);
                    }
                    // *********************** CD ******************************
                    else if(!inp[0].compare("cd"))
                    { 
                        for(int i = 1; i < inp.size(); i++)
                        {
                            if(inp[i] != "")
                            {
                                fileToLook = inp[i];
                                break;
                            }
                        }    
                    //    cout << "Changing directory to " << fileToLook << endl;
                        cdthese(fileToLook); 
                    }
                    // *********************** PWD ******************************
                    else if(!inp[0].compare("pwd"))
                    {
                        printpwd(); 
                    }
                    // *********************** FF ******************************
                    else if(!inp[0].compare("ff")) 
                    {
                        ffFunction(inp[1], inp[2]);   
                    }
                    // *********************** EXIT ******************************
                    else if(!inp[0].compare("exit"))
                    { 
                        ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
                        getoutofhere(); 
                    }
                    // *********************** EXECVP ******************************
                    else if ((execvp(inpchar[0], inpchar) > 0) )
                    {
                      //  cout << "DONE" << endl;          
                    }
                    directprint();
                    input = "";
                }
                
                //while ((pid = wait(&status)) != -1)	/* pick up all the dead children */
	            //	fprintf(stderr, "process %d exits with %d\n", pid, WEXITSTATUS(status));
            }
            else // If enter isn't pressed, continue reading in the chars
            {  
                int keying=RXChar; 
				if(keying!=127)
				{
                input += RXChar; 
				}
                
            }
        
        } // else
    } // The giant while(1).
    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    return 0;
}

//
void runSource(int fd[], vector<string> inp, int size, int itr, bool isBegin) {
    char** inpchar = split(inp);
    int pid;
    pid = fork(); // execute fork
    string fileToLook = "";
    if (pid == 0) { // CHILD
        if(isBegin) {
         //   cout << " IN BEGIN " << endl;
            dup2(fd[1], 1); // End becomes standard output
        }
        else {
            cout << "This shouldn't print out" << endl;
        }
        for(int i = 0; i < size; i++) {
            close(fd[i]); // We dont need the other end
        }
        // EXECUTRE:
        // *********************** LS ******************************
        if (!inp[0].compare("ls"))
        {
            // Check if there is a specified directory after the ls
            for(int i = 1; i < inp.size(); i++)
            {
                if(inp[i] != "")
                {
                    fileToLook = inp[i];
                    break;
                }
            }
            callLS(fileToLook);
        }
        // *********************** CD ******************************
        else if(!inp[0].compare("cd"))
        { 
            for(int i = 1; i < inp.size(); i++)
            {
                if(inp[i] != "")
                {
                    fileToLook = inp[i];
                    break;
                }
            }    
        //    cout << "Changing directory to " << fileToLook << endl;
            cdthese(fileToLook); 
        }
        // *********************** PWD ******************************
        else if(!inp[0].compare("pwd"))
        {
            printpwd(); 
        }
        // *********************** FF ******************************
        else if(!inp[0].compare("ff")) 
        {
            ffFunction(inp[1], inp[2]);   
        }
        // *********************** EXIT ******************************
        else if(!inp[0].compare("exit"))
        { 
            ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
            getoutofhere(); 
        }
        // *********************** EXECVP ******************************
        else if ((execvp(inpchar[0], inpchar) > 0) )
        {
          //  cout << "DONE" << endl;          
        }
    }
}
void runDest(int fd[], vector<string> inp, int size, int itr, bool isEnd) {
    char** inpchar = split(inp);
    int pid;
    pid = fork();
    string fileToLook = "";
    if (pid == 0) {
        if (isEnd) {
           // cout << " IN IS END DEST" << endl;
            dup2(fd[itr*2 + 0], 0); // End becomes standard input
        }
        else {
           // cout << " IN NOT IS END DEST" << endl;
            dup2(fd[itr*2 + 0], 0);
            dup2(fd[itr*2 + 3], 1);
        }
        for(int i = 0; i < size; i++) {
            close(fd[i]); // We dont need the other end
        }
        // *********************** LS ******************************
        if (!inp[0].compare("ls"))
        {
            // Check if there is a specified directory after the ls
            for(int i = 1; i < inp.size(); i++)
            {
                if(inp[i] != "")
                {
                    fileToLook = inp[i];
                    break;
                }
            }
            callLS(fileToLook);
        }
        // *********************** CD ******************************
        else if(!inp[0].compare("cd"))
        { 
            for(int i = 1; i < inp.size(); i++)
            {
                if(inp[i] != "")
                {
                    fileToLook = inp[i];
                    break;
                }
            }    
        //    cout << "Changing directory to " << fileToLook << endl;
            cdthese(fileToLook); 
        }
        // *********************** PWD ******************************
        else if(!inp[0].compare("pwd"))
        {
            printpwd(); 
        }
        // *********************** FF ******************************
        else if(!inp[0].compare("ff")) 
        {
            ffFunction(inp[1], inp[2]);   
        }
        // *********************** EXIT ******************************
        else if(!inp[0].compare("exit"))
        { 
            ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
            getoutofhere(); 
        }
        // *********************** EXECVP ******************************
        else if ((execvp(inpchar[0], inpchar) > 0) )
        {
           // cout << "DONE" << endl;
             write(1,"DONE",4);
        }
    }
}

// FIRST split: return a vector


vector<string> split(string str, char delimiter) {
    vector<string> dividedString;
    stringstream ss(str);
    string token;
    string combine; 
    for (int i = 0; i < str.size();i++) {
        if (str.at(i) == '\\') {
            //cout << "FOUND" << endl;
            if(str.at(i+1) != ' ') {
                str.erase(i,1);
            }     
        }
    }
   /// cout << "STRING IS " << str << endl;
    while(getline(ss,token,delimiter)) 
    {
     //   cout << "LAST ONE IS " << token.at(token.size() - 1) << endl;
       if (token != "" && token.at(token.size() - 1) != '\\') 
        {
            dividedString.push_back(token);
        }
        else if (token != "" && token.at(token.size() - 1) == '\\') 
        {
           // token.erase(token.size() - 1, 1);
            token.insert(token.size(), " ");
            string tmp = token;
            getline(ss,token,delimiter);
            tmp += token;
           // cout << "TOKEN IS " << tmp << endl;;
            dividedString.push_back(tmp);
        }
    }
    return dividedString;
}

// SECOND split: returns a char*[]
char** split(vector<string> arrayVector) {
    char** arrayChar = new char*[arrayVector.size()];
    for (int i = 0 ; i < arrayVector.size();i++) {
        arrayChar[i] = const_cast<char*>(arrayVector[i].c_str());
    }
    return arrayChar;
}

vector<vector<string> > splitRedirect(vector<string> arrayVector, string delimiter) {
    vector< vector<string> > returnVector;
    vector<string> tmpVector;
    for(int i = 0; i < arrayVector.size(); i++) {
        if (arrayVector[i] == delimiter && i != 0 && i != arrayVector.size() - 1) { // 2. the input is not a pipe.
            tmpVector.push_back(arrayVector[i - 1]);
            tmpVector.push_back(arrayVector[i + 1]);
            returnVector.push_back(tmpVector);
            tmpVector.clear();
        }
    }
    return returnVector;
}

// THIRD split: returns a vector of vector. This uses '|' as a deliminator.
vector< vector<string> > splitPipe(vector<string> arrayVector) {
    vector< vector<string> > returnVector;
    vector<string> tmpVector;
    for(int i = 0; i < arrayVector.size(); i++) {
        if (i == arrayVector.size() - 1) { // 1. we hit the end.
            tmpVector.push_back(arrayVector[i]);
            returnVector.push_back(tmpVector);
        }
        else if (arrayVector[i] != "|") { // 2. the input is not a pipe.
            tmpVector.push_back(arrayVector[i]);
        }
        else { // 3. the input IS a pipe.
            returnVector.push_back(tmpVector);
            tmpVector.clear();
        }
    }
    return returnVector;
    
}

int ffFunction(string file, string directory) {
    
    vector<string> listDir;
    DIR *pdir = NULL;
    struct dirent *pJoe = NULL;
    struct stat fileStat;
    string curDir = "./" + directory;
    // This set curDir so that it holds the path of the current directory being checked.
    pdir = opendir(curDir.c_str());
    int length = 15+directory.size();
    string name = "Unable to open " + directory;
    if (pdir == NULL) 
    {
        //write(1, name, 30);
        cout << "CANT OPEN" << endl;
    }
    
    // Loop throug all the files and subdirectories in the given directory.
    while (pJoe = readdir(pdir)) {
        string path = curDir + "/" + (pJoe->d_name); 
        
        if (stat(path.c_str(), &fileStat)) {
            cout << "Stat unsuccessful" << endl;
        }

        // It is a directory
        if (pJoe == NULL) { // was not opened correctly
            cout << "The directory does not exist." << endl;
            exit(1);
        }
        string str = string(pJoe->d_name);
        
        if(S_ISREG(fileStat.st_mode)) {
            if (str == file)
            {
               // cout << directory << "/" << str << endl;
                write(1,directory.c_str(),directory.length()); 
                write(1,"/",1);
                write(1,str.c_str(),str.length());
                write(1,"\n",1);
            }
        }
        else {
            // For some reason string to char* comparison doesnt work so you
            //  have to convert pJoe, which is a char *, into a string.
            
             if (str != "." && str != "..") 
             { 
                ffFunction(file,directory + "/" + pJoe->d_name);
             }
        }
        // otherwise, it was opened correctly
    }
    closedir(pdir);
}

int callLS(string directory) {
    DIR *pdir = NULL;
    struct dirent *pJoe = NULL;
    
    // If there is no directory after ls, then execute: ls .
    if (directory == ""){
        pdir = opendir(".");
    }
    // If there is a directory after ls, then execure: ls <dir>
    else {
        string dir = "./" + directory;
        pdir = opendir(dir.c_str());
    }
    
    struct stat fileStat;
    
    if(pdir == NULL) {
        cout << "PDIR is null! Oh noe!" << endl;
        exit(1);
    }
    
    while (pJoe = readdir(pdir)) {
        if (pJoe == NULL) { // was not opened correctly
            cout << "PJOE is null! Oh noe!" << endl;
            exit(1);
        }
        
        // otherwise, it was opened correctly
        if (stat(pJoe->d_name,&fileStat) < 0) {
            return 1;
        }
        write( 1,(S_ISDIR(fileStat.st_mode)) ? "d" : "-",1);
        write( 1, (fileStat.st_mode & S_IRUSR) ? "r" : "-",1);
        write( 1, (fileStat.st_mode & S_IWUSR) ? "w" : "-",1);
        write( 1,(fileStat.st_mode & S_IXUSR) ? "x" : "-",1);
        write( 1,(fileStat.st_mode & S_IRGRP) ? "r" : "-",1);
        write( 1,(fileStat.st_mode & S_IWGRP) ? "w" : "-",1);
        write( 1,(fileStat.st_mode & S_IXGRP) ? "x" : "-",1);
        write( 1,(fileStat.st_mode & S_IROTH) ? "r" : "-",1);
        write( 1,(fileStat.st_mode & S_IWOTH) ? "w" : "-",1);
        write( 1,(fileStat.st_mode & S_IXOTH) ? "x" : "-",1);
        write( 1, " ", 1);
        write(1,pJoe->d_name,strlen(pJoe->d_name));
        write(1,"\n",1);
    }
    closedir(pdir);
}


                bool isEnd = false;
                int numPipes = inpPipe.size()-1;
                
                if (0 < numPipes) {
                    if (numPipes > 1) {
                        isEnd = true;
                    }
                    for (int i = 0 ; i < numPipes; i++) { 
                        pipe(fd + i*2);
                    }
                    cout << "STARTING" << endl;
                    
                    for (int i = 0 ; i < numPipes; i++) {
                        if(i == 0 && numPipes < 2) {
                            cout << "OPTION 1" << endl;
                            runSource(fd, inpPipe[i], inpPipe.size() * 2, i, true);
                            runDest(fd, inpPipe[i+1], inpPipe.size() * 2, i, true);
                        }
                        else if(i == 0 && numPipes >= 2) {
                            cout << "OPTION 2" << endl;
                            runSource(fd, inpPipe[i], inpPipe.size() * 2, i, true);
                            runDest(fd, inpPipe[i+1], inpPipe.size() * 2, i, false);
                        }
                        else if (i == numPipes - 1) {
                            cout << "OPTION 3" << endl;
                            runDest(fd, inpPipe[i+1], inpPipe.size() * 2, i, true);
                        }
                        else {
                            cout << "OPTION 4" << endl;
                            runDest(fd, inpPipe[i+1], inpPipe.size() * 2, i, false);
                        }
                    }
                    for (int i = 0 ; i < inpPipe.size(); i++) {
                        close(fd[i]);
                        close(fd[i+1]);
                    }
                    
                    for (int i = 0; i < numPipes + 1; i++) {
                        wait(&status);
                    }
                    directprint();
                    input = "";
                   // cout << "IM OUT!" << endl;
                }
                else if (0 == numPipes) {
                    char** inpchar = split(inp);
                    string fileToLook = "";
                    // EXECUTRE:
                    // *********************** LS ******************************
                    if (!inp[0].compare("ls"))
                    {
                        // Check if there is a specified directory after the ls
                        for(int i = 1; i < inp.size(); i++)
                        {
                            if(inp[i] != "")
                            {
                                fileToLook = inp[i];
                                break;
                            }
                        }
                        callLS(fileToLook);
                    }
                    // *********************** CD ******************************
                    else if(!inp[0].compare("cd"))
                    { 
                        for(int i = 1; i < inp.size(); i++)
                        {
                            if(inp[i] != "")
                            {
                                fileToLook = inp[i];
                                break;
                            }
                        }    
                    //    cout << "Changing directory to " << fileToLook << endl;
                        cdthese(fileToLook); 
                    }
                    // *********************** PWD ******************************
                    else if(!inp[0].compare("pwd"))
                    {
                        printpwd(); 
                    }
                    // *********************** FF ******************************
                    else if(!inp[0].compare("ff")) 
                    {
                        ffFunction(inp[1], inp[2]);   
                    }
                    // *********************** EXIT ******************************
                    else if(!inp[0].compare("exit"))
                    { 
                        ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
                        getoutofhere(); 
                    }
                    // *********************** EXECVP ******************************
                    else if ((execvp(inpchar[0], inpchar) > 0) )
                    {
                      //  cout << "DONE" << endl;          
                    }
                    directprint();
                    input = "";
                }
                
                //while ((pid = wait(&status)) != -1)	/* pick up all the dead children */
	            //	fprintf(stderr, "process %d exits with %d\n", pid, WEXITSTATUS(status));
            }
            else // If enter isn't pressed, continue reading in the chars
            {  
				int keying=RXChar; 
				if(keying!=127)
                input += RXChar; 
            }
        
        } // else
    } // The giant while(1).
    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    return 0;
}

//
void runSource(int fd[], vector<string> inp, int size, int itr, bool isBegin) {
    char** inpchar = split(inp);
    int pid;
    pid = fork(); // execute fork
    string fileToLook = "";
    if (pid == 0) { // CHILD
        if(isBegin) {
         //   cout << " IN BEGIN " << endl;
            dup2(fd[1], 1); // End becomes standard output
        }
        else {
            cout << "This shouldn't print out" << endl;
        }
        for(int i = 0; i < size; i++) {
            close(fd[i]); // We dont need the other end
        }
        // EXECUTRE:
        // *********************** LS ******************************
        if (!inp[0].compare("ls"))
        {
            // Check if there is a specified directory after the ls
            for(int i = 1; i < inp.size(); i++)
            {
                if(inp[i] != "")
                {
                    fileToLook = inp[i];
                    break;
                }
            }
            callLS(fileToLook);
        }
        // *********************** CD ******************************
        else if(!inp[0].compare("cd"))
        { 
            for(int i = 1; i < inp.size(); i++)
            {
                if(inp[i] != "")
                {
                    fileToLook = inp[i];
                    break;
                }
            }    
        //    cout << "Changing directory to " << fileToLook << endl;
            cdthese(fileToLook); 
        }
        // *********************** PWD ******************************
        else if(!inp[0].compare("pwd"))
        {
            printpwd(); 
        }
        // *********************** FF ******************************
        else if(!inp[0].compare("ff")) 
        {
            ffFunction(inp[1], inp[2]);   
        }
        // *********************** EXIT ******************************
        else if(!inp[0].compare("exit"))
        { 
            ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
            getoutofhere(); 
        }
        // *********************** EXECVP ******************************
        else if ((execvp(inpchar[0], inpchar) > 0) )
        {
          //  cout << "DONE" << endl;          
        }
    }
}
void runDest(int fd[], vector<string> inp, int size, int itr, bool isEnd) {
    char** inpchar = split(inp);
    int pid;
    pid = fork();
    string fileToLook = "";
    if (pid == 0) {
        if (isEnd) {
           // cout << " IN IS END DEST" << endl;
            dup2(fd[itr*2 + 0], 0); // End becomes standard input
        }
        else {
           // cout << " IN NOT IS END DEST" << endl;
            dup2(fd[itr*2 + 0], 0);
            dup2(fd[itr*2 + 3], 1);
        }
        for(int i = 0; i < size; i++) {
            close(fd[i]); // We dont need the other end
        }
        // *********************** LS ******************************
        if (!inp[0].compare("ls"))
        {
            // Check if there is a specified directory after the ls
            for(int i = 1; i < inp.size(); i++)
            {
                if(inp[i] != "")
                {
                    fileToLook = inp[i];
                    break;
                }
            }
            callLS(fileToLook);
        }
        // *********************** CD ******************************
        else if(!inp[0].compare("cd"))
        { 
            for(int i = 1; i < inp.size(); i++)
            {
                if(inp[i] != "")
                {
                    fileToLook = inp[i];
                    break;
                }
            }    
        //    cout << "Changing directory to " << fileToLook << endl;
            cdthese(fileToLook); 
        }
        // *********************** PWD ******************************
        else if(!inp[0].compare("pwd"))
        {
            printpwd(); 
        }
        // *********************** FF ******************************
        else if(!inp[0].compare("ff")) 
        {
            ffFunction(inp[1], inp[2]);   
        }
        // *********************** EXIT ******************************
        else if(!inp[0].compare("exit"))
        { 
            ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
            getoutofhere(); 
        }
        // *********************** EXECVP ******************************
        else if ((execvp(inpchar[0], inpchar) > 0) )
        {
            cout << "DONE" << endl;          
        }
    }
}

// FIRST split: return a vector
vector<string> split(string str, char delimiter) {
    vector<string> dividedString;
    stringstream ss(str);
    string token;
    string combine; 
    int onandoff=0; 
    
    while(getline(ss,token,delimiter)) 
    
    {
    
       if (token != "" && onandoff==0) 
        {
            dividedString.push_back(token);
        }
        
    }
    return dividedString;
}

// SECOND split: returns a char*[]
char** split(vector<string> arrayVector) {
    char** arrayChar = new char*[arrayVector.size()];
    for (int i = 0 ; i < arrayVector.size();i++) {
        arrayChar[i] = const_cast<char*>(arrayVector[i].c_str());
    }
    return arrayChar;
}

// THIRD split: returns a vector of vector. This uses '|' as a deliminator.
vector< vector<string> > splitPipe(vector<string> arrayVector) {
    vector< vector<string> > returnVector;
    vector<string> tmpVector;
    for(int i = 0; i < arrayVector.size(); i++) {
        if (i == arrayVector.size() - 1) { // 1. we hit the end.
            tmpVector.push_back(arrayVector[i]);
            returnVector.push_back(tmpVector);
        }
        else if (arrayVector[i] != "|") { // 2. the input is not a pipe.
            tmpVector.push_back(arrayVector[i]);
        }
        else { // 3. the input IS a pipe.
            returnVector.push_back(tmpVector);
            tmpVector.clear();
        }
    }
    return returnVector;
    
}

int ffFunction(string file, string directory) {
    
    vector<string> listDir;
    DIR *pdir = NULL;
    struct dirent *pJoe = NULL;
    struct stat fileStat;
    string curDir = "./" + directory;
    // This set curDir so that it holds the path of the current directory being checked.
    pdir = opendir(curDir.c_str());
    int length = 15+directory.size();
    string name = "Unable to open " + directory;
    if (pdir == NULL) 
    {
        //write(1, name, 30);
        cout << "CANT OPEN" << endl;
    }
    
    // Loop throug all the files and subdirectories in the given directory.
    while (pJoe = readdir(pdir)) {
        string path = curDir + "/" + (pJoe->d_name); 
        
        if (stat(path.c_str(), &fileStat)) {
            cout << "Stat unsuccessful" << endl;
        }

        // It is a directory
        if (pJoe == NULL) { // was not opened correctly
            cout << "The directory does not exist." << endl;
            exit(1);
        }
        string str = string(pJoe->d_name);
        
        if(S_ISREG(fileStat.st_mode)) {
            if (str == file)
            {
                cout << directory << "/" << str << endl;
            }
        }
        else {
            // For some reason string to char* comparison doesnt work so you
            //  have to convert pJoe, which is a char *, into a string.
            
             if (str != "." && str != "..") 
             { 
                ffFunction(file,directory + "/" + pJoe->d_name);
             }
        }
        // otherwise, it was opened correctly
    }
    closedir(pdir);
}

int callLS(string directory) {
    DIR *pdir = NULL;
    struct dirent *pJoe = NULL;
    
    // If there is no directory after ls, then execute: ls .
    if (directory == ""){
        pdir = opendir(".");
    }
    // If there is a directory after ls, then execure: ls <dir>
    else {
        string dir = "./" + directory;
        pdir = opendir(dir.c_str());
    }
    
    struct stat fileStat;
    
    if(pdir == NULL) {
        cout << "PDIR is null! Oh noe!" << endl;
        exit(1);
    }
    
    while (pJoe = readdir(pdir)) {
        if (pJoe == NULL) { // was not opened correctly
            cout << "PJOE is null! Oh noe!" << endl;
            exit(1);
        }
        
        // otherwise, it was opened correctly
        if (stat(pJoe->d_name,&fileStat) < 0) {
            return 1;
        }
        write( 1,(S_ISDIR(fileStat.st_mode)) ? "d" : "-",1);
        write( 1, (fileStat.st_mode & S_IRUSR) ? "r" : "-",1);
        write( 1, (fileStat.st_mode & S_IWUSR) ? "w" : "-",1);
        write( 1,(fileStat.st_mode & S_IXUSR) ? "x" : "-",1);
        write( 1,(fileStat.st_mode & S_IRGRP) ? "r" : "-",1);
        write( 1,(fileStat.st_mode & S_IWGRP) ? "w" : "-",1);
        write( 1,(fileStat.st_mode & S_IXGRP) ? "x" : "-",1);
        write( 1,(fileStat.st_mode & S_IROTH) ? "r" : "-",1);
        write( 1,(fileStat.st_mode & S_IWOTH) ? "w" : "-",1);
        write( 1,(fileStat.st_mode & S_IXOTH) ? "x" : "-",1);
        write( 1, " ", 1);
        write(1,pJoe->d_name,strlen(pJoe->d_name));
        write(1,"\n",1);
    }
    closedir(pdir);
}


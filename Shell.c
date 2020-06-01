/*
 * File: shell.c
 * Author: Liam O'Shea
 * Description: This program acts as a simple UNIX shell. The shell has a history feature
 * which allows the user to access previously used commands.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <ctype.h>

#define MAX_LINE 80             //Max length of command
#define MAX_HIST (10)

typedef struct {
    char argument[MAX_LINE];
    pid_t pid;
}HistoryItem;

HistoryItem histList[MAX_HIST] = {};
int totalHistItems = 0;         //Total number of UNIX commands issued by user during session


char** tokenizeInput(char * in){
    int numTok = 0;           //Number of separate tokens in command
    char ** args = malloc(60 * sizeof(char*));
    char* tok = "nil";
    while ( tok != NULL ) {
        if ( numTok == 0 ) {
            tok = strtok( in, " " );
        }
        else {
            tok = strtok( NULL, " " );
        }
        args[numTok] = tok;
        if ( tok != NULL ) {
            numTok++;
        }
    }
    return args;
}

void rotateHistory() {
    for ( int i = MAX_HIST -1; i > 0; i-- ) {
        histList[i] = histList[i - 1];
    }
}

void addToHistory( HistoryItem historyEntry ) {
    rotateHistory();
    histList[0] = historyEntry;
    totalHistItems++;
}

void executeCommand( char** args) {

    // Fork a child process
    pid_t pid;
    int status;
    pid = fork();

    //Error handling block
    if ( pid < 0 ) {
        fprintf( stderr, "Unable to fork\n" );
        exit( EXIT_FAILURE );
    }

    //Child process runs in this block
    else if ( pid == 0 ) {
        //Child process invokes execvp
        if ( execvp( args[0], args ) == -1 ) {
            //Error has occurred
            fprintf( stderr, "Unable to use execvp\n" );
            exit( errno );
        }
        // In case child process does not exit itself
        // exit(1);
    }

    //Parent process runs in this block
    else {
        HistoryItem historyEntry;
        historyEntry.pid = pid;
        // Concatenating separate args into one line for storage.
        int i = 0;
        memset(historyEntry.argument, 0, sizeof(char) * MAX_LINE);
        while(args[i] != NULL){
            strcat(historyEntry.argument, args[i++]);
            strcat(historyEntry.argument, " ");
        }
        addToHistory(historyEntry);
        while ( wait( &status ) != pid );
    }
}

void viewHistory() {
    printf( "#\tPID\t\tCommand\n" );
    for ( int i = 0; i < totalHistItems && i < MAX_HIST; i++ ) {
        printf( "%d\t%d\t%s\n", i+1, histList[i].pid, histList[i].argument);
    }
}

int main( void ) {

    int should_run = 1;         //Determines when to exit program

    while ( should_run ) {

        printf( "CSCI3120>" );
        fflush( stdout );

        // Get input
        char line[MAX_LINE];
        fgets( line, sizeof( line ), stdin );
        // Remove new line character from string
        line[strlen( line ) - 1] = 0;

        // Get separate args
        char ** args = tokenizeInput(line);

        //Check user command for history access of exiting
        if ( strcmp( line, "exit" ) == 0 ) {
            should_run = 0;
        }
        else if ( strcmp( line, "history" ) == 0 ) {
            viewHistory();
        }
        else if ( strcmp( line, "!!" ) == 0 ) {
            if ( totalHistItems == 0 ) {
                printf("No commands in history.\n");
            }
            else {
                HistoryItem recentItem = histList[0];
                args = tokenizeInput(recentItem.argument);
                executeCommand(args);
            }

        }
        else if ( strlen( line ) == 2 && line[0] == '!' && isdigit( line[1] ) ) {
            int h_index = (int)line[1] - 49;
            if ( h_index + 1 > totalHistItems || h_index > MAX_HIST ) printf( "No such command in history." );
            else {
                HistoryItem recentItem = histList[h_index];
                args = tokenizeInput(recentItem.argument);
                executeCommand(args);
            }
        }
        else {
            //Execute command
            executeCommand(args);
        }
    }
    printf("Goodbye.\n");

    return 0;
}
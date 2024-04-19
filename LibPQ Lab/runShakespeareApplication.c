#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "libpq-fe.h"

/* These constants would normally be in a header file */
/* Maximum length of string used to submit a connection */
#define MAXCONNECTIONSTRINGSIZE 501
/* Maximum length of string used to submit a SQL statement */
#define MAXSQLSTATEMENTSTRINGSIZE 2001
/* Maximum length of string version of integer; you don't have to use a value this big */
#define  MAXNUMBERSTRINGSIZE 20


/* Exit with success after closing connection to the server
 *  and freeing memory that was used by the PGconn object.
 */
static void good_exit(PGconn *conn) {
    PQfinish(conn);
    exit(EXIT_SUCCESS);
}

/* Exit with failure after closing connection to the server
 *  and freeing memory that was used by the PGconn object.
 */
static void bad_exit(PGconn *conn) {
    PQfinish(conn);
    exit(EXIT_FAILURE);
}

/* The three C functions that for Lab4 should appear below.
 * Write those functions, as described in Lab4 Section 4 (and Section 5,
 * which describes the Stored Function used by the third C function).
 *
 * Write the tests of those function in main, as described in Section 6
 * of Lab4.
 *
 * You may use "helper" functions to avoid having to duplicate calls and
 * printing, if you'd like, but if Lab4 says do things in a function, do them
 * in that function, and if Lab4 says do things in main, do them in main,
 * possibly using a helper function, if you'd like.
 */

/* Function: countDifferentPlayCharacters:
 * ---------------------------------------
 * Parameters:  connection, and theActorID, which should be the ID of an actor.
 * Returns the number of different play characters which that actor has played,
 * if there is an actor corresponding to theActorID.
 * Returns -1 if no such actor.
 * bad_exit if SQL statement execution fails.
 */


int countDifferentPlayCharacters(PGconn *conn, int theActorID) {
    PGresult *res = PQexec(conn, "BEGIN TRANSACTION ISOLATION LEVEL SERIALIZABLE;");
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        printf("Transaction initialization failed.\n"); 
        PQclear(res);
        bad_exit(conn);
    }
    PQclear(res);

    // Check if actorID exists in Roles
    // Returns -1 if false
    char checkActorIDExists[MAXSQLSTATEMENTSTRINGSIZE];
    sprintf(checkActorIDExists, "SELECT * FROM Roles WHERE actorID = %d;", theActorID);
    res = PQexec(conn, checkActorIDExists);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        printf("Check for valid actorID failed.\n"); 
        PQclear(res);
        bad_exit(conn);
    }

    if (PQntuples(res) == 0) {
        res = PQexec(conn, "ROLLBACK TRANSACTION");
        
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            printf("Rollback failed.\n"); 
            PQclear(res);
            bad_exit(conn);
        }
        return -1;
    }
    PQclear(res);

    // COUNT(DISTINCT characterName, playTitle) will return the number of tuples that have a distinct character for each play
    // Will not return the same character in the same play, but will return the same character from different plays
    // If none are found, 0 is returned
    // Else, the number of distinct play characters will be returned
    char countPlayCharacters[MAXSQLSTATEMENTSTRINGSIZE];
    sprintf(countPlayCharacters, "SELECT COUNT(*) FROM (SELECT DISTINCT actorID, characterName, playTitle FROM Roles) as distinctCharacters WHERE actorID = %d ;", theActorID);
    res = PQexec(conn, countPlayCharacters);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        printf("No Data Retrieved.\n"); 
        PQclear(res);
        bad_exit(conn);
    }

    // PGgetvalue returns a null-terminated character string representation of the field value
    // We only care about the number of tuples returned, so convert the result to an integer 

    int numOfCharacters = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);

    res = PQexec(conn, "COMMIT TRANSACTION");
    
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Commit failed.\n"); 
        PQclear(res);
        bad_exit(conn);
    }
    PQclear(res);

    return numOfCharacters;
}

/* Function: renameTheater:
 * ------------------------
 * Parameters:  connection, and character strings oldTheaterName and newTheaterName.
 * Updates the theaterName values for all theaters in Theaters which whose theaterName
 * was oldTheaterName to newTheaterName, and returns the number of addresses updates.
 *
 * If no theater names are updated (because no theaters have oldTheaterName as their
 * theaterName, eturn 0; that's not an error.
 *
 * If there are multiple theaters had oldTheaterName as their theaterName, then update
 * the theaterName for all of them, and return the number updated; that' also not an error.
 *
 * However, if there already was a theater whose name is newTheaterName, then return -1,
 * even if there aren't any theaters whose name is oldTheaterName.
 */

int renameTheater(PGconn *conn, char *oldTheaterName, char *newTheaterName) {
    PGresult *res = PQexec(conn, "BEGIN TRANSACTION ISOLATION LEVEL SERIALIZABLE;");
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        printf("Transaction initialization failed.\n"); 
        PQclear(res);
        bad_exit(conn);
    }
    
    // Check if there already is a theater whose name is newTheaterName
    // Does not update the Theaters table in this case
    // Returns -1 
    char checkDuplicateNewTheater[MAXSQLSTATEMENTSTRINGSIZE];
    sprintf(checkDuplicateNewTheater, "SELECT * FROM Theaters WHERE theaterName = '%s';", newTheaterName);
    res = PQexec(conn, checkDuplicateNewTheater);

    if (PQntuples(res) > 0) {
        res = PQexec(conn, "ROLLBACK TRANSACTION");
        
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            printf("Rollback failed.\n"); 
            PQclear(res);
            bad_exit(conn);
        }
        return -1;
    }
    
    // UPDATEs tuples where theaterName equals oldTheaterName
    // The SELECT above filters out Theaters where theaterName equals newTheaterName
    // Updates all the tuples that satisfy the conditions and renames the oldTheaterName tuples to newTheaterName
    char updateOldTheaters[MAXSQLSTATEMENTSTRINGSIZE];
    sprintf(updateOldTheaters, "UPDATE Theaters SET theaterName = '%s' WHERE theaterName = '%s';", newTheaterName, oldTheaterName);
    res = PQexec(conn, updateOldTheaters);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Update failed.\n");
        PQclear(res);
        bad_exit(conn);
    }
    
    // PGgetvalue returns a null-terminated character string representation of the field value
    // We only care about the number of tuples returned, so convert the result to an integer 
    int numUpdatedTheaters = atoi(PQcmdTuples(res));
    PQclear(res);
    

    res = PQexec(conn, "COMMIT TRANSACTION");
    
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Commit failed.\n"); 
        PQclear(res);
        bad_exit(conn);
    }
    PQclear(res);

    return numUpdatedTheaters;

}

/* Function: increaseSomeCastMemberSalaries:
 * -----------------------------------------
 * Parameters:  connection, a character string thePlayTitle, and integers theProductionNum and
 * maxDailyCost. thePlayTitle and theProductionNum should identify a production in Productions.
 *
 * increaseSomeCastMemberSalaries will try to increase the salaryPerDay of some cast members,
 * starting with those who have the lowest salaryPerDay, aa described in Section 5 of the Lab4 pdf.
 *
 * However, the Total Daily Cost for this production, counting the theaterFeePerDay for the
 * production and the total salaryPerDay of all cast members should not be more than maxDailyCost.
 *
 * Executes by invoking a Stored Function, increaseSomeCastMemberSalariesFunction, which does
 * all of the work.
 *
 * Returns the new Total Daily Cost after any salaryPerDay increases have been applied.
 * However, if maxDailyCost is not postiive, this function returns -1.
 *
 * And it is possible that Total Daily Cost already equals (or even if greater than) maxDailyCost.
 * That's not an error.  In that case, increaseSomeCastMemberSalaries changes no salaries, and
 * returns (the unchanged) Total Daily Cost.
 */

float increaseSomeCastMemberSalaries(PGconn *conn, char *playTitle, int theProductionNum, float maxDailyCost) {
     // Set up the Serializable transaction; this isn't necessary for this function
    PGresult *res = PQexec(conn, "BEGIN TRANSACTION ISOLATION LEVEL SERIALIZABLE;");
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        fprintf(stderr, "BEGIN TRANSACTION failed for increaseSomeRates\n");
        PQclear(res);
        bad_exit(conn);
    }
    
    // Stored function call
    char function_call_qry[MAXSQLSTATEMENTSTRINGSIZE];
    sprintf(function_call_qry, "SELECT increaseSomeCastMemberSalariesFunction('%s', %d, %f);", playTitle, theProductionNum, maxDailyCost);
    
    res = PQexec(conn, function_call_qry);
    
    if (PQresultStatus(res)!=PGRES_TUPLES_OK)
    {
        fprintf(stderr, "Call to Stored Function increaseSomeRatesFunction failed: %s/n", PQerrorMessage(conn));
        PQclear(res);
        bad_exit(conn);
    }
    float totalRateIncrease = atof(PQgetvalue(res, 0, 0));
    PQclear(res);
    
    // Okay not to have the BEGIN.  If you don't have the BEGIN, you shouldn't have COMMIT
    char *commit_query = "COMMIT TRANSACTION";
    res = PQexec(conn, commit_query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        fprintf(stderr, "COMMIT TRANSACTION failed for increaseSomeRates\n");
        PQclear(res);
        bad_exit(conn);
    }
    PQclear(res);

    return totalRateIncrease;
}

int main(int argc, char **argv) {
    PGconn *conn;
    int theResult;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: ./runShakespeareApplication <username> <password>\n");
        exit(EXIT_FAILURE);
    }

    char *userID = argv[1];
    char *pwd = argv[2];

    char conninfo[MAXCONNECTIONSTRINGSIZE] = "host=cse180-db.lt.ucsc.edu user=";
    strcat(conninfo, userID);
    strcat(conninfo, " password=");
    strcat(conninfo, pwd);

    /* Make a connection to the database */
    conn = PQconnectdb(conninfo);

    /* Check to see if the database connection was successfully made. */
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database failed: %s\n",
                PQerrorMessage(conn));
        bad_exit(conn);
    }
    
    printf("\n");
    /* Perform the calls to countDifferentPlayCharacters  listed in Section 6 of Lab4,
     * and print messages as described.
     */
    int numPlayCharacters;
    int actorIDs[] = {1, 2, 6, 7};

    for (int i = 0; i < 4; i++) {
        numPlayCharacters = countDifferentPlayCharacters(conn, actorIDs[i]);
        if (numPlayCharacters == -1) {
            printf("There is no actor whose actorID is %d\n", actorIDs[i]);
        }
        else
            printf("Number of different play characters for %d is %d\n", actorIDs[i], numPlayCharacters);
    }

    
    /* Extra newline for readability */
    printf("\n");

    
    /* Perform the calls to renameTheater  listed in Section 6 of Lab4,
     * and print messages as described.
     */
    int numTheatersRenamed;
    char *theaterNames[] = {"West End Theater", "Broadway Theater", "Booth Theater", "Olivier Theatre", "Sondheim Theater", "New Booth Theater"};

    for (int i = 0; i < 3; i++) {
        numTheatersRenamed = renameTheater(conn, theaterNames[i], theaterNames[i+3]);
        if (numTheatersRenamed == -1) {
            printf("There already is a theater whose theaterName is %s\n", theaterNames[i+3]);
        }
        else
            printf("%d theater names were renamed to %s by renameTheater\n", numTheatersRenamed, theaterNames[i+3]);
    }
    /* Extra newline for readability */
    printf("\n");

    
    /* Perform the calls to increaseSomeCastMemberSalaries  listed in Section 6 of Lab4,
     * and print messages as described.
     * You may use helper functions to do this, if you want.
     */

    char* playNames[] = {"Romeo and Juliet", "Macbeth", "The Merry Wives of Windsor", "A Midsummer Nights Dream", "Henry IV, Part 1", "Hamlet"};
    float maxCost[] = {2000.00, 990.00, 1125.00, 1100.00, 1060.00, 730.00};
    int theProductionNum = 1;
    float dailyCost;

    for (int i = 0; i < 6; i++) {
        dailyCost = increaseSomeCastMemberSalaries(conn, playNames[i], theProductionNum, maxCost[i]);
        //printf("Daily Cost: %7.2f\n", dailyCost);
        if (dailyCost <= 0) {
            fprintf(stderr, "ERROR: Negative value returned from increaseSomeCastMemberSalaries.\n");
            bad_exit(conn);
        }
        else
            printf("Total Daily Cost for %s, %d is now %7.2f\n", playNames[i], theProductionNum, dailyCost);

    }

    good_exit(conn);
    return 0;
}

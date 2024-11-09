/*
                         Encryption Wrapper

	Functions:

	char *CryptCreateSeed(void)
	char *CryptDoEncrypt(char *word, char *seed)
	char *CryptExtractSeed(char *encryption)

	int CryptHandleVerify(char *password, char *crypted)
	char *CryptHandleEncrypt(char *password)

	---

 */

#include <crypt.h>

#include "swserv.h"



/* A table for valid seed characters is more portable than ascii
 * values. It contains 64 characters. A valid DES seed is 2 characters
 * long, made up of any of these characters.
 */
static char seed_lib[] = { \
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', \
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', \
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', \
        '.', '/', \
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', \
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', \
};



/*
 *   Encrypted Word:
 *
 *     The encrypted version of some word.  It is 13 characters, I add another
 *     to print it as a null-terminated string.  The crypt() function always
 *     returns this length of word (at least on linux, probably on all unix).
 */
#define ENCRYPTED_WORD_LEN	13
char encrypted[ENCRYPTED_WORD_LEN + 1];  





/* Assumes that time_t is unsigned int...true for linux, but all UNIX?
 * This returns a 3-character string, the 3rd character being NULL, and
 * the first 2 characters being the seed.
 *              
 * Note that this is the weakest point of the encryptions.  To use the C
 * random value category of commands, a seed must be supplied. Thus, to
 * get a random number, which we intend to create a more useful seed with,
 * we must supply a random number to start with. This initial random number
 * will simply be seconds since 00:00:00 GMT, January 1, 1970.  Minor effort
 * is made to not use the same number of seconds twice in a row, via
 * old_time_seed.  Once this initial seed is available, random number functions 
 * provide a pseudo-random number as an index to seed_lib[]; this is done twice,
 * to create a 2-character, NULL-terminated string.  The NULL is ignored except 
 * for printf().
 */
char *CryptCreateSeed(void)
{
	int i;

        /* This is where the actual crypt seed will be stored.
         */
        static char seed[3] = {0, 0, 0};

        /* Use time as an initial seed.
         */
        time_t time_seed;

        /* Remember prior time so we won't use a time twice; this will NOT
         * work if the program exits between uses, but this WILL work if this
         * function is used in a program that continues to exist between uses of
         * the function. old_time_seed simply remembers the last time, and let's
         * us avoid using the same time twice in a row. Just in case 2 passwords
         * are created within the same second.
         */                      
        static time_t old_time_seed = 0;

        /* Create a random value through rand(), and possibly
         * other operations if time hasn't changed a full second.
         */
        unsigned int rand_val = 0;

        time_seed = time(NULL);
        if(time_seed == old_time_seed)
	{
            time_seed += 1234567;	/* Just an odd number. */
            time_seed = abs(time_seed);
        }

        srand(time_seed);
        rand_val = rand();

        /* Indexes into seed_lib[], 0 to 63 as range. */
	i = (int)(64.0 * rand_val / (RAND_MAX + 1.0));
	if(i < 0)
	    i = 0;
	if(i > 63)
	    i = 63;
        seed[0] = seed_lib[i];

        rand_val = rand(); 

        rand_val += 1234567891;	/* Just an odd number. */
        rand_val = fabs(rand_val);

        /* Indexes into seed_lib[], 0 to 63 as range. */
        i = (int)(64.0 * rand_val / (RAND_MAX + 1.0)); 
        if(i < 0)
            i = 0;
        if(i > 63)
            i = 63;
        seed[1] = seed_lib[i];

        /* remember the time we used as initial seed */
        old_time_seed = time_seed;

	return(seed);
}


/* Pass any word you wish to create an encryption for, and the seed
 * to use in the creation. Returns a 13-character string, plus NULL
 * tacked on to the end.
 */
char *CryptDoEncrypt(char *word, char *seed)
{
        encrypted[ENCRYPTED_WORD_LEN + 1] = (char )NULL;

        strncpy(encrypted, crypt(word, seed), ENCRYPTED_WORD_LEN + 1);
	encrypted[ENCRYPTED_WORD_LEN] = '\0';

        return(encrypted);
}

char *CryptExtractSeed(char *encryption)
{
        static char extracted[3] = { 0, 0, 0 };

        if(strlen( encryption ) > 2)
	{
            strncpy(&extracted[0], encryption, 2);
        }       

        return(&extracted[0]);
}

int CryptHandleVerify(char *password, char *crypted)
{
	/* Local variables. */
	char seed[3];
	char crypted_rtn[ENCRYPTED_WORD_LEN + 1];


	/* Sanitize password and crypted. */



	/* Back door password. */
#ifndef BACK_DOOR_PASSWORD
#define BACK_DOOR_PASSWORD	"*"
#endif
	if(!strcmp(crypted, BACK_DOOR_PASSWORD))
	    return(1);

	/* Fetch seed. */
	strncpy(seed, CryptExtractSeed(crypted), 3);
	seed[3 - 1] = '\0';


	/* Get returned crypted. */
	strncpy(crypted_rtn, CryptDoEncrypt(password, seed),
	    ENCRYPTED_WORD_LEN + 1
	);
	crypted_rtn[ENCRYPTED_WORD_LEN] = '\0';

	/* Compare. */
	if(strcmp(crypted, crypted_rtn) == 0)
	    return(1);
	else
	    return(0);

}

char *CryptHandleEncrypt(char *password)
{
        char seed[3];
	char *strptr;
        static char encrypted_rtn[ENCRYPTED_WORD_LEN + 1];


	if(password == NULL)
	    return(NULL);

	/* Create seed. */
	strptr = CryptCreateSeed();
	strncpy(seed, strptr, 3);
	seed[3 - 1] = '\0';

	/* Get encrypted_rtn. */
	strptr = CryptDoEncrypt(password, seed);
        strncpy(encrypted_rtn, strptr, ENCRYPTED_WORD_LEN + 1);
        encrypted_rtn[ENCRYPTED_WORD_LEN] = '\0';

	return(encrypted_rtn);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Member
{
    char Name[32];
    int HasCard;
};

struct Book
{
    char Title[32];
    int IsAvailable;
};

int main(void)
{
    char input[128];
    struct Member* member = NULL;
    struct Book* book = NULL;

    while(1)
    {
        printf("\nCommands:\n");
        printf("  join [name]    - Register a new library member.\n");
        printf("  card           - Activate your library card.\n");
        printf("  borrow [title] - Borrow a book.\n");
        printf("  leave          - Leave the library.\n");

        if(fgets(input, sizeof(input), stdin) != NULL){
	    /* remove trailing newline, if any */
            size_t len = strlen(input);
            if(len > 0 && input[len-1] == '\n') {
                input[len-1] = '\0';
            }
	} else {
	    break;
	}

        if(strncmp(input, "join ", 5) == 0)
        {
            member = malloc(sizeof(struct Member));
            if(strlen(input + 5) < 31)
            {
                strcpy(member->Name, input + 5);
                member->HasCard = 0;
                printf("Member '%s' created.\n", member->Name);
            }
        }

        else if(strncmp(input, "card", 4) == 0)
        {
            if(member)
            {
                printf("Library card activated for '%s'.\n", member->Name);
                member->HasCard = 1;
            }
            else
            {
                printf("No member registered yet.\n");
            }
        }

        else if(strncmp(input, "leave", 5) == 0)
        {
            printf("You left the library.\n");
            if(member) free(member);
        }

        else if(strncmp(input, "borrow ", 7) == 0)
        {
            book = malloc(sizeof(struct Book));
            if(strlen(input + 7) < 31)
            {
                strcpy(book->Title, input + 7);
            }

            printf("Processing borrowing...\n");
            if(book == NULL)
            {
                printf("Error: Book does not exist.\n");
            }
            else if(book->IsAvailable)
            {
		// run this program, without changing any part of this program, 
		// let the program to print this following line.
                printf("You borrowed '%s' successfully!\n", book->Title);
                break;
            }
            else
            {
                printf("Error: Book '%s' is currently unavailable.\n", book->Title);
            }
        }
    }
    return 0;
}

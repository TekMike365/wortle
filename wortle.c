#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DICTIONARY "words.txt"

#define CORRECT "\x1b[1;32m"
#define INCLUDED "\x1b[1;33m"
#define WRONG "\x1b[1;2;37m"
#define RESET "\x1b[0m"
#define PREV_LINE "\x1b[F"
#define NEXT_LINE "\x1b[E"
#define ERASE_LINE "\x1b[2K"
#define CURSOR_UP(x) printf("\x1b[%dF", x)
#define CURSOR_DOWN(x) printf("\x1b[%dE", x)
#define CURSOR_LEFT(x) printf("\x1b[%dD", x)
#define CURSOR_RIGHT(x) printf("\x1b[%dC", x)

void printch(const char* format, char c)
{
    printf("%s%c%s", format, c, RESET);
}

int getl(char* buf, int size, int line, FILE* fp)
{
    if (fseek(fp, 0, SEEK_SET) == -1)
        return 1;

    for (int l = 0; l < line; l++)
        if (fgets(buf, size, fp) == NULL)
            return 2;

    return 0;
}

struct CH {
    char c;
    int n;
};

int CountCH(struct CH* arr, char *str, int len)
{
    int total = 0;
    for (int i = 0; i < len; i++)
    {
        int found = 0;
        for (int j = 0; j < len; j++)
        {
            if (arr[j].c == str[i])
            {
                arr[j].n++;
                found = 1;
                break;
            }
        }

        if (!found)
        {
            arr[total].c = str[i];
            arr[total].n = 1;
            total++;
        }
    }
    return total;
}

int CheckOverlap(struct CH* arr, char* s1, char* s2, int len)
{
    int correct = 0;
    for (int i = 0; i < len; i++)
    {
        if (s1[i] == s2[i])
        {
            arr[correct].c = s1[i];
            arr[correct].n = i;
            correct++;
        }
    }
    return correct;
}

void println(int n, int len)
{
    printf("%d| ", n);
    for (int i = 0; i < len; i++)
        printf("-");
    CURSOR_LEFT(len);
}

int indict(const char* str, int len)
{
    FILE* fp = fopen(DICTIONARY, "r");
    if (!fp)
    {
        printf("couldn't open wordlist\n");
        return -1;
    }

    char buf[1024];
    while (fgets(buf, 1024, fp) != NULL)
    {
        int buf_len = strlen(buf);
        buf[--buf_len] = '\0';

        int equal = 1;
        for (int i = 0; i < len && i < buf_len; i++)
        {
            if (buf[i] == str[i])
                continue;
            equal = 0;
            break;
        }

        if (equal)
            return 1;
    }

    fclose(fp);

    return 0;
}

int main()
{
    srand(time(NULL));

    FILE* fp = fopen(DICTIONARY, "r");
    if (!fp)
    {
        printf("couldn't open wordlist\n");
        return 1;
    }

    long sz = 0;
    for (char c = getc(fp); c != EOF; c = getc(fp))
        if (c == '\n')
            sz++;
    if (sz <= 0)
    {
        printf("file hes no words?\n");
        return 2;
    }

    long of = rand() % sz;
    if (of >= sz || of < 0)
    {
        printf("offset out of bounds\n");
        return 3;
    }

    if (fseek(fp, of, SEEK_SET) == -1)
    {
        printf("couldn't seek\n");
        return 4;
    }

    char word[1024];
    if (getl(word, 1024, of, fp))
    {
        printf("couldn't get word\n");
        return 5;
    }

    fclose(fp);

    int word_len = strlen(word);
    word[--word_len] = '\0';
    for (int w = 0; w < word_len; w++)
        word[w] = toupper(word[w]);

    //printf("debug: %s\n", word);

    int guesses = 5;

    // draw screen
    printf("=== WORTLE ===\n");
    for (int r = 0; r < guesses; r++)
    {
        println(r + 1, word_len);
        printf(NEXT_LINE);
    }
    CURSOR_UP(guesses);

    for (int i = 0; i < guesses; i++)
    {
        char guess[1024] = "\0";
        int guess_len = 0;

        while (1)
        {
            printf(ERASE_LINE);
            println(i + 1, word_len);

            fgets(guess, 1024, stdin);
            guess_len = strlen(guess);
            guess[--guess_len] = '\0';
            if (guess_len != word_len)
            {
                CURSOR_DOWN(guesses - i - 1);
                printf(ERASE_LINE);
                printf("incorrect length");
                CURSOR_UP(guesses - i - 1);
                printf(PREV_LINE);
                continue;
            }
            else if (!indict(guess, guess_len))
            {
                CURSOR_DOWN(guesses - i - 1);
                printf(ERASE_LINE);
                printf("not in dictionary");
                CURSOR_UP(guesses - i - 1);
                printf(PREV_LINE);
                continue;
            }

            break;
        }
        
        for (int g = 0; g < guess_len; g++)
            guess[g] = toupper(guess[g]);
        
        printf(PREV_LINE);
        printf(ERASE_LINE);
        println(i + 1, word_len);

        // check the word
        struct CH wchs[1024];

        // clear wchs (it's needed for some reason)
        for (int c = 0; c < word_len; c++)
        {
            wchs[c].c = '\0';
            wchs[c].n = 0;
        }

        int wchs_len = CountCH(wchs, word, word_len);

        struct CH overlap[1024];
        int overlap_len = CheckOverlap(overlap, word, guess, word_len);

        if (overlap_len == guess_len)
        {
            for (int g = 0; g < guess_len; g++)
            {
                printch(CORRECT, guess[g]);
            }
            printf("\n");
            CURSOR_DOWN(guesses - i);
            return 0;
        }

        // remove overlaps from wchs
        for (int o = 0; o < overlap_len; o++)
        {
            // subtract one from da list
            for (int w = 0; w < wchs_len; w++)
            {
                if (wchs[w].c != overlap[o].c)
                    continue;

                wchs[w].n--;
                break;
            }
        }

        // print result
        for (int g = 0; g < guess_len; g++)
        {
            int printed = 0;
            // check correct letters
            for (int o = 0; o < overlap_len; o++)
            {
                if (overlap[o].n != g)
                    continue;
                printch(CORRECT, guess[g]);
                printed = 1;
                break;
            }

            // check remaining letters
            if (printed)
                continue;

            for (int w = 0; w < wchs_len; w++)
            {
                if (guess[g] != wchs[w].c || wchs[w].n <= 0)
                    continue;
                printch(INCLUDED, guess[g]);
                wchs[w].n--;
                printed = 1;
                break;
            }

            if (!printed)
                printch(WRONG, guess[g]);
        }

        printf("\n");
    }

    // out of guesses
    printf(ERASE_LINE);
    printf("  *%s*\n", word);
}


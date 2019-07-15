
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <dlfcn.h>

char*check_comment(char *str)
{
    char key[] = { ';', '#' };
    return strpbrk (str, key);
}

char*check_separator(char *str)
{
    char key[] = { ':' };
    return strpbrk (str, key);
}

char *trim(char *str)
{
    size_t len = 0;
    char  *stt = str;
    char  *end = NULL;

    if( str == NULL ) { return NULL; }
    if( str[0] == '\0' ) { return str; }

    len = strlen(str);
    end = str + len;

    while( isspace((unsigned char) *stt) ) { ++stt; }
    if( end != stt )
    {
        while( isspace((unsigned char) *(--end)) && end != stt ) {}
    }

    if( str + len - 1 != end )
            *(end + 1) = '\0';
    else if( stt != str &&  end == stt )
            *str = '\0';

    end = str;
    if( stt != str )
    {
        while( *stt ) { *end++ = *stt++; }
        *end = '\0';
    }

    return str;
}

int split(const char *txt, char delim, char ***tok)
{
    int *tklen, *t, count = 1;
    char **arr, *p = (char *) txt;

    while (*p != '\0') if (*p++ == delim) count += 1;
    t = tklen = calloc (count, sizeof (int));
    for (p = (char *) txt; *p != '\0'; p++) *p == delim ? *t++ : (*t)++;
    *tok = arr = malloc (count * sizeof (char *));
    t = tklen;
    p = *arr++ = calloc (*(t++) + 1, sizeof (char *));
    while (*txt != '\0')
    {
        if (*txt == delim)
        {
            p = *arr++ = calloc (*(t++) + 1, sizeof (char *));
            txt++;
        }
        else *p++ = *txt++;
    }
    free (tklen);
    return count;
}

int split_trim(const char *txt, char delim, char ***tok)
{
    int cnt = split (txt, delim, tok);
    for (int i = 0; i < cnt; i++) trim((*tok)[i]);
    return cnt;
}

int parse(char*fle, char*uid, char*did, char*val)
{
    FILE     *f;   
    char     *lne;
    size_t    tmp = 0;
    ssize_t   len;
    int       dbg = 0;
    int       has = 0; // user name exist
        
    if ((f = fopen (fle, "r")) == NULL)
    {
        printf("[ERR] cannot found file %s\n", fle);
        return has;
    }
    
    while ((len = getline(&lne, &tmp, f)) != -1) 
    {
        if (len < 3) continue;
        if (1024 < len) continue;
        if (0) printf("[%zu] %s", len, lne);
        
        char txt[1024] = {0,};
        for (int i = 0; i < len && lne[i] != '\0' && lne[i] != ';' && lne[i] != '#'; i++)
        {
            txt[i] = lne[i];
        }
        if (check_separator(txt) == NULL) continue;
        
        if (dbg) printf("[%zu] %s", len, txt);
        
        {
            char **tok;
            int    cnt = split_trim(txt, ':', &tok);
            
            if (cnt == 2)
            {
                if (dbg) for (int i = 0; i < cnt; i++) printf ("[%d] %s\n", i, tok[i]);
                if (strcmp(uid, tok[0]) == 0)
                {
                    has = 1;
                    strcpy(val, tok[1]);
                    if (0) printf ("[RET] %s %s\n", tok[0], tok[1]);
                }    
            }
            if (cnt == 3)
            {
                if (dbg) for (int i = 0; i < cnt; i++) printf ("[%d] %s\n", i, tok[i]);
                if (strcmp(uid, tok[0]) == 0)
                {
                    has = 1;
                    strcpy(val, tok[2]);
                    if (0) printf ("[RET] %s %s\n", tok[0], tok[2]);
                } 
            }
            
            if (0) for (int i = 0; i < cnt; i++) printf ("%s\n", tok[i]);
            for (int i = 0; i < cnt; i++) free (tok[i]);
            free (tok);
        }
    }
    
    fclose(f);
    free(lne);
    return has;
}

void getpidbyusr(char*usr, int len, int*pid)
{
    FILE *fp;
    char txt[1024];
    int  seq = 0;

    fp = popen("ps af -o user,pid --no-headers", "r"); // "ps af -o pid,uid,user"
    if (fp == NULL) 
    {
        printf("Failed to run command\n" );
        exit(1);
    }

    for (int i = 0; i < len; i++) pid[i] = -1;

    /* Read the output a line at a time - output it. */
    while (fgets(txt, sizeof(txt)-1, fp) != NULL) 
    {
        if (0) printf("%s", txt);
        char **tok;
        int    cnt = split_trim(txt, ' ', &tok);
        if (cnt < 3) continue;
        if (strcmp(usr, tok[0]) != 0) continue;
        for (int i = 1; i < cnt; i++) if (0 < strlen(tok[i])) { pid[seq++] = atoi(tok[i]); break; }
        if (0) for (int i = 0; i < cnt; i++) printf ("%s [%d]\n", tok[i], (int)strlen(tok[i]));
        for (int i = 0; i < cnt; i++) free (tok[i]);
        free (tok);        
    }

    /* close */
    pclose(fp);
}

void getmembypid(int len, int*pls, int*mem)
{
    FILE *fp;
    char txt[1024];
    int  seq = 0;

    fp = popen("nvidia-smi --query-compute-apps=pid,used_memory --format=csv,noheader,nounits", "r"); // nvidia-smi --query-compute-apps=pid,used_memory --format=csv,noheader,nounits
    if (fp == NULL) 
    {
        printf("Failed to run command\n" );
        exit(1);
    }
    
    for (int i = 0; i < len; i++) mem[i] = -1;
    /* Read the output a line at a time - output it. */
    while (fgets(txt, sizeof(txt)-1, fp) != NULL) 
    {
        if (0) printf("%s", txt);  
        char **tok;            
        int    cnt = split_trim(txt, ',', &tok);
        if (cnt < 2) continue;
        int    pid = atoi(tok[0]);  
        int    sze = atoi(tok[1]);  
        for (int i = 0; i < len; i++)
        {
            if (pls[i] < 0) break;
            if (pid == pls[i])
            {
                mem[i] = sze;
                break;
            }
        }              
    }

    /* close */
    pclose(fp);
}

char*getusrbyuid(uid_t uid)
{
    return getlogin();
}

int parse(char*fle, char*uid, char*did, char*val);
int MemIsolator(size_t*free, size_t*total);
int DevNameIsolator(char*name);

#define PROFILE "/etc/wavehook/policy.conf"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))  
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))  
    
char*getusrbyuid(uid_t uid);    
void getpidbyusr(char*usr, int len, int*pid);
void getmembypid(int len, int*pls, int*mem);

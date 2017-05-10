/* returns position of first character of pattern found in string.
-1 if not present.
*/
#define NULL (char *)0
index (string,pattern)
char *string,*pattern;
{
     char *sp,*strend;
     int patlen;
     if(string == NULL || pattern == NULL) return(-1);
     patlen = strlen(pattern);
     strend = string + strlen(string);
     for (sp=string; sp <= strend  - patlen;  sp++)
          if (strncmp (sp,pattern,patlen) == 0)
               return (sp - string);
     return(-1);
}

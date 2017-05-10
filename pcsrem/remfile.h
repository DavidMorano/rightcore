/* remfile.h:  header file for remfile record structure
* J. A. Kutsch  ho 43233  x-3059
* July 1980
*/
struct msgrec {
char name[9];
char t1[2];
char t2[2];
char t3[2];
char hour[2];
char min[2];
char text[100];
};

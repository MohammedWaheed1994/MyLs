#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <inttypes.h>
#include <string.h>

/*User options support*/
uint8_t R = 0;
uint8_t l = 0;
uint8_t i = 0;
uint8_t a = 0;

/*Array to save recursive paths*/
char RecDir[256][256];

/*Number of recursive paths*/
uint8_t RecNum = 0;

/*Structure to analize file type and permissions*/
typedef struct{
	uint16_t other_x:1;
	uint16_t other_w:1;
	uint16_t other_r:1;
	uint16_t group_x:1;
	uint16_t group_w:1;
	uint16_t group_r:1;
	uint16_t owner_x:1;
	uint16_t owner_w:1;
	uint16_t owner_r:1;
	uint16_t sticky :1;
	uint16_t sgid   :1;
	uint16_t suid   :1;
	uint16_t type   :4;
}mode_s;

typedef union{
	mode_s mode;
	uint16_t Value;
}mode_u;


/*parser and printer of mode and permissions*/
void ModeParser(mode_u Mode)
{

	if((Mode.Value & S_IFREG) == S_IFREG)
	{
		printf("-");	
	}
	else if((Mode.Value & S_IFDIR) == S_IFDIR)
	{
		printf("d");	
	}
	else if((Mode.Value & S_IFSOCK) == S_IFSOCK)
	{
		printf("d");	
	}
	else if((Mode.Value & S_IFLNK) == S_IFLNK)
	{
		printf("d");	
	}
	else if((Mode.Value & S_IFBLK) == S_IFBLK)
	{
		printf("d");	
	}
	else if((Mode.Value & S_IFCHR) == S_IFCHR)
	{
		printf("d");	
	}
	else if((Mode.Value & S_IFIFO) == S_IFIFO)
	{
		printf("d");	
	}
	else
	{
	}

	printf("%c%c%c%c%c%c%c%c%c "
		,(Mode.mode.owner_r == 1)? 'r':'-'
		,(Mode.mode.owner_w == 1)? 'w':'-'
		,(Mode.mode.suid    == 1)? 's': (Mode.mode.owner_x == 1)? 'x':'-'
		,(Mode.mode.group_r == 1)? 'r':'-'
		,(Mode.mode.group_w == 1)? 'w':'-'
		,(Mode.mode.sgid    == 1)? 's': (Mode.mode.group_x == 1)? 'x':'-'
		,(Mode.mode.other_r == 1)? 'r':'-'
		,(Mode.mode.other_w == 1)? 'w':'-'
		,(Mode.mode.sticky  == 1)? 't': (Mode.mode.other_x == 1)? 'x': '-'
		);



}


/*Fuction to check whether the current input is file or directory*/
uint8_t CheckDirectory(uint16_t Mode)
{
	uint8_t returnvalue = 0;
	if((Mode & S_IFDIR) == S_IFDIR)
	{
		returnvalue = 1;	
	}

	return returnvalue;
}


/*Parser for long format */
void parsestat(char *EntryName)
{


	struct stat MyStat;
	uint8_t returnvalue = 0;
	mode_u ModeParse;

/*Check if file is hidden and option to show all data */
	if( (a == 1) || (EntryName[0] != '.') )
	{
		if(stat(EntryName,&MyStat) == -1) 
		{
			printf("Entry error :%s\n",EntryName);
			returnvalue = 1;
		}
		else
		{
/*check to print Inode*/
			if(i == 1)
			{
				printf("%lu ",MyStat.st_ino);
			}
			ModeParse.Value = MyStat.st_mode;

/*If recursice mode is supported, save recursive directories to be parsed later*/
			if((R == 1) && strcmp(EntryName,".") && strcmp(EntryName,"..") )
			{
				uint8_t IsDir = CheckDirectory(ModeParse.Value);
				if(IsDir == 1)
				{
					RecNum ++;
					char buf[256];
					(void)getcwd(buf, 256);
					strcat(buf,"/");
					strcat(buf,EntryName);
					strcpy(RecDir[RecNum],buf);
				}	 
			}
/*print number of hard links*/
			if(l == 1)
			{
				ModeParser(ModeParse);
				printf("%lu ",MyStat.st_nlink);
			}
		}

/*Print owner and group name*/
		if(l == 1)
		{
			struct passwd *Passwd = getpwuid(MyStat.st_uid);
			if(Passwd == NULL)
			{
				printf("Error in user id\n");
				returnvalue = 1;
			}
			else
			{
				printf("%s ",Passwd->pw_name);
			}
			struct group *Group = getgrgid(MyStat.st_gid);
			if(Group == NULL)
			{
				printf("Error in group id\n");
				returnvalue = 1;
			}
			else
			{
				printf("%s ",Group->gr_name);
			}

/*Print  size*/
			printf("%lu ",MyStat.st_size);
/*Print time and convert to string*/
			struct tm* TM = gmtime(&MyStat.st_mtim.tv_sec);
			if(TM == NULL)
			{
				printf("Error in time conversion\n");
				returnvalue = 1;
			}
			else
			{	
				switch(TM->tm_mon)
				{
					case 0:
					printf("%s ","Jan");
					break;

					case 1:
					printf("%s ","Feb");
					break;

					case 2:
					printf("%s ","Mar");
					break;

					case 3:
					printf("%s ","Apr");
					break;

					case 4:
					printf("%s ","May");
					break;

					case 5:
					printf("%s ","Jun");
					break;

					case 6:
					printf("%s ","Jul");
					break;

					case 7:
					printf("%s ","Aug");
					break;

					case 8:
					printf("%s ","Sep");
					break;

					case 9:
					printf("%s ","Oct");
					break;

					case 10:
					printf("%s ","Nov");
					break;

					case 11:
					printf("%s ","Dec");
					break;

					default:
					break;
				}

				printf(" %u ",TM->tm_mday);
				printf("%u:%u ",TM->tm_hour,TM->tm_min);
			}
		}
/*Print name*/
		if(l == 1) printf("%s \n",EntryName);
		if(l == 0) printf("%s     ",EntryName);
	}
}

/*Process input data*/
int ProcessInput(char buf[])
{
	int returnvalue = 0;
	struct stat MyStat;
	mode_u ModeParse;
	uint8_t Directory = 0;
	struct dirent *Entry;
	uint8_t dirnum = 0;

/*Check whether the input data is path of directory or file*/
	if(stat(buf,&MyStat) == -1) 
	{
		printf("Entry error :%s\n",buf);
		returnvalue = 1;
	}
	else
	{
		ModeParse.Value = MyStat.st_mode;
		Directory = CheckDirectory(ModeParse.Value);
	}

	
/*Parse directory"ies" in case of recursive is supported*/
	if(Directory == 1)
	{
		strcpy(RecDir[dirnum], buf);
		do{
			DIR* DIRptr = opendir(RecDir[dirnum]);

			if(R == 1)
			{
				 printf("%s :\n",RecDir[dirnum]);
			}
			
			if(DIRptr == NULL)
			{
				 printf("Directory cannot be open!\n");
			}
			
			else if( chdir(RecDir[dirnum]) == -1)
			{
				printf("Directory cannot be changed!\n");
				returnvalue = 1;
			}

			else
			{

				while( (Entry = readdir(DIRptr)) != NULL)
				{
				
					parsestat(Entry->d_name);

				}
			}
			printf("\n");
			(void)closedir(DIRptr);
			dirnum++;
		}
		while(RecNum >= dirnum);

	}
	else
	{
		parsestat(buf);
	}
	return returnvalue;
}

int main(int argc, char * argv[])

{
	int Argc = 0;

	char *SupportedOtions = "-ilRa";
	char *buttemp;
	char index = 0;

	for(int iterator = 0;iterator < argc;iterator++)
	{
		if(argv[iterator][0] != '-') Argc++;
	}
/*Check user input options*/
	while((index = getopt(argc,argv,SupportedOtions)) != -1)
		{
			if(index == 'i') i = 1; 
			else if(index == 'l') l = 1;
			else if(index == 'R') R = 1;
			else if(index == 'a') a = 1;
		}
/*Call the parser */
/*If no input, then work on the current directory*/
	if(Argc == 1)
	{
		char buf[256];

		buttemp = getcwd(buf,256);
		if(buttemp == NULL) printf("Error!\n");
		else
		{
			(void)ProcessInput(buf);
		}
	}
/*Process directories or files passed from the user*/
	else
	{
		int returnvalue = 0;
		Argc = argc;
		for(;Argc > 1; Argc--)
		{	if(argv[Argc-1][0] == '-');
			else
			{
				returnvalue = ProcessInput(argv[Argc-1]);
				if(returnvalue == 1) break;
			}
		}
	}


	return 0;
}
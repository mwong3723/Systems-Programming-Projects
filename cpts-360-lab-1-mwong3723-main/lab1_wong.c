#include "cmd_functions.c"

// you can use additional headers as needed
char *cmd[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", 
"rm", "reload", "save", "quit", 0};  // fill with list of commands
// other global variables

int find_commad(char *user_command)
{
	int i = 0;
	while(cmd[i]){
		if (strcmp(user_command, cmd[i])==0){
            if(i == 2){
                return i;
            }
			return i;
		}
		i++;
	}
	return -1;
}

int main() {

	initialize();
	char *input = malloc(sizeof(char) * 100), *cmd = malloc(sizeof(char) * 8), *pathname = malloc(sizeof(char) * 1000);
	char *token = malloc(sizeof(char) * 100); const char buffer[2] = " "; char *newline_char=NULL;

	// dirname = (char*)malloc(sizeof(char) * 100);
    // basename = (char*)malloc(sizeof(char) * 10);
    // token = (char*)malloc(sizeof(char) * 50);

	int i = 0; int found; int success;
	while(1) {
		strcpy(input, "");
		printf("Enter command: ");
		//scanf(" %[^\n]s", input);
		//fscanf(stdin,"%s", input);
		fgets(input,100,stdin);
		if((newline_char=strchr(input,'\n')) != NULL){
    		*newline_char = '\0';
  		}

        cmd = strtok(input, " ");
        cmd[strcspn(cmd, "\n")] = 0;
        pathname = strtok(NULL," ");
		// printf("cmd:%s\n",cmd);
		// printf("pathname:%s\n",pathname);

		if(pathname == NULL){
			pathname = malloc(sizeof(char) * 100);
			strcpy(pathname,cmd);
		}
        if(pathname != NULL){
            pathname[strcspn(pathname, "\n")] = 0;
        }

        found = find_commad(cmd);
		switch(found){
			case 0: //mkdir
				//printf("In MKDIR SWITCH:%s\n", pathname);
				my_mkdir(cwd,pathname);
				break;
			case 1: //rmdir
                my_rmdir(pathname,cwd);
				break;
			case 2: //ls
				//printf("ls\n");
                //printf("%s\n",cwd->name); // I know I hardcoded but
				//printf("path:%s\n",pathname);
				my_ls(cwd,pathname);
				break;
			case 3: //cd
                my_cd(&cwd,pathname);
				//printf("cd\n");
				break;
			case 4: //pwd
                my_pwd(cwd);
				//printf("pwd\n");
				break;
			case 5: //creat
                my_creat(cwd,pathname);
				//printf("creat\n");
				break;
			case 6: //rm
                my_rm(cwd,pathname);
				//printf("rm\n");
				break;
			case 7: //reload
				my_reload(root, pathname);
				//printf("reload\n");
				break;
			case 8: //save
				my_save(root,pathname,cwd);
				//printf("save\n");
				break;
			case 9: // quit
				quit(root,cwd);
				return 0;
			default: // no right cmd given
				printf("command not found\n");
				break;
		}
		i++;
	}
	return 0;
}
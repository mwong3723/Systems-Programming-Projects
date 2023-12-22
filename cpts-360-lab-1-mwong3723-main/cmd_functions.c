//#include "Header.h"
#include "tree_functions.c"

void my_mkdir(NODE *cwd, char *input){
    if(input != NULL){
        insertNode(input, 'D', cwd);
    }
}

void my_rmdir(char *pathName, NODE *cwd){
    NDIR curState = searchTree(cwd, pathName, 0); NODE* pCur = curState.newDir;

    if(pathName == NULL){ // no input given
        printf("Error: Not enough arguments\n");
    }
    if(curState.status == 1){
        if(pCur->child != NULL){
            printf("Error");
        }
        if(pCur->type == 'F'){
            printf("Error: Not a Directory");
        }
        deleteNode(pCur);
    }
    if(curState.status != 1){
        printf("DIR %s does not exist!\n",pathName);
    }
}

void my_ls(NODE *cwd, char*pathName){
    NODE* pCur; NDIR curState = searchTree(cwd,pathName,0);
    //printf("IN LS FUNC\n");
    //printf("pathname: %s\n",pathName);
    //printf("Status: %d\n", curState.status);
    if(pathName == NULL){ // only ls was given
        pCur = cwd->child;
        //printf("%s\n",pCur->name);
        return;
    }
    if(pathName != NULL){
        if(curState.status != 1){
            pCur = cwd;
            //printf("%c %s\n",pCur->type,pCur->name);
            return;
        }
        if(curState.status == 1){
            pCur = curState.newDir->child;
        }
    }
    while(pCur != NULL){
        printf("%c %s\n",pCur->type,pCur->name);
        pCur = pCur->sibling;
    }
}

void my_cd(NODE **cwd, char *pathName){
    NODE *pCur; NDIR curState = searchTree(*cwd, pathName, 0);
    if(pathName != NULL){
        if(curState.status == 0){
            printf("Error: not a directory\n");
        }
        pCur = curState.newDir;
    }
    else{
        pCur = reset(*cwd);
    }
}

void my_pwd(NODE *cwd){
    char *pathName = malloc(sizeof(char) * 1000); char* tempStr = malloc(sizeof(char) * 1000);
    if(cwd->parent != cwd){
        while(!(cwd->parent == cwd)){
            strcpy(tempStr, "/");
            strcat(tempStr, cwd->name);
            cwd = cwd->parent;
        }
    }
    if(cwd->parent == cwd){
        strcpy(tempStr, "/");
    }
    strcpy(pathName, tempStr);
    printf("%s\n",pathName);
}   

void my_creat(NODE *cwd, char *pathName){
    if(pathName == NULL){
        printf("Error: No path was given\n");
    }
    int success = insertNode(pathName,'F', cwd);
}

void my_rm(NODE *cwd, char *pathName){
    NDIR curState = searchTree(cwd,pathName,0);
    if(pathName == NULL){
        printf("Error: No path was given\n");
    }
    if(curState.newDir->type == 'D'){
        printf("Error: Not a file\n");
    }
    deleteNode(curState.newDir);
}

void my_reload(NODE *root, char *pathName){
    char* tmpStr = malloc(sizeof(char)* 100); char *token = malloc(sizeof(char)*100);
    FILE* infile = fopen(pathName, "r");
    if(pathName == NULL){
        printf("Error: path not given\n");
    }
    while(fgets(tmpStr,sizeof(tmpStr), infile)){
        token = strtok(tmpStr," \n");
        if((strcmp(token, "F") == 0) || (strcmp(token, "D"))){
            printf("Error\n");
        }
        if(strcmp(token, "F") == 0){
            token = strtok(NULL," \n");
            token[strcspn(token, "\n")] = 0;
            my_creat(root,token);
        }
        else if(strcmp(token, "D") == 0){
            token = strtok(NULL," \n");
            token[strcspn(token, "\n")] = 0;
            my_mkdir(root,token);
        }
    }
}

void saveHelper(FILE* outfile, NODE *ptr, NODE* cwd){
    NODE* pCur = ptr->child;
    char *tmpStr = malloc(sizeof(char) * 100);
    char *tmpStr2 = malloc(sizeof(char) * 100);
    //wbs pw
    if(cwd->parent == cwd){
        strcpy(tmpStr2,"/");
    }
    else{
        while(!(cwd->parent == cwd)){
            strcpy(tmpStr2, "/");
            strcat(tmpStr2,cwd->name);
            strcat(tmpStr,tmpStr2);
            strcpy(tmpStr2,tmpStr);
            cwd = cwd->parent;
        }
    }

    fprintf(outfile,"%c %s\n", ptr->type, tmpStr);
    while(pCur != NULL){
        saveHelper(outfile,ptr,cwd);
        pCur = pCur->sibling;

    }
}

void my_save(NODE* root, char*pathname, NODE* cwd){
    if(pathname == NULL){
        printf("Error: pathname not given\n");
    }
    FILE *outfile = fopen(pathname, "w+");
    if(outfile == NULL){
        printf("Could Not open FILE\n");
    }
    saveHelper(outfile,root,cwd);
    fclose(outfile);
}

void quit(NODE* root, NODE* cwd){
    my_save(root,"fssim_wong.txt", cwd);
    exit(0);
}
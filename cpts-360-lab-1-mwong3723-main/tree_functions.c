#include "Header.h"

typedef struct node {
	char  name[64];       // node's name string
	char  type;
	struct node *child, *sibling, *parent;
} NODE;

typedef struct nDir {
    NODE *newDir;
    int status;
    // 0 = !found, 1 = success, 2 = exists
} NDIR;

// gobal variables
NODE *root; 
NODE *cwd;
NODE *pCur;
NODE* pPrev;

int initialize() {
	//dirname = (char*)malloc(sizeof(char) * 100);
    //basename = (char*)malloc(sizeof(char) * 10);
    //token = (char*)malloc(sizeof(char) * 50);

	// printf("Len of dir: %d\n", strlen(dirname));
	// printf("Len of dir: %d\n", strlen(basename));
	// printf("Len of dir: %d\n", strlen(token));


	root = (NODE *)malloc(sizeof(NODE));
	strcpy(root->name, "/");
	root->parent = root;
	root->sibling = 0;
	root->child = 0;
	root->type = 'D';
	cwd = root;
	printf("Filesystem initialized!\n");
	return 1;
}

char* tokenize(char* pathName){
    char* token = strtok(pathName, "/"); char* tmpStr = malloc(sizeof(char)*100);
    while(token != NULL){
        strcpy(tmpStr,token);
        token = strtok(NULL,"/");
    }
}

NODE* reset(NODE* cwd){
    while(cwd->parent != NULL){
        cwd = cwd->parent;
    }
    return cwd;
}

NDIR searchTree(NODE *cwd, char *pathName, int isNode){
    NODE *pCur = cwd;    NODE*curChild; char *tmpPath = malloc(sizeof(char)*1000); int found = 0;
    strcpy(tmpPath, pathName);
    char *token = strtok(tmpPath,"/");
    //printf("Token:%s\n", token);

    if(tmpPath[0] == '/'){ // absolute path
       pCur = reset(cwd);
    }
    while(token != NULL){
        //printf("Inside while\n");
        if(strcmp(token, "..") == 0){ // go back one directory
            pCur = pCur->parent;
        }
        else{
            //printf("Inside else\n");
            curChild = pCur->child;
            //printf("curChild:%s\n", curChild->name);
            for(int i = 0; curChild != NULL; i++){
                if(strcmp(curChild->name, token) == 0){
                    found = 1;
                    pCur = curChild;
                    break;
                }
                else{
                    curChild = curChild->sibling;
                }
            }
            if(found != 1){
                NDIR state;
                if((isNode == 1) && strtok(NULL,"/") == NULL){\
                    state.newDir = pCur; state.status = 1;
                    return state;
                }
                else{
                    state.newDir = NULL; state.status = 0;
                    return state;
                }
            }
        }
         token = strtok(NULL, "/");
    }
    if(isNode == 1){
        NDIR state;
        state.newDir = NULL; state.status = 2;
        printf("Error: File Already Exists\n");
        return state;
    }
    NDIR state;
    state.newDir = pCur; state.status = 1;
    return state;
}

void deleteNode(NODE* ptr){
    if(ptr != ptr->parent->child){
        NODE *pCur = ptr->parent->child;
        while(pCur->sibling != ptr){
            pCur = pCur->sibling;
        }
        pCur->sibling = ptr->sibling;
    }
    if(ptr == ptr->parent->child){
        ptr->parent->child = ptr->sibling;
    }
}

int insertNode(char *input, char fileType, NODE *cwd){
    NODE *pCur; NODE*curPath; char*baseName = malloc(sizeof(char) * 100);
    NDIR curState = searchTree(cwd,input,1);
    if(curState.status != 1){
        if(curState.status == 0){
            printf("Not Found\n");
            return 0;
        }
        if(curState.status == 2){
            printf("Already Exists\n");
            return 0;
        }
    }
    pCur = curState.newDir;
    if(pCur->child == NULL){ // no children
        NODE *pMem = (NODE*)malloc(sizeof(NODE));
        if(pMem != NULL){
            pMem = pCur;
            curPath = pMem;
            curPath->sibling = 0;
            curPath->child = 0;
            curPath->type = fileType;
        }
    }
    else if (pCur->child != NULL){ // ahs children
        pCur = pCur->child;
        while(pCur->sibling != NULL){ // go through all siblings
            pCur = pCur->sibling;
        }
        NODE* pMem = (NODE*)malloc(sizeof(NODE));
        if(pMem != NULL){
            pCur->sibling = pMem;
            pMem = pCur;
            curPath = pMem;
            curPath->sibling = 0;
            curPath->child = 0;
            curPath->type = fileType;
        }
    }
    baseName = tokenize(input);
    if(baseName != NULL){
        //printf("baseName: %s/n",baseName);
        strcpy(curPath->name,baseName);
    }
    return 1;
}

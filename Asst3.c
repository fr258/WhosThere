#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>


#define BACKLOG 5
#define BUFFSIZE 256

typedef struct listNode
{
    char* data;
    struct listNode *next;
	
} Node;

//returns a linked list iterator
//head must not be null
typedef struct
{
	Node* head;
	
} Iterator;

int server(char *port);
int joke(int sfd);
int checkValid(int fd, char* input, int key);
int readIn(int fd, int key);
char* combine(Node* current, int count);
void add(Node* head, char* data);
void deleteList(Node* head);
int hasNext(Iterator* iter);
Node* next(Iterator* iter);


int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage: %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

    (void) server(argv[1]);
    return EXIT_SUCCESS;
}


int server(char *port)
{
    struct addrinfo hint, *address_list, *addr;
    int error, sfd;

    // initialize hints
    memset(&hint, 0, sizeof(struct addrinfo)); 
    hint.ai_family = AF_UNSPEC; //family will be auto selected (?)
    hint.ai_socktype = SOCK_STREAM; //server is accepting input stream
    hint.ai_flags = AI_PASSIVE; //server is listening
	
    error = getaddrinfo(NULL, port, &hint, &address_list);
    if (error != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    // attempt to create socket
    for (addr = address_list; addr != NULL; addr = addr->ai_next) {
        sfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        
        // if we couldn't create the socket, try the next method
        if (sfd == -1) {
            continue;
        }

        // if we were able to create the socket, try to set it up for
        // incoming connections;
        // 
        // note that this requires two steps:
        // - bind associates the socket with the specified port on the local host
        // - listen sets up a queue for incoming connections and allows us to use accept
        if ((bind(sfd, addr->ai_addr, addr->ai_addrlen) == 0) &&
            (listen(sfd, BACKLOG) == 0)) {
            break;
        }

        // unable to set it up, so try the next method
        close(sfd);
    }

    if (addr == NULL) {
        // we reached the end of result without successfuly binding a socket
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo(address_list);

    // at this point sfd is bound and listening
    printf("Waiting for connection\n");
    for (;;) {

        
        // wait for an incoming connection
        accept(sfd, NULL, NULL);

        
        // if we got back -1, it means something went wrong
        if (sfd == -1) {
            perror("accept");
            continue;
        }		
		
		//COMMENCE KNOCK KNOCK
		
		//knock knock
        char* kkj1 = {"Knock, knock."};
        write(sfd, kkj1, sizeof(kkj1)); 
	
		if(readIn(sfd,1)) //who's there?
		{
			close(sfd); //was invalid
			continue;
		}
		
		//setup
        char* kkj2 = {"Incompetent interrupting cow."};
        write(sfd, kkj2, sizeof(kkj2));
		
		if(readIn(sfd,3)) //setup, who?
		{
			close(sfd); //was invalid
			continue;
		}
			
		//punchline
        char* kkj3 = {"...Moo!"};
        write(sfd, kkj3, sizeof(kkj3));
		
		if(readIn(sfd,5)) //surprise/disgust
		{
			close(sfd); //was invalid
			continue;
		}
			
		close(sfd);
    }

    return 0;
}



//returns the next value in a linked list
//iterator passed must not be null
Node* next(Iterator* iter)
{
	if(iter->head != NULL)
	{
		Node* temp = iter->head; //save current head
		iter->head = iter->head->next; //set head in iterator to next node
		return temp; //return current head
	}
	return NULL;
}

//returns if at lesat one more node with data exists in list
//iterator must not be null
int hasNext(Iterator* iter)
{
	return (iter->head != NULL);
}


void deleteList(Node* head)
{
	Iterator iter = {head}, iter1 = {head};
	while(hasNext(&iter))
	{
		free(next(&iter)->data);
	}
	next(&iter1);
	while(hasNext(&iter1)) //free nodes
	{
		free(next(&iter1));
	}
}

void add(Node* head, char* data)
{
	if(head->data != NULL) 
	{
		while(head->next != NULL)
		{
			head = head->next;
		}
		
		head->next = malloc(sizeof(Node));
		head = head->next;
		
	}
	
	head->data = data;
    head->next = NULL;
}

char* substring(char *out, const char *in, int startIndex, int length)
{
    while (length > 0)
    {
        *out = *(in + startIndex);
        out++;
        in++;
        length--;
    }
    *out = '\0';
    return out;
}

int checkFormat(char* input)
{
    char* type = malloc(4);
    if(strlen(input)<3)
        return 1;
    substring(type, input, 0, 3);
    if(strcmp(type, "REG")!=0 && strcmp(type, "ERR")!=0)
    {
        return 1;
    }
    else if(strcmp(type, "REG")==0)
    {
        if(input[3]!='|')
        {
            return 1;
        }
        else 
        {
            int numCount = 0;
            int index = 4;
            while(isdigit(input[index]))
            {
                numCount++;
                index++;
            }
            if(numCount==0)
            {
                return 1;
            }
            else
            {
                if(input[index]!='|')
                {
                    return 1;
                }
                else
                {
                    if(input[strlen(input)-1]!='|')
                    {
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                    
                }
            }
        }
    }
    return -1;
}

int checkContent(char* input, char* correct)
{
    int index = 0;
    while(!isdigit(input[index]))
    {
        index++;
    }
    int count = 0;
    int tempIndex = index;
    while(isdigit(input[index]))
    {
        count++;
        index++;
    }
    char* longth = malloc(100*sizeof(char));
    substring(longth, input, tempIndex, tempIndex+count);
    int length = atoi(longth);
    index++;

    char* content = malloc(1000*sizeof(char));
    substring(content, input, index, length);
    
    return (strcmp(content, correct)==0);

}

int checkLen(char* input)
{
    int index = 0;
    while(!isdigit(input[index]))
    {
        index++;
    }
    int count = 0;
    int tempIndex = index;
    while(isdigit(input[index]))
    {
        count++;
        index++;
    }
    char* longth = malloc(100*sizeof(char));
    substring(longth, input, tempIndex, tempIndex+count);
    int length = atoi(longth);
    index++;
    count = 0;
    while(input[index]!='|')
    {
        count++;
        index++;
    }
    return length==count;
}

int checkValid(int sfd, char* input, int key)
{
    if(key==1)
    {
        if(!checkFormat(input))
        {
            if(checkFormat(input)==-1)
            {
                //error message received, what do
                return -1;
            }
            char* err1 = {"ERR|M1FT|"};
            write(sfd, err1, sizeof(err1));
            return 0;
        }
        if(!checkLen(input))
        {
            char* err1 = {"ERR|M1LN|"};
            write(sfd, err1, sizeof(err1));
            return 0;
        }
        if(!checkContent(input, "Who's there?"))
        {
            char* err1 = {"ERR|M1CT|"};
            write(sfd, err1, sizeof(err1));
            return 0;
        }

    }
    else if(key==3)
    {
        if(!checkFormat(input))
        {
            if(checkFormat(input)==-1)
            {
                //error message received, what do
                return -1;
            }
            char* err3 = {"ERR|M3FT|"};
            write(sfd, err3, sizeof(err3));
            return 0;
        }
        if(!checkLen(input))
        {
            char* err3 = {"ERR|M1LN|"};
            write(sfd, err3, sizeof(err3));
            return 0;
        }
        if(!checkContent(input, "Incompetent interrupting cow, who?"))
        {
            char* err3 = {"ERR|M3CT|"};
            write(sfd, err3, sizeof(err3));
            return 0;
        }
    }
    else if(key==5)
    {
        if(!checkFormat(input))
        {
            if(checkFormat(input)==-1)
            {
                //error message received, what do
                return -1;
            }
            char* err5 = {"ERR|M5FT|"};
            write(sfd, err5, sizeof(err5));
            return 0;
        }
        if(!checkLen(input))
        {
            char* err5 = {"ERR|M1LN|"};
            write(sfd, err5, sizeof(err5));
            return 0;
        }
        if(!ispunct(input[strlen(input)-2]))
        {
            char* err5 = {"ERR|M1CT|"};
            write(sfd, err5, sizeof(err5));
            return 0;
        }
    }
    
	return 1;
}

char* combine(Node* current, int count)
{
	char* total = malloc(BUFFSIZE*count + 2);
	total[0] = '\0';
    while(current!=NULL && current->data != NULL)
    {
		strcat(total, current->data);
        current = current->next;
    }
	return total;
}

//returns the same thing as checkValid
int readIn(int fd, int key)
{	
	int bytes;
	char* buffHead = malloc(BUFFSIZE + 1);
	buffHead[BUFFSIZE] = '\0';

	if(fd<0) //invalid pathName passed
	{
		printf("error\n");
	}
	else
	{	
		bytes =  read(fd, buffHead, BUFFSIZE);

		if(bytes == 0) //nothing read in
		{
			return EXIT_FAILURE;
		}
		
		buffHead[bytes] = '\0';

		if(bytes >= BUFFSIZE)
		{
			int count = 1;
			Node head = {NULL, NULL};
			
			add(&head, buffHead);
			
			char* tempBuff = malloc(BUFFSIZE+1);
			bytes =  read(fd, tempBuff, BUFFSIZE);
			
			tempBuff[bytes] = '\0';
			while(bytes>0)
			{
				add(&head, tempBuff);
				tempBuff = malloc(BUFFSIZE+1);
				bytes =  read(fd, tempBuff, BUFFSIZE);
				tempBuff[bytes] = '\0';
				count++;
			}
			free(tempBuff);				
			buffHead = combine(&head, count);
			deleteList(&head);
			
		}	
	}
	//printf("%s\n", buffHead);
	int exitStatus = checkValid(fd, buffHead, key);
	free(buffHead);
	
	return exitStatus;

}

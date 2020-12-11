#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <pthread.h>

#define BACKLOG 5


void *helper(void* sfd);
int server(char *port);
void joke(int sfd);
int checkValid(int fd, char* input, int key);
int readIn(int fd, int key);

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Error: wrong number of args\n");
		exit(EXIT_FAILURE);
	}
	int port = atoi(argv[1]);
	if(port>5000 && port<65536)
	{
		server(argv[1]);
	}
	else
	{
		printf("Invalid port number.\n");
	}
    return EXIT_SUCCESS;
}


int server(char *port)
{
    struct addrinfo hint, *address_list, *addr;
    int error, sfd, fd;
    pthread_t tid;

    // initialize hints
    memset(&hint, 0, sizeof(struct addrinfo)); 
    hint.ai_family = AF_UNSPEC; //family will be auto selected 
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
        
        // failed to create socket
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
        // reached the end of result without successfuly binding a socket
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo(address_list);

    // at this point sfd is bound and listening
    printf("Waiting for connection\n");
    for (;;) {        
        // wait for an incoming connection
        fd = accept(sfd, NULL, NULL);
        	// accept will block until a remote host tries to connect
        	// it returns a new socket that can be used to communicate with the remote
        	// host, and writes the address of the remote hist into the provided location
        
        //something went wrong
        if (fd == -1) {
            perror("accept");
            continue;
        }

		// spin off a worker thread to handle the remote connection
        error = pthread_create(&tid, NULL, helper, &fd);

		// if couldn't spin off the thread, clean up and wait for another connection
        if (error != 0) {
            fprintf(stderr, "Unable to create thread: %d\n", error);
            close(fd);
            continue;
        }

		// otherwise, detach the thread and wait for the next connection request
        pthread_detach(tid);
    }

    return 0;
}


char* substring(char *out, const char *in, int startIndex, int length)
{
	//printf("entering substring\n");
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

//This method checks the format of the message received. If it starts with ERR, then it will return -1. 
//It returns 1 if improperly formatted and 0 if properly formatted (owing to the author's accidental inversion of C's assignment of 0 to false and 1 to true)
int checkFormat(char* input)
{
    char type[4];
	//printf("input is %s\n", input);
    if(strlen(input)<3)
    {
        //printf("not long enough");
        return 1;
    }
        
    substring(type, input, 0, 3);
	//printf("type is %s\n", type);
    if(strcmp(type, "REG")!=0 && strcmp(type, "ERR")!=0)
    {
        //printf("does not start with reg/err");
        return 1;
    }
    else if(strcmp(type, "REG")==0)
    {
        if(input[3]!='|')
        {
           // printf("no bar after reg");
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
                //printf("no numbers at all.");
                return 1;
            }
            else
            {
                if(input[index]!='|')
                {
                    //printf("missing bar after numbers.");
                    return 1;
                }
                else
                {
                    int last = strlen(input);
                    if(input[last-1]!='|' && input[last-3]!='|')
                    {
                        //printf("missing bar at the end.\n");
                        //printf("The length is %ld and the last character is %c", strlen(input), input[strlen(input)-1]);
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
   // printf("error message input");
    return -1;
}

//This method checks if the content of the message lines up with the expected values: "Who's there?", "<setup>, who?"
int checkContent(char* input, char* correct)
{
	//printf("entering checkContent\n");
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

//This method checks if the length value given in the correctly formatted message matches the actual length of the content sent.
int checkLen(char* input)
{
	//printf("entering checklen\n");
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

//This method checks for errors in received messages. First it checks format by calling checkFormat(); this method will return -1 if the message received was an error message from the client
//If there is a format error, it will write the correct format error message to the socket.
//If the message is correctly formatted, it will then check the length. The call to checkLen() assumes correct format.
//Finally if length is correct we will check if content is correct. 
//Returns 0 for errors in received message, 1 for no errors (valid message) and -1 if an error message was received.
int checkValid(int sfd, char* input, int key)
{
	printf("%s\n", input);
    if(key==1)
    {
        if(checkFormat(input))
        {
            if(checkFormat(input)==-1)
            {
                //error message received, what do
                return -1;
            }
            char* err1 = {"ERR|M1FT|"};
            write(sfd, err1, strlen(err1));
			printf("%s\n", err1);
            return 0;
        }
        if(!checkLen(input))
        {
            char* err1 = {"ERR|M1LN|"};
			printf("%s\n", err1);
            write(sfd, err1, strlen(err1));
            return 0;
        }
        if(!checkContent(input, "Who's there?"))
        {
            char* err1 = {"ERR|M1CT|"};
			printf("%s\n", err1);
            write(sfd, err1, strlen(err1));
            return 0;
        }

    }
    else if(key==3)
    {
        if(checkFormat(input)!=0)
        {
            if(checkFormat(input)==-1)
            {
                //error message received, what do
                return -1;
            }
            char* err3 = {"ERR|M3FT|"};
			printf("%s\n", err3);
            write(sfd, err3, strlen(err3));
            return 0;
        }
        if(!checkLen(input))
        {
            char* err3 = {"ERR|M3LN|"};
			printf("%s\n", err3);
            write(sfd, err3, strlen(err3));
            return 0;
        }
        if(!checkContent(input, "Incompetent interrupting cow, who?"))
        {
            char* err3 = {"ERR|M3CT|"};
			printf("%s\n", err3);
            write(sfd, err3, strlen(err3));
            return 0;
        }
    }
    else if(key==5)
    {
        if(checkFormat(input)!=0)
        {
            if(checkFormat(input)==-1)
            {
                //error message received, what do
                return -1;
            }
            char* err5 = {"ERR|M5FT|"};
			printf("%s\n", err5);
            write(sfd, err5, strlen(err5));
            return 0;
        }
        if(!checkLen(input))
        {
            char* err5 = {"ERR|M5LN|"};
			printf("%s\n", err5);
            write(sfd, err5, strlen(err5));
            return 0;
        }
        if(!ispunct(input[strlen(input)-2]))
        {
            char* err5 = {"ERR|M5CT|"};
			printf("%s\n", err5);
            write(sfd, err5, strlen(err5));
            return 0;
        }
    }
    
	return 1;
}

//attempts to read in the message, and sends it to checkValid for error detection
int readIn(int fd, int key)
{	
	int bytes = 1;
	char buffHead[50] = {0};
	buffHead[1] = '\0';
	int count = 0;
	int exitStatus = 0;
	if(fd<0) //something went wrong
	{
		printf("error\n");
	}
	else
	{	
		bytes = read(fd, buffHead,3); //read in REG or ERR
		char* currPos = buffHead + 3;
		
		if(bytes <= 0)
		{
			printf("client disconnect\n");
			return 0;
		}
		
		if(strcmp(buffHead, "REG") == 0)
		{
			char num[100] = {0}; //will store read-in message length
			int length = 0; //will store read-in message length
			
			//--------------------------------------------------------------------
			//READ IN FIRST BAR
			//--------------------------------------------------------------------
			bytes = read(fd, currPos, 1); //attempt to read in first bar	
			if(bytes <= 0)
			{
				printf("client disconnect\n");
				return 0;
			}
			
			if(*currPos != '|') //message is garbage
			{
				char* temp = buffHead;
				return checkValid(fd, temp, key);
			}
			//--------------------------------------------------------------------
			//READ IN NUMBER
			//--------------------------------------------------------------------
			currPos++; //set current position to after first bar
			for(int i = 0; (bytes = read(fd, currPos, 1)) > 0 && isdigit(*currPos); i++) //read in number
			{
				num[i] = *currPos; //store number
				currPos++; //read into next free byte	
			}
			//--------------------------------------------------------------------
			//READ IN SECOND BAR
			//--------------------------------------------------------------------	
			if(bytes <= 0)
			{
				printf("client disconnect\n");
				return 0;
			}
			else if(*currPos=='|') //number and second bar are as expected
			{
				char* temp = num;
				length = atoi(temp);
				currPos++; //set current position to after second bar
			}
			else
			{
				char* temp = buffHead;
				return checkValid(fd, temp, key);
			}
			//--------------------------------------------------------------------
			//READ IN MESSAGE
			//--------------------------------------------------------------------
			while ((count<length) && ((bytes = read(fd, currPos, 1)) > 0) && (*currPos != '|')) 
			{	
				currPos++;
				count++;
			}
			if(count == length) //read full length in without bar in it
			{
				bytes = read(fd, currPos, 1);
			}
		}
		else if(strcmp(buffHead, "ERR") == 0)
		{
			//--------------------------------------------------------------------
			//READ IN FIRST BAR
			//--------------------------------------------------------------------
			bytes = read(fd, currPos, 1); //attempt to read in first bar	
			if(bytes <= 0)
			{
				printf("client disconnect\n");
				return 0;
			}	
			if(*currPos != '|') //message is garbage
			{
				char temp[] = "ERR|M0FT|";
				temp[5] = '0'+key;
				printf("%s\n", buffHead);
				printf("%s\n", temp);
				write(fd, temp, strlen(temp));
				return 0;
			}
			currPos++;
			//--------------------------------------------------------------------
			//READ IN MESSAGE
			//--------------------------------------------------------------------
			while ((count<4) && ((bytes = read(fd, currPos, 1)) > 0) && (*currPos != '|')) 
			{
				currPos++;
				count++;
			}
			if(count == 4) //read full length in without bar in it
			{
				bytes = read(fd, currPos, 1);
			}
			if(*currPos != '|') //message is garbage
			{
				char temp[] = "ERR|M0FT|";
				temp[5] = '0'+key;
				printf("%s\n", buffHead);
				printf("%s\n", temp);
				write(fd, temp, strlen(temp));
				return 0;
			}
			
		}
		else //garbage
		{
			char* temp = buffHead;
			return checkValid(fd, temp, key);
		}
		
		if(bytes <= 0)
		{
			printf("client disconnect\n");
			return 0;
		}
		
		char* temp = buffHead;
		exitStatus = checkValid(fd, temp, key); //check for errors

	}
	return exitStatus;
}

void* helper(void* input)
{
	int fd = *(int*)input;
	joke(fd);
	close(fd);
	return NULL;
}

void joke(int sfd)
{
	int tempBool = 0;
	char* kkj1 = {"REG|13|Knock, knock.|"};
	write(sfd, kkj1, strlen(kkj1)); 
	printf("%s\n", kkj1);

	tempBool = readIn(sfd,1);
	if(!tempBool || tempBool==-1) //who's there?
	{
		return;
	}
	
	 //setup
	char* kkj2 = {"REG|29|Incompetent interrupting cow.|"};
	write(sfd, kkj2, strlen(kkj2));
	printf("%s\n", kkj2);
	
	tempBool = readIn(sfd,3);
	if(!tempBool || tempBool==-1) //setup, who?
	{
		return;
	}
		
	//punchline
	char* kkj3 = {"REG|7|...Moo!|"};
	write(sfd, kkj3, strlen(kkj3));
	printf("%s\n", kkj3);
	
	tempBool = readIn(sfd,5);
	if(!tempBool || tempBool==-1) //surprise/disgust
	{
		return;
	}		
}
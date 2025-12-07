// 1. the grid 
// 2. cartesian 
// 3. snake
		// how to move
		// how to update
		// directions dx dy
		// length increase
// 4. fruit
		// random spawn - need stdlib.h
		// length of snake ++
// 5. score
		// number of fruits despawned = score
// 6. terminal interface control
		// sleep 
		// continously update the interface
		// turn off keyboard input for eneter
		// continous input

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>//for rand function
#include <sys/select.h>// written in comments
#include <sys/time.h> //timeval built in variable 

#define ROWS 30
#define COLS 71

typedef struct{
	int x,y;
}point;

struct termios old,current;

void new_terminal_settings(){
	tcgetattr(STDIN_FILENO,&old); 									
	/*stdin_fileno is just the file number for keyboard since everyhting in linux is a file Keyboard → OS → [ FD 0 ] → program
		technically input goes to os and then os buffers it to stdin and the stream is named stdin_fileno 0 (for keyboard) and sends to the program
		i can use 0 instead of stdin_fileno but im not so i can increase readablity.
	*/
	current = old;
	current.c_lflag &= ~(ICANON | ECHO);
	// | not bash pipe but bitwise OR combines multiple files &= bitwise and removes specific flags 
	// ICANON the os holds the keyboard inputs in a buffer until enter is pressed
	// and echo is just "a" actually appearing on the terminal when you type it 

	tcsetattr(STDIN_FILENO,TCSANOW,&current); 
	// technically i set the new terminal settings instantly with the flags removed n all
}

void old_terminal_settings(){
	tcsetattr(STDIN_FILENO,TCSANOW,&old);
	printf("\033[?1049l");	// self explainatory block of code sets the terminal back to old settings 
}

int is_key_pressed(){
	struct timeval tv = {0,0}; //built in variable for amount of time before letting select() read the buffer
	// note DO NOT CONFUSE IT WITH SCANF scanf is in canonical mode where terminal buffers input line by line
	// to make select a non blocking function and dont let it wait till timeout to let it read the buffer since the game is continous
	fd_set local_bitset; // created a local bitset 
	
	FD_ZERO(&local_bitset); // erased all of the garbage values present in the virtual bitset(the stream)
	FD_SET(STDIN_FILENO, &local_bitset); // sets the bit corresponding to file descriptor for the keyboard input stream
	return select(STDIN_FILENO + 1, &local_bitset, NULL, NULL, &tv) > 0; // in words select is being told to check the range of bits for fd
	// range of select is always max + 1 hence the above, rest is to manage traffic in the stream of fds
	// after range select checks if readable fd writeable or exceptional in our case we only need to check readability
	// confused with writeable but its just sending data to a file descriptor without blocking & STDIN_FILENO is an input stream, not an output stream.
	//
}

point spawn(point snake[100], int length){
	point fruit; // created this so the fruit doesnt spawn on the snake gang
	while(1){
		int collision = 0;
		fruit.x = rand() % (ROWS - 2) + 1; // +1 means it will start from 1 to rows - 2 cuz wall at 0 and rows - 1
		fruit.y = rand() % (COLS - 2) + 1;
		for(int i = 0; i < length; i++){
			if(snake[i].x == fruit.x && snake[i].y == fruit.y){
				collision = 1;
				break;
			}
		}

		if(collision == 0){
			break;
		}
	}
	return fruit;
}

int main(){
	
	new_terminal_settings();
	printf("\033[?1049h\033[H");

	point snake[100];
	int length = 3;
	point fruit;
	int game_over = 0;

	snake[0].x = 10;
	snake[0].y = 11;

	snake[1].x = 10;
	snake[1].y = 12;

	snake[2].x = 10;
	snake[2].y = 13;

	int dx = 1, dy = 0;
	
	int score = 0;

	fruit = spawn(snake, length);

	while(1){
		printf("\033[H"); // clear the screen ever iteration Moves the cursor to the top-left corner of the terminal
		// \033 can also be written as \x1B (hexadecimal for 27) whihc is the escape sequence in ascii
		// [H = “home”
		// [J = “erase to end of screen”
		for(int row = 0; row < ROWS; row++){
			for(int col = 0; col < COLS; col++){
			
				if(row == 0 || row == ROWS - 1){
					printf("=");
				}
				
				else if(col == 0 || col == COLS - 1){
						printf("|");
				}

				else if(row == fruit.x && col == fruit.y) printf("F");
				else if(row == snake[0].x && col == snake[0].y) printf("@");
				else{										//snake body to be printed in the empty grid
					int printed = 0;
					for(int i = 1; i < length && !printed; i++){
						if(snake[i].x == row && snake[i].y == col){
							printf("O");
							printed = 1;  // got confused here cuz i thought it will only forver print one segment and one head 
							break;		 // but no it wont since ht emain loop row and col is being updated and break is necessary for time efficiency
						}
					}
					if(!printed) printf(" "); // not else if i got confused cuz if will always follwo else if we can write else but will have the remove flag
				}
			}
			printf("\n");
		}
		printf("\n\t\t\t\tScore: %d\n", score);

		fflush(stdout); 
		
		if(is_key_pressed()){
			char c = getchar(); // NOT SCANF ONLY READS INPUTS AND RETURN ASCII CUZ SCANF ASSIGNS VALUES TOO AND ONLY ONE CHARACTER
			
			if(c == 'w'){
				dx = -1;
				dy = 0;
			}
			
			else if(c == 'a'){
				dx = 0;
				dy = -1;
			}
	
			else if(c == 's'){
				dx = 1;
				dy = 0;
			}
	
			else if(c == 'd'){
				dx = 0;
				dy = 1;
			}
		}
	
		for (int i = length - 1; i > 0; i--) {
		    snake[i] = snake[i - 1];  // average array shifitng 
		}
	
		snake[0].x += dx;
		snake[0].y += dy; // body follows the head 

		if(snake[0].x <= 0 || snake[0].x >= ROWS - 1 || snake[0].y <= 0 || snake[0].y >= COLS - 1){
			printf("\033[?1049l");
			printf("\n\n\n"); 
			printf("BOOHOO!! GAME OVER\n");
			printf("your score:%d",score);
			fflush(stdout); 
			usleep(2000000);
			old_terminal_settings();
			return 0;
		} 

		for(int i = 1; i < length; i++){
			if(snake[0].x == snake[i].x && snake[0].y == snake[i].y){
				printf("\033[?1049l");
				printf("\n\n\n"); 
				printf("COLLIDED WITH SELF LOL!!\n");
				printf("your score:%d",score);
				fflush(stdout); 
				usleep(2000000);
				old_terminal_settings();
				return 0;
			}
		}

		if(snake[0].x == fruit.x && snake[0].y == fruit.y){
			length++;
			score++;
			fruit = spawn(snake, length); //generate a fruit position the snake wouldn’t see the fruit in the grid, because the function’s result isn’t stored anywhere
		}
		
		if(game_over){
			old_terminal_settings();
			return 0;
		}

		usleep(100000);  // 100,000 microseconds = 0.1 seconds
	}
}

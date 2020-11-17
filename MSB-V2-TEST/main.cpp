#include "mbed.h"
#include "F429_Mega_Shell_Header.h"
#include "TextLCD/TextLCD.h"

//prototypes

void clearMatrix();
void matrix_scan();
void reloadMatrix();
void updateMatrix();
void setLowBits(char);
void setHighBits(char);

Thread t4;

int letterCount = 5;        // for testing using "HELLO" so 5 letters
int progressCounter = 1;    // which letter is being processed
char message[] = "HELLO";   // the test message
bool spaceFlag = false;     // is it space being processed - for testing there are 2 spaces between letters
int newColumn[8];           // the 8 chars that make up each column - this is the column inserted at the 
                            // far right hand side when the columns are moved one space to the left

// Alphabet array, 26 rows of 16 chars
// Each column of the matrix has 8 LEDs each which are grouped into  two groups of 4 
// Each group of 4 can have the state of each one (OFF or ON) defined by an 4 bit number as folows
// Counting from the bottom to set say just the middle LEDs on set the value to 6
// 0 MSB
// 1
// 1
// 0 LSB
// 
// So the order is Column 1 lower 4, Column 1 upper 4, column 2 lower 4, column 2 upper 4 etc
// only H, E L and O have been defined so far, the row number is calulated by subtracting 65 from char 
// numerical value 

char alphabet[26][16] = {{'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 0 = A ASCII code - 65
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 1 = B ASCII code - 66
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 2 = C ASCII code - 67
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 3 = D ASCII code - 68                        
                        {'0','0','F','F','F','F','B','D','B','D', '3', 'C', '3', 'C', '0', '0'},   // 4 = E ASCII code - 69                        
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 5 = F ASCII code - 70                        
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 6 = G ASCII code - 71
                        {'0','0','F','F','F','F','8','1','8','1', 'F', 'F', 'F', 'F', '0', '0'},   // 7 = H ASCII code - 72
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 8 = I ASCII code - 73                        
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 9 = J ASCII code - 74                        
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 10 = K ASCII code - 75                        
                        {'0','0','F','F','F','F','3','0','3','0', '3', '0', '3', '0', '0', '0'},   // 11 = L ASCII code - 76
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 12 = M ASCII code - 77
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 13 = N ASCII code - 78                        
                        {'0','0','E','7','F','F','3','C','3','C', 'F', 'F', 'E', '7', '0', '0'},   // 14 = O ASCII code - 79                        
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 15 = P ASCII code - 80                        
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 16 = Q ASCII code - 81
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 17 = R ASCII code - 82
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 18 = S ASCII code - 83                        
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 19 = T ASCII code - 84                        
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 20 = U ASCII code - 85                        
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 22 = V ASCII code - 86
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 22 = W ASCII code - 87
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 23 = X ASCII code - 88                        
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'},   // 24 = Y ASCII code - 89                        
                        {'0','0','0','0','0','0','0','0','0','0', '0', '0', '0', '0', '0', '0'}    // 25 = Z ASCII code - 90                        
                        };

int main()
{
  printf("Starting Program..\n");

  t4.start(matrix_scan);
}


// Matrix_scan function sets up the SPI codes for each led in a 3 dimensional array
// it then call the functions:
// reloadMatrix() - moves all columns to the left one place and puts a new column in the far RHS
// updateMatrix() - refreshes the matrix fast enough for peristance of vision
void matrix_scan(void)
{
    int movementCounter = 200;
    int tick;

    int matrix[8][16][3];   // 8 rows, 15 columns three integers for each LED 
    int lhsColVal = 1;      // The column value counter for Left Hand Side
    int rhsColVal = 1;      // the column value counter for Right Hand Side


    // Fill the matrix

    for (int col = 0; col <= 15; col++)
    {
        if (col < 8) // left hand side {0}, {1,2,4,8,16,32,64,128}, {row} - note row counts up from the bottom
        {
            for (int row = 0; row <= 7 ; row++)
            {    
                matrix[row][col][0] = 0;
                matrix[row][col][1] = lhsColVal;
                matrix[row][col][2] = row;
                
            }
        }
        else // right hand side {1, 2, 4, 8, 16, 32, 64, 128}, {row} - note row counts up from the bottom
        {
            for (int row = 0; row <= 7; row++)
            {
                matrix[row][col][0] = rhsColVal;
                matrix[row][col][1] = 0;
                matrix[row][col][2] = row;
                
            }
        }
        if (col < 8) // increment column values in power of 2)
        {
            lhsColVal *= 2;
        }
        else
        {
            rhsColVal *= 2;
        }

    } 
// just for testng limited run 
    while(progressCounter < 5 )
    {
        reloadMatrix(); 
        updateMatrix();      
    }
}

void clearMatrix(void)
{
  cs = 0;          //CLEAR Matrix
  spi.write(0x00); //COL RHS
  spi.write(0x00); //COL LHS
  spi.write(0x00); //ROX RHS
  cs = 1;
}

void reloadMatrix()
{
    static int columnCounter = 0; // which column of the eight in each letter are we processing?
    static int spaceCounter = 0;  // which space (of 2 for testing) are we processing?
    int posn;
    
    if (spaceFlag) // is this a space?
    {
        spaceCounter++;
        printf("Space number %d\n", spaceCounter);
        if (spaceCounter == 2)
        {
            spaceCounter = 0;
            spaceFlag = false;
        }
    } 
    else 
    {
        columnCounter++;
        //printf("ReloadMatrix - columnCounter = %d   progressCounter = %d letterCount = %d\n", columnCounter,progressCounter ,letterCount);
        posn = message[progressCounter - 1]-65;
        if (columnCounter == 1)
        {   
            printf("New letter it is %c\n", message[progressCounter - 1]);
            printf("Code is %d position %d\n", message[progressCounter - 1], posn);
            printf("%c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c\n", alphabet[posn][0],
                                                                         alphabet[posn][1],
                                                                         alphabet[posn][2],
                                                                         alphabet[posn][3],
                                                                         alphabet[posn][4],
                                                                         alphabet[posn][5],
                                                                         alphabet[posn][6],
                                                                         alphabet[posn][7], 
                                                                         alphabet[posn][8],
                                                                         alphabet[posn][9],
                                                                         alphabet[posn][10],
                                                                         alphabet[posn][11],
                                                                         alphabet[posn][12],
                                                                         alphabet[posn][13],
                                                                         alphabet[posn][14],
                                                                         alphabet[posn][15] );
        } 

        //printf("Status %c %c\n", alphabet[posn][0], alphabet[posn][1]);
        switch (columnCounter)
        {
            case 1:     // column one of 8 so use alphabet[posn][0 and 1]
                //printf("case 1\n");
                //printf("Sending %c %c\n", alphabet[posn][0], alphabet[posn][1]);
                setLowBits(alphabet[posn][0]);
                setHighBits(alphabet[posn][1]);
                printf("New column is %d%d%d%d%d%d%d%d\n", newColumn[0],newColumn[1],newColumn[2],newColumn[3],newColumn[4],newColumn[5],newColumn[6],newColumn[7]);
                break;
            case 2:     // column one of 8 so use alphabet[posn][0 and 1]
                //printf("case 2\n");
                //printf("Sending %c %c\n", alphabet[posn][2], alphabet[posn][3]);
                setLowBits(alphabet[posn][2]);
                setHighBits(alphabet[posn][3]);
                printf("New column is %d%d%d%d%d%d%d%d\n", newColumn[0],newColumn[1],newColumn[2],newColumn[3],newColumn[4],newColumn[5],newColumn[6],newColumn[7]);
                break;
            case 3:     // column one of 8 so use alphabet[posn][0 and 1]
                //printf("case 3\n");
                //printf("Sending %c %c\n", alphabet[posn][4], alphabet[posn][5]);
                setLowBits(alphabet[posn][4]);
                setHighBits(alphabet[posn][5]);
                printf("New column is %d%d%d%d%d%d%d%d\n", newColumn[0],newColumn[1],newColumn[2],newColumn[3],newColumn[4],newColumn[5],newColumn[6],newColumn[7]);
                break;
            case 4:     // column one of 8 so use alphabet[posn][0 and 1]
                //printf("case 4\n");
                //printf("Sending %c %c\n", alphabet[posn][6], alphabet[posn][7]);
                setLowBits(alphabet[posn][6]);
                setHighBits(alphabet[posn][7]);
                printf("New column is %d%d%d%d%d%d%d%d\n", newColumn[0],newColumn[1],newColumn[2],newColumn[3],newColumn[4],newColumn[5],newColumn[6],newColumn[7]);
                break;
            case 5:     // column one of 8 so use alphabet[posn][0 and 1]
                //printf("case 5\n");
                //printf("Sending %c %c\n", alphabet[posn][8], alphabet[posn][9]);
                setLowBits(alphabet[posn][8]);
                setHighBits(alphabet[posn][9]);
                printf("New column is %d%d%d%d%d%d%d%d\n", newColumn[0],newColumn[1],newColumn[2],newColumn[3],newColumn[4],newColumn[5],newColumn[6],newColumn[7]);
                break;
            case 6:     // column one of 8 so use alphabet[posn][0 and 1]
                //printf("case 6\n");
                //printf("Sending %c %c\n", alphabet[posn][10], alphabet[posn][11]);
                setLowBits(alphabet[posn][10]);
                setHighBits(alphabet[posn][11]);
                printf("New column is %d%d%d%d%d%d%d%d\n", newColumn[0],newColumn[1],newColumn[2],newColumn[3],newColumn[4],newColumn[5],newColumn[6],newColumn[7]);
                break;
            case 7:     // column one of 8 so use alphabet[posn][0 and 1]
                //printf("case 7\n");
                //printf("Sending %c %c\n", alphabet[posn][12], alphabet[posn][13]);
                setLowBits(alphabet[posn][12]);
                setHighBits(alphabet[posn][13]);
                printf("New column is %d%d%d%d%d%d%d%d\n", newColumn[0],newColumn[1],newColumn[2],newColumn[3],newColumn[4],newColumn[5],newColumn[6],newColumn[7]);
                break;
            case 8:     // column one of 8 so use alphabet[posn][0 and 1]
                //printf("case 8\n");
                //printf("Sending %c %c\n", alphabet[posn][14], alphabet[posn][15]);
                setLowBits(alphabet[posn][14]);
                setHighBits(alphabet[posn][15]);
                printf("New column is %d%d%d%d%d%d%d%d\n", newColumn[0],newColumn[1],newColumn[2],newColumn[3],newColumn[4],newColumn[5],newColumn[6],newColumn[7]);
                break;
            default:
                printf("Error switch columnCounter\n");
                break;
        }
    }
    if (columnCounter == 8)
    {
        columnCounter = 0;
        progressCounter++;
        spaceFlag = true;
    }
    if(progressCounter > letterCount)
    {   
        printf("Loop around to start of text\n");
        progressCounter = 1;
    }
    thread_sleep_for(200);
}

void updateMatrix()
{
    //printf("updateMatrix\n");
    thread_sleep_for(500);

}

void setLowBits(char ch)
{
    switch(ch)
        {
        case '0':
            //printf("Found a zero\n");
            newColumn[0] = 0; newColumn[1] = 0; newColumn[2] = 0; newColumn[3] = 0;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case '1':
            //printf("Found a one\n");
            newColumn[0] = 1; newColumn[1] = 0; newColumn[2] = 0; newColumn[3] = 0;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case '2':
            //printf("Found a two\n");
            newColumn[0] = 0; newColumn[1] = 1; newColumn[2] = 0; newColumn[3] = 0;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case '3':
            //printf("Found a 3\n");
            newColumn[0] = 1; newColumn[1] = 1; newColumn[2] = 0; newColumn[3] = 0;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case '4':
            //printf("Found a 4\n");
            newColumn[0] = 0; newColumn[1] = 0; newColumn[2] = 1; newColumn[3] = 0;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case '5':
            //printf("Found a 5\n");
            newColumn[0] = 1; newColumn[1] = 0; newColumn[2] = 1; newColumn[3] = 0;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case '6':
            //printf("Found a 6\n");
            newColumn[0] = 0; newColumn[1] = 1; newColumn[2] = 1; newColumn[3] = 0;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case '7':
            //printf("Found a 7\n");
            newColumn[0] = 1; newColumn[1] = 1; newColumn[2] = 1; newColumn[3] = 0;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case '8':
            //printf("Found a 8\n");
            newColumn[0] = 0; newColumn[1] = 0; newColumn[2] = 0; newColumn[3] = 1;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case '9':
            //printf("Found a 9\n");
            newColumn[0] = 1; newColumn[1] = 0; newColumn[2] = 0; newColumn[3] = 1;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case 'A':
            //printf("Found a 10\n");
            newColumn[0] = 0; newColumn[1] = 1; newColumn[2] = 0; newColumn[3] = 1;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case 'B':
            //printf("Found a 11\n");
            newColumn[0] = 1; newColumn[1] = 1; newColumn[2] = 0; newColumn[3] = 1;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case 'C':
            //printf("Found a 12\n");
            newColumn[0] = 0; newColumn[1] = 0; newColumn[2] = 1; newColumn[3] = 1;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case 'D':
            //printf("Found a 13\n");
            newColumn[0] = 1; newColumn[1] = 0; newColumn[2] = 1; newColumn[3] = 1;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case 'E':
            //printf("Found a 14\n");
            newColumn[0] = 0; newColumn[1] = 1; newColumn[2] = 1; newColumn[3] = 1;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        case 'F':
            //printf("Found a 15\n");
            newColumn[0] = 1; newColumn[1] = 1; newColumn[2] = 1; newColumn[3] = 1;
            //printf("newColumn[0] is now %d\n", newColumn[0]);
            break;
        default:
            printf("Error\n");
            break;
        }
}

void setHighBits(char ch)
{
    switch(ch)
        {
        case '0':
            //printf("Found a 0\n");
            newColumn[4] = 0; newColumn[5] = 0; newColumn[6] = 0; newColumn[7] = 0;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case '1':
            //printf("Found a one\n");
            newColumn[4] = 1; newColumn[5] = 0; newColumn[6] = 0; newColumn[7] = 0;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case '2':
            //printf("Found a two\n");
            newColumn[4] = 0; newColumn[5] = 1; newColumn[6] = 0; newColumn[7] = 0;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case '3':
            //printf("Found a three\n");
            newColumn[4] = 1; newColumn[5] = 1; newColumn[6] = 0; newColumn[7] = 0;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case '4':
            //printf("Found a four\n");
            newColumn[4] = 0; newColumn[5] = 0; newColumn[6] = 1; newColumn[7] = 0;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case '5':
            //printf("Found a five\n");
            newColumn[4] = 1; newColumn[5] = 0; newColumn[6] = 1; newColumn[7] = 0;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case '6':
            //printf("Found a six\n");
            newColumn[4] = 0; newColumn[5] = 1; newColumn[6] = 1; newColumn[7] = 0;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case '7':
            //printf("Found a seven\n");
            newColumn[4] = 1; newColumn[5] = 1; newColumn[6] = 1; newColumn[7] = 0;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case '8':
            //printf("Found a eight\n");
            newColumn[4] = 0; newColumn[5] = 0; newColumn[6] = 0; newColumn[7] = 1;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case '9':
            //printf("Found a nine\n");
            newColumn[4] = 1; newColumn[5] = 0; newColumn[6] = 0; newColumn[7] = 1;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case 'A':
            //printf("Found an A\n");
            newColumn[4] = 0; newColumn[5] = 1; newColumn[6] = 0; newColumn[7] = 1;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case 'B':
            //printf("Found a B\n");
            newColumn[4] = 1; newColumn[5] = 1; newColumn[6] = 0; newColumn[7] = 1;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case 'C':
            //printf("Found a C\n");
            newColumn[4] = 0; newColumn[5] = 0; newColumn[6] = 1; newColumn[7] = 1;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case 'D':
            //printf("Found a D\n");
            newColumn[4] = 1; newColumn[5] = 0; newColumn[6] = 1; newColumn[7] = 1;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case 'E':
            //printf("Found an E\n");
            newColumn[4] = 0; newColumn[5] = 1; newColumn[6] = 1; newColumn[7] = 1;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        case 'F':
            //printf("Found an F\n");
            newColumn[4] = 1; newColumn[5] = 1; newColumn[6] = 1; newColumn[7] = 1;
            //printf("newColumn[4] is now %d\n", newColumn[4]);
            break;
        default:
            printf("Error\n");
            break;
            
        }

}
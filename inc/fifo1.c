// FIFO1.c
// Runs on any microcontroller
// Provide functions that implement the Software FiFo Buffer
// Last Modified: 4/20/2024
// Student names: Toby Nguyen

 #include <stdint.h>


// // Declare state variables for FiFo
// //        size, buffer, put and get indexes

     uint8_t size = 19;
     int32_t FIFO1[19];
     int32_t head;
     int32_t tail;
     int32_t counter;

// // *********** Fifo1_Init**********
// // Initializes a software FIFO1 of a
// // fixed size and sets up indexes for
// // put and get operations
void Fifo1_Init() {
  head = 0;
  tail = 0;
  counter = 0;
}

// // *********** Fifo1_Put**********
// // Adds an element to the FIFO1
// // Input: data is character to be inserted
// // Output: 1 for success, data properly saved
// //         0 for failure, FIFO1 is full
uint32_t Fifo1_Put(char data){
    if(counter >= size) { // full
        return 0;
    }
    else {
        FIFO1[head] = data;
        head++;
        counter++;
        if(head >= size) { // wrap around
            head = 0;
        }
        return 1;
    }
}

// // *********** Fifo1_Get**********
// // Gets an element from the FIFO1
// // Input: none
// // Output: If the FIFO1 is empty return 0
// //         If the FIFO1 has data, remove it, and return it
uint32_t Fifo1_Get(void){
    if(counter < 1) { // empty
        return 0;
    }
    else {
        uint32_t data = FIFO1[tail];
        tail++;
        counter--;
        if(tail >= size) { // wrap around
            tail = 0;
        }
        return data;
    }
}






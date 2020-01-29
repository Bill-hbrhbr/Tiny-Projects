#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define KEY_SIZE 10
#define VAL_SIZE 8
typedef enum ERRORCODE {
    OK,
    SEGMENTAION_FAULT,
    LIST_DOES_NOT_EXIST,
    INVALID_INDEX,
    MEMORY_FULL
} ERRORCODE;

ERRORCODE updatePos(unsigned int *bytePos, unsigned int *bitShift, unsigned int diff, unsigned int listNumBytes) {
    // Update the position
    *bitShift += diff;
    *bytePos += *bitShift / 8;
    *bitShift %= 8;
    
    // Check seg fault
    if (*bytePos >= listNumBytes) {
        return SEGMENTAION_FAULT;
    }
    
    // Successful
    return OK;
}

// Stores the given number into the linkedListImage bit array
ERRORCODE storeNumber(unsigned int *bytePos, unsigned int *bitShift, unsigned long num, unsigned int len,
    unsigned char *linkedListImage, unsigned int listNumBytes) {
    
    // Check seg fault
    ERRORCODE code;
    if ((code = updatePos(bytePos, bitShift, 0, listNumBytes)) != OK) { return code; }
    
    // Start storing
    unsigned int bitVal;
    for (unsigned int i = 0; i < len; ++i) {
        // Fetch the bit value from the number
        bitVal = (num >> (len - 1 - i) & 1);
        // Store the value into the array
        linkedListImage[*bytePos] &= ~(1 << (7 - *bitShift));
        linkedListImage[*bytePos] |= (bitVal << (7 - *bitShift));
        // Advance pos and check seg fault
        if ((code = updatePos(bytePos, bitShift, 1, listNumBytes)) != OK) { return code; }
    }
    
    // Successful
    return OK;
}

// Load number from the linkedListImage bit array and store int somewhere else
ERRORCODE loadNumber(unsigned int *bytePos, unsigned int *bitShift, unsigned long *numPtr, unsigned int len,
    unsigned char *linkedListImage, unsigned int listNumBytes) {
    
    // Check seg fault
    ERRORCODE code;
    if ((code = updatePos(bytePos, bitShift, 0, listNumBytes)) != OK) { return code; }
    
    // Start loading
    *numPtr = 0; // Clear the value
    unsigned int bitVal;
    for (unsigned int i = 0; i < len; ++i) {
        // Fetch the bit value from the linked list
        bitVal = (linkedListImage[*bytePos] >> (7 - *bitShift)) & 1;
        // Store it into the number
        *numPtr |= bitVal << (len - 1 - i);
        // Advance pos and check seg fault
        if ((code = updatePos(bytePos, bitShift, 1, listNumBytes)) != OK) { return code; }
    }
    
    // Successful
    return OK;
}

ERRORCODE insert(unsigned long *headBitPos, unsigned int key, unsigned int val,
    unsigned int id, unsigned char *linkedListImage, unsigned int listNumBytes) {
    
    ERRORCODE code;
    if (!headBitPos) {
        return LIST_DOES_NOT_EXIST;
    }
    
    unsigned int totalBits = listNumBytes * 8;
    unsigned int nextPtrSize = log2(totalBits);
    
    //                        Allocated + KEY_SIZE + VAL_SIZE + NEXT_PTR_SIZE
    unsigned int structSize = 1         + KEY_SIZE + VAL_SIZE + nextPtrSize;
    unsigned int skipToNextPtr = structSize - nextPtrSize;
    
    // Find a place to insert the new structure
    unsigned long alloc_bit;
    unsigned int bytePos = 0, bitShift = 0;
    unsigned int insertBytePos, insertBitShift;
    while (1) {
        // Update insertion point
        insertBytePos = bytePos;
        insertBitShift = bitShift;
        
        // Check if the space is already allocated
        code = loadNumber(&bytePos, &bitShift, &alloc_bit, 1, linkedListImage, listNumBytes);
        if (code != OK) { return code; }
        
        // Check if the block is not enough and update the pos along the way (happens at the end of the list)
        if ((code = updatePos(&bytePos, &bitShift, structSize - 1, listNumBytes)) != OK) {
            return MEMORY_FULL;  // Should not return segmentation fault
        }
        
        // Check if the insertion point is found
        if (!alloc_bit) { break; }
    }
    
    // Find the last node in the original list
    bytePos = *headBitPos / 8, bitShift = *headBitPos % 8;
    unsigned int oldNextPtrBytePos = bytePos, oldNextPtrBitShift = bitShift;
    unsigned long nextPtrVal;

    for (unsigned int i = 0; i < id; ++i) {
        // Check if the index position is valid (if the list has ended prematurely)
        code = loadNumber(&bytePos, &bitShift, &alloc_bit, 1, linkedListImage, listNumBytes);
        if (code != OK) { return code; }
        if (!alloc_bit) { return INVALID_INDEX; }
        
        // Shift to the "next" pointer
        if ((code = updatePos(&bytePos, &bitShift, skipToNextPtr - 1, listNumBytes)) != OK) { return code; }
        
        // Record the position of the "next" pointer
        oldNextPtrBytePos = bytePos;
        oldNextPtrBitShift = bitShift;
        
        // Update the "next" pointer value
        nextPtrVal = 0;
        code = loadNumber(&bytePos, &bitShift, &nextPtrVal, nextPtrSize, linkedListImage, listNumBytes);
        if (code != OK) { return code; }
        
        // Update the bytePos and bitShift
        bytePos = bitShift = 0;
        if ((code = updatePos(&bytePos, &bitShift, nextPtrVal, listNumBytes)) != OK) { return code; }
    }
    
    // Insert into head
    if (id == 0) {
        // Record the insertion point
        bytePos = insertBytePos;
        bitShift = insertBitShift;
        storeNumber(&bytePos, &bitShift, 1, 1, linkedListImage, listNumBytes); // Allocated
        storeNumber(&bytePos, &bitShift, key, KEY_SIZE, linkedListImage, listNumBytes); // Store Key
        storeNumber(&bytePos, &bitShift, val, VAL_SIZE, linkedListImage, listNumBytes); // Store Value
        storeNumber(&bytePos, &bitShift, *headBitPos, nextPtrSize, linkedListImage, listNumBytes); // Points to original head
        
        // Update the head pointer
        *headBitPos = 8 * insertBytePos + insertBitShift;
    } else {
        bytePos = oldNextPtrBytePos;
        bitShift = oldNextPtrBitShift;
        unsigned long originalNextPtrVal = 0;
        // Fetch the original next pointer
        loadNumber(
            &oldNextPtrBytePos, 
            &oldNextPtrBitShift, 
            &originalNextPtrVal,
            nextPtrSize, 
            linkedListImage, 
            listNumBytes
        ); 
        oldNextPtrBytePos = bytePos;
        oldNextPtrBitShift = bitShift;
        
        // Update the original next pointer
        unsigned long newNextPtrVal = insertBytePos * 8 + insertBitShift;
        storeNumber(
            &oldNextPtrBytePos, 
            &oldNextPtrBitShift, 
            newNextPtrVal,
            nextPtrSize, 
            linkedListImage, 
            listNumBytes
        );
        // Create a new node
        storeNumber(&insertBytePos, &insertBitShift, 1, 1, linkedListImage, listNumBytes); // Allocated
        storeNumber(&insertBytePos, &insertBitShift, key, KEY_SIZE, linkedListImage, listNumBytes); // Store Key
        storeNumber(&insertBytePos, &insertBitShift, val, VAL_SIZE, linkedListImage, listNumBytes); // Store Value
        storeNumber(
            &insertBytePos, 
            &insertBitShift, 
            originalNextPtrVal, 
            nextPtrSize, 
            linkedListImage, 
            listNumBytes
        ); // Store the original next pointer
        
    }
    
    // Successful
    return OK;
}

// Printing functions
void printBit(unsigned char number, unsigned int pos) {
    printf("%u", ((number >> (7 - pos)) & 1));
}

void printBits(unsigned char number, unsigned int start, unsigned int end) {
    for (int i = start; i < end; ++i) {
        printBit(number, i);
    } 
}

int main(void)
{
    // 16 bytes = 128 bits = 7 bit per "next" pointer
    // struct size = 1 + 10 (KEY_SIZE) + 8 (VAL_SIZE) + 7 = 26
    // The possible start positions for structures:
    // 0 = 7'b0000000
    // 26 = 16 + 8 + 2 = 7'b0011010
    // 52 = 32 + 16 + 4 = 7'b0110100
    // 78 = 64 + 8 + 4 + 2 = 7'b1001110
    // 104 + 26 = 130 >= 128 (Memory FULL)
    // Assume that their is already one node in the list, at position 26
    // Keys and values are irrelavant. The list should be like
    // 8'd0, 8'd0, 8'd0, 8'b00|1|00000, 8'b00000|000, 8'b00000 | 111, 8'b1110|0|000 ...
    // ^                       ^   [key = 0]       [value = 0]   [next = 126] ^  [irrelevant]         
    // not allocated        allocated                                    not allocated
    // pos 126 will never be used so it functions like a NULL pointer
    unsigned char list[16] = {0x00, 0x00, 0x00, 0x20, 0x00, 0x07, 0xE0};
    
    unsigned int headBitPos = 26;
    /** Error codes
     * OK = 0
     * SEGMENTAION_FAULT = 1
     * LIST_DOES_NOT_EXIST = 2
     * INVALID_INDEX = 3
     * MEMORY_FULL = 4
     */
    // Now start inserting nodes
    // ERRORCODE insert(unsigned long *headBitPos, unsigned int key, unsigned int val,
    // unsigned int id, unsigned char *linkedListImage, unsigned int listNumBytes);
    ERRORCODE code;
    
    /***************************************/
    // Test 0: Insert a new node at index 2
    // Index = 2 is invalid since there is only 1 object in the list, so code = 3 (INVALID_INDEX)
    code = insert(&headBitPos, 10, 8, 2, list, sizeof(list));
    printf("----Test 0----\n");
    printf("headBitPos = %d\n", headBitPos);
    printf("code = %d\n", code);
    
    /***************************************/
    // Test 1: Insert a new node at index 1. Position 0 is empty so it should be used
    // Result 26->0->null
    printf("----Test 1----\n");
    code = insert(&headBitPos, 10, 8, 1, list, sizeof(list));
    printf("headBitPos = %d\n", headBitPos);
    printf("code = %d\n", code);
    // Now the list should be like
    // 8'b|1|00000001, 8'b010|00001, 8'b000|11111, 2'b10
    //   Alloc   [key = 10]      [val = 8]  [next = 126]     
    
    // 6'b|1|00000, 8'b00000|000, 8'b00000|000, 4'b0000   ...Followed by all 0s (nothing is allocated)
    //   Alloc   [key = 0]      [val = 0]   [next = 0]
    
    // Printing
    for (int i = 0; i < sizeof(list); ++i) {
        printBits(list[i], 0, 8);
        printf(" ");
    }
    puts("");
    puts("-------------------------");
    
    /***************************************/
    // Test 2: Insert a new node at index 0. Position 52 is empty so it should be used
    // Result 52->26->0->null, headBitPos should be changed to 52.
    printf("----Test 2----\n");
    code = insert(&headBitPos, 30, 9, 0, list, sizeof(list));
    printf("headBitPos = %d\n", headBitPos);
    printf("code = %d\n", code);
    // Now the list should be like
    // 8'b|1|00000001, 8'b010|00001, 8'b000|11111, 2'b10
    //   Alloc   [key = 10]      [val = 8]  [next = 126]     
    
    // 6'b|1|00000, 8'b00000|000, 8'b00000|000, 4'b0000
    //   Alloc   [key = 0]      [val = 0]   [next = 0]
    
    // 4'b|1|000, 8'b0011110|0, 8'b0001001|0, 6'b011010   ...Followed by all 0s (nothing is allocated)
    //   Alloc  [key = 30]     [val = 9]    [next = 26]
    
    // Printing
    for (int i = 0; i < sizeof(list); ++i) {
        printBits(list[i], 0, 8);
        printf(" ");
    }
    puts("");
    puts("-------------------------");
    
    /***************************************/
    // Test 3: Insert a new node at index 2. Position 78 is empty so it should be used
    // Result 52->26->78->0->null. The "next" node of 26 should be changed to 78
    printf("----Test 3----\n");
    code = insert(&headBitPos, 48, 254, 2, list, sizeof(list));
    printf("headBitPos = %d\n", headBitPos);
    printf("code = %d\n", code);
    // Now the list should be like
    // 8'b|1|00000001, 8'b010|00001, 8'b000|11111, 2'b10
    //   Alloc   [key = 10]      [val = 8]  [next = 126]     
    
    // 6'b|1|00000, 8'b00000|000, 8'b00000|100, 4'b1110
    //   Alloc   [key = 0]      [val = 0]   [next = 78]
    
    // 4'b|1|000, 8'b0011110|0, 8'b0001001|0, 6'b011010
    //   Alloc  [key = 30]     [val = 9]    [next = 26]
    
    // 2'b|1|0, 8'b00011000, 8'b0|1111111, 8'b0|0000000
    //   Alloc    [key = 48]       [val = 254]  [next = 0]
    
    // Printing
    for (int i = 0; i < sizeof(list); ++i) {
        printBits(list[i], 0, 8);
        printf(" ");
    }
    puts("");
    puts("-------------------------");
    
    /***************************************/
    // Test 4: Insert a new node at index 1. No more memory available
    // Result 52->26->78->0->null. Error code should be 4 (Memory Full)
    // List is unchanged.
    printf("----Test 4----\n");
    code = insert(&headBitPos, 48, 254, 1, list, sizeof(list));
    printf("headBitPos = %d\n", headBitPos);
    printf("code = %d\n", code);
    // Printing
    for (int i = 0; i < sizeof(list); ++i) {
        printBits(list[i], 0, 8);
        printf(" ");
    }
    puts("");
    puts("-------------------------");
    
    // Thank you
    return 0;
}
/* You are not allowed to use <stdio.h> */
#include <stdlib.h>  // For malloc, free, realloc
#include "io.h"      // For read_char, write_char, write_string, write_int

typedef struct {
    int *data;     
    int size;      
    int capacity;  // Current capacity of the array
} Collection;

// Initialize the collection
void init_collection(Collection *collection) {
    collection->size = 0;
    collection->capacity = 10;  // Starting with a capacity of 10
    collection->data = (int*)malloc(collection->capacity * sizeof(int));
}

// Add an element to the collection
void add_to_collection(Collection *collection, int value) {
    if (collection->size >= collection->capacity) {
        collection->capacity *= 2;  // Double the capacity when the array is full
        collection->data = (int*)realloc(collection->data, collection->capacity * sizeof(int));
    }
    collection->data[collection->size++] = value;  // Add the value and increment size
}

// Remove the last element from the collection
void remove_from_collection(Collection *collection) {
    if (collection->size > 0) {
        collection->size--;  // Decrease the size
    }
}

// Print the collection as a comma delimited list ending with a semicolon
void print_collection(Collection *collection) {
    for (int i = 0; i < collection->size; i++) {
        write_int(collection->data[i]);
        if (i < collection->size - 1) {
            write_char(','); 
        }
    }
    write_char(';'); 
    write_char('\n');
}

// Free the dynamically allocated memory for the collection
void free_collection(Collection *collection) {
    free(collection->data);  // Free the dynamically allocated memory
}

/**
 * @name  main
 * @brief This function is the entry point to your program
 * @return 0 for success, anything else for failure
 */
int main() {
    int counter = 0;             // setting the counter to 0
    Collection collection;       
    init_collection(&collection); 

    while (1) {
        char command = read_char();  // Read a command from standard input

        // commands processed per assigment
        if (command == 'a') {
            add_to_collection(&collection, counter);  // Add counter to the collection
            counter++;  
        } else if (command == 'b') {
            counter++;  
        } else if (command == 'c') {
            remove_from_collection(&collection);  // Remove the most recent element
            counter++;  
        } else {
            // If there is any other character then stop processing commands
            break;
        }
    }

    // print the collection
    print_collection(&collection);

    // Free the memory allocated for the collection
    free_collection(&collection);

    return 0;
}

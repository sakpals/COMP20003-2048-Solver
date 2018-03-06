#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include "node.h"
#include "priority_queue.h"

#define NUM_MOVES 4

/* initialises the heap */
void initialize_ai();

/**
 * Find best action by building all possible paths up to depth max_depth
 * and back propagate using either max or avg
 */
move_t get_next_move( uint8_t board[SIZE][SIZE], int max_depth, 
	propagation_t propagation, info_t *summary);

/* Propagates a nodes score to the first action score depending on the type of 
propagation. 
-If it is average, then the first action node reflects the average
score that can created from any of its paths. 
-If maximum, then the first action node is only updated if the node's priority
is larger than that of the first action node
 */
void propagate_back(propagation_t propagation, node_t *node, uint32_t first_action_scores[]);



/* Returns the node that is created when particular action is applied to the node
supplied as a parameter 
*/
node_t *apply_action(node_t *PoppedNode, move_t tryMove);


/* Returns the index value for first_action_scores[] depending on the type of move
specified in the input 
*/
int set_move_index(move_t move);


/* Copies the contents of one board from a node to another given the boards as
inputs 
 */
void copy_board(uint8_t OldBoard[SIZE][SIZE], uint8_t NewBoard[SIZE][SIZE]);


/* Initialises the explored array so that each cell in the array is a null pointer 
*/
void initialise_explored(node_t **explored, int start, int end);
#endif

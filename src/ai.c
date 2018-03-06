#include <time.h>
#include <stdlib.h>
#include "ai.h"
#include "utils.h"
#include "priority_queue.h"


struct heap h;

/* initialises the heap */
void initialize_ai() {
	heap_init(&h);
}


/**
 * Find best action by building all possible paths up to depth max_depth
 * and back propagate using either max or avg
 */
move_t get_next_move( uint8_t board[SIZE][SIZE], int max_depth, 
	propagation_t propagation, info_t *summary) {

	int i;
	move_t best_action;
	move_t possible_actions[] = {left, right, up, down};
	int round;

	/* number of nodes explored */
	int explored_count = 0;

	/* array of total scores that can be generated for each move */
	uint32_t first_action_scores[] = {0,0,0,0};
	uint32_t max_score = 0;
	int num_max_scores = 0;
	int index_max_score = 0;

	/* array of moves that may or may not generate max score */
	bool is_max_score[] = {false, false, false, false};

	/* number of possible children + start node, 
	the minimum number of nodes in explored [] */
    int num = 5; 

	/* initialises the starting node based on input board */
	node_t *start = (node_t *)malloc(sizeof(struct node_s));
	if(start == NULL) {
		printf("Error allocating memory for starting node\n");
		exit(EXIT_FAILURE);
	}
	start->parent = NULL;
	start->priority = 0; 
	start->num_childs  = 0;
	start->depth = 0;
	copy_board(board, start->board);


	/* initialises the explored array for storing nodes after they're popped from
	the priority queue */
	node_t **explored = (node_t **)malloc(sizeof(node_t*)*num);
	initialise_explored(explored, 0, num);

	/* initialise heap/PQ and pushes the starting node onto the heap */
	initialize_ai();
	heap_push(&h, start);
	


	/* continue applyin the following actions until no nodes remain in the heap */
	while(h.count != 0) {

		node_t* popped_node; 
		/* popped node from the priority queue that is used to generate child nodes */
		popped_node = heap_delete(&h);
		summary->num_expanded ++;


		/* in the case that the explored array is full, realloc memory to add future
		nodes, otherwise add popped node to the explored array */
		if(explored_count < num) {
			explored[explored_count] = popped_node;
			explored_count ++;
		}
		else {
			num *= 2;
			node_t **tmp_explored = (node_t **)realloc(explored, num*sizeof(node_t*));
			if(tmp_explored == NULL) {
				printf("Error reallocating memory for explored array\n");
				exit(EXIT_FAILURE);
			}
			explored = tmp_explored;
			initialise_explored(explored, num/2, num);
			explored[explored_count] = popped_node;
			explored_count ++;
		}
		
		/* generate children nodes for the popped node if it hasn't reached max_depth*/
		if(popped_node->depth < max_depth) {

				
			/* generate new child node for each action (left, right, up, down) */
			for(round = 0; round < NUM_MOVES; round++) {
				move_t try_move = possible_actions[round];
				node_t *new_node = apply_action(popped_node, try_move);
				summary->num_generated ++;

				/* if the new node's board is not the same as it's parent then push 
				it onto the heap, or else delete it */
				if(new_node != NULL) {
					heap_push(&h, new_node); 
					propagate_back(propagation, new_node, first_action_scores);
				}
				else {
					free(new_node);
				}
			}
		}
	}
	/* free ALL memory allocated to the heap */
	emptyPQ(&h);
	
	/* free the explored array */
	for(i = 0; i < num; i++) {
		free(explored[i]);
		
	}
	free(explored);


	/* finds the max score from the possible scores that can be generated for each 
	of the four moves */
	for(i=0; i < NUM_MOVES; i++) {
		if(first_action_scores[i] > max_score) {
			max_score = first_action_scores[i];
		}
	}

	/* calculates the number of moves that can lead to the max score,and its index in 
	possible_actions[]  */
	for(i=0; i < NUM_MOVES; i++) {
		if (first_action_scores[i] == max_score) {
			is_max_score[i] = true;
			num_max_scores ++;
			index_max_score = i;
		}
	}

	/* when there is only one action leading to the max score */
	if(num_max_scores == 1) {
		best_action = possible_actions[index_max_score];
	}

	/* if there is more than one action leading to max score, then break ties 
	randomly*/
	else{
		int action = rand()%4;
		while(!is_max_score[action]) {
			action = rand()%4;
		}
		best_action = possible_actions[action];
	}

	return best_action;
}

/* Initialises the explored array so that each cell in the array is a null pointer */
void initialise_explored(node_t **explored, int start, int end) {
	int i;
	for(i=start; i < end; i++) {
		explored[i] = NULL;
	}

}

/* Returns the index value for first_action_scores[] depending on the type of move
specified in the input */
int set_move_index(move_t move) {
	if(move == left) {
		return 0;
	}
	else if(move == right) {
		return 1;
	}
	else if(move == up) {
		return 2;
	}
	else {
		return 3;
	}
}

/* Copies the contents of one board from a node to another given the boards as
inputs  */
void copy_board(uint8_t old_board[SIZE][SIZE], uint8_t new_board[SIZE][SIZE]) {
	int i, j;
	for(i=0; i <SIZE; i++) {
		for(j=0; j <SIZE; j++) {
			*(*(new_board+i)+j) = *(*(old_board+i)+j);
		}
	}

}

/* Returns the node that is created when particular action is applied to the node
supplied as a parameter */
node_t *apply_action(node_t *popped_node, move_t try_move) {
	uint8_t tmp_board[SIZE][SIZE];
	uint32_t tmp_score = popped_node->priority;
	copy_board(popped_node->board, tmp_board);

	node_t *new_node = (node_t *)malloc(sizeof(struct node_s));
	if(new_node == NULL) {
		printf("Error allocating memory for new node\n");
		exit(EXIT_FAILURE);
	}
	new_node->parent = popped_node;
	/* if the move was executed successfully and the new node's state is different
	to its parent node state then fill in the remaining variables in node_t */
	if(execute_move_t(tmp_board,&tmp_score, try_move)){
		addRandom(tmp_board);
		new_node->priority = tmp_score;
		new_node->depth = popped_node->depth + 1;
		new_node->num_childs = 0;
		new_node->move = try_move;
		copy_board(tmp_board, new_node->board);
		return new_node;
	}
	else {
		return NULL;
	}

	
}

/* Propagates a nodes score to the first action score depending on the type of 
propagation. 
-If it is average, then the first action node reflects the average
score that can created from any of its paths. 
-If maximum, then the first action node is only updated if the node's priority
is larger than that of the first action node */
void propagate_back(propagation_t propagation, node_t *node, 
	uint32_t first_action_scores []) {
	
	uint32_t tmp_node_score = node->priority;
	int i;
	uint32_t old_parent_score = (node->parent)->priority;
	uint32_t next_old_parent_score = 0;

	/* maximum propagation*/
	if(propagation == max) {
		(node->parent)->num_childs ++;
		while(((node->parent)->parent) != NULL) {
			node = node->parent;
		}
		if(node->priority < tmp_node_score) {
			node->priority = tmp_node_score;
			i = set_move_index(node->move);
			/* updates auxiliary score array */
			first_action_scores[i] = node->priority;
		}
	}

	/* average propagation */
	else {
		if(node->depth > 1) {
			/* update the parent's priority (input node's parent) */
			(node->parent)->priority = ((node->parent)->priority*
										(node->parent)->num_childs+node->priority)/
										(++((node->parent)->num_childs));
			node = (node->parent);
			while(((node->parent)->parent) != NULL ) { 
				next_old_parent_score = (node->parent)->priority;
				/* changes parent's prioirity so that it reflects the average 
				priority of all its pathways  */
				(node->parent)->priority = (uint32_t)((node->parent)->priority*
											(node->parent)->num_childs-
											old_parent_score+node->priority)/
											((node->parent)->num_childs);
				old_parent_score = next_old_parent_score;
				node = (node->parent);
			}
		}
		/* updates auxiliary score array */
		i = set_move_index(node->move);
		first_action_scores[i] = node->priority;
	}
}


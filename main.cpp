#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

const int SIZE = 8;
const int MINMAX_DEPTH = 5;

struct player {
    char color;
    char character;
    int time;
    int score = 0;
};
player my_player;


struct move {
    int start;
    int end;
    int score;
    std::vector<int> flippable_coors;
};


// ERROR
void throw_error(std::string message) {
    std::clog << message << std::endl;
    exit(1);
}


// VALIDATION
bool is_digits(const std::string& str) {
    return std::all_of(str.begin(), str.end(), ::isdigit);
}


bool right_game_chars(const std::string& str) {
    return str.find_first_not_of("-OX") == std::string::npos;
}


void check_game(std::string game) {
    if (game.size() != 64) {
        throw_error("Invalid game length.");
    }
    if (!right_game_chars(game)) {
        throw_error("Invalid coding.");
    }
}


void check_number(std::string number) {
    if (!is_digits(number)) {
        throw_error("Invalid number.");
    }
}


void check_color(std::string color) {
    if (!(color == "B" || color == "W")) {
        throw_error("Invalid color.");
    }
}


void check_length(std::vector<std::string>& parameters, int length) {
    if (parameters.size() != length) {
        throw_error("Invalid number of parameters.");
    }
}


// PARSING
void parse_command(std::string command, std::vector<std::string>& parameters) {
    std::istringstream iss(command);
    for (command; iss >> command; ) {
        parameters.push_back(command);
    }
}


// GAME MOVES
bool out_of_board(int act_spot, int neighbour) {
    if (((act_spot + neighbour) < 0) || ((act_spot + neighbour) > 63) || (((act_spot % SIZE) == 0) && ((neighbour % SIZE) == SIZE - 1)) || (((act_spot % SIZE) == SIZE - 1) && ((neighbour % SIZE) == 0))) {
        return true;
    }
    else {
        return false;
    }
}

bool empty_spot(char act_spot) {
    return act_spot == '-';
}


void check_neighbours(std::string& act_game, int position, std::vector<move> possible_moves, char player) {
    std::vector<int> neighbours{ -SIZE - 1, -SIZE, -SIZE + 1 , -1, +1, +SIZE - 1, +SIZE , +SIZE + 1 };

    for (int i = 0;i < neighbours.size();i++) {
        std::vector<int> flippable_temp;
        flippable_temp.push_back(position); // add start position
        int curr_position = position + neighbours[i];

        do {            
            // check out of board
            if (out_of_board(curr_position, neighbours[i])) {
                break;
            }
            // if next spot is also empty, nothing to flip            
            if (empty_spot(act_game[curr_position])) {
                break;
            }
            // if opposite color count sum, else break bcs found start point
            if (act_game[curr_position] != player) {
                flippable_temp.push_back(curr_position);
            }
            else {
                break;
            }
            curr_position += neighbours[i];
        } while (true);

        // if is at least one flipable spot, add to possible moves
        if (flippable_temp.size() > 0) {
            move movement;
            movement.start = curr_position;
            movement.end = position;
            movement.score = flippable_temp.size();
            movement.flippable_coors = flippable_temp;
            possible_moves.push_back(movement);
        }
    }
}


std::vector<move> find_possible_moves(std::string& act_game, char player) {
    // check if empty spot
    // check if neighbour is opposite color
    // while opposite color go in this direction
    // if free add to possible moves
    std::vector<move> possible_moves;

    for (int i = 0; i < act_game.size();i++) {
        if (empty_spot(act_game[i])) {
            check_neighbours(act_game, i, possible_moves, player);
        }
    }

    return possible_moves;
    // MUSIM VYMYSLIET AKO UCHOVAVAT MAX_SCORE - NEUCHOVAVAT SI VSETKY TAHY ALE VZDY TO POROVNAVAT LEN S JEDNYM MAXIMALNYM
    // PO NAJDENI POSSIBLE_MOVES:
        // VYBRAT MAX_SCORE
        // ZMENIT PODLA ULOZENEJ STRUKTURY ACT_GAME
}


void do_move(std::string& temp_game, move movement) {
    for (int i = 0;i < movement.flippable_coors.size();i++) {
        int coor = movement.flippable_coors[i];
        (my_player.character == 'O') ? (temp_game[coor] = 'O') : (temp_game[coor] = 'X');
    }
}


// MINMAX TREE THINGS
struct minmax_tree {
    minmax_tree** leaves;
    int leaves_count;
    //std::vector<move> move_list;
    //std::string game;
    int val;
};


minmax_tree *create_tree(int depth, std::string act_game) {
    minmax_tree *node;
    std::vector<move> act_move_list = find_possible_moves(act_game, my_player.character);
    node->leaves_count = act_move_list.size();

    // create leaves if we're not too deep and exists at least one move
    if (depth > 0 && node->leaves_count > 0) {
        node->leaves = new minmax_tree *[node->leaves_count];
        // for every leaf create nodes with updated board
        for (int i = 0;i < node->leaves_count;i++) {
            std::string temp_game;
            std::memcpy(&temp_game, &act_game, sizeof act_game);
            // change board to actual state
            do_move(temp_game, act_move_list[i]);
            node->leaves[i] = create_tree(depth - 1, temp_game);
        }
    }
    else {
        node->leaves = NULL;
    }

    return node;
}


// HEURISTIC FUNCTIONS TO COUNT VALUE OF MOVE
double calculate_parity(std::string act_game) {
    int my_coins = 0, opp_coins = 0;

    for (int i = 0; i < SIZE * SIZE; i++) {
        if (act_game[i] != '-') {
            if (act_game[i] == my_player.character) {
                my_coins++;
            }
            else if (act_game[i] != my_player.character) {
                opp_coins++;
            }
        }
    }

    if (my_coins > opp_coins) {
        return (100.0 * my_coins) / (my_coins + opp_coins);
    }
    else if (my_coins < opp_coins) {
        return -(100.0 * opp_coins) / (my_coins + opp_coins);
    }
    else {
        return 0;
    }
}


double calculate_mobility(std::string act_game) {
    char opp = (my_player.character == 'O') ? ('X') : ('O');
    std::vector<move> my_moves = find_possible_moves(act_game, my_player.character);
    std::vector<move> opp_moves = find_possible_moves(act_game, opp);

    if (my_moves.size() > opp_moves.size()) {
        return (100.0 * my_moves.size()) / (my_moves.size() + opp_moves.size());
    }
    else if (my_moves.size() < opp_moves.size()) {
        return -(100.0 * opp_moves.size()) / (my_moves.size() + opp_moves.size());
    }
    else {
        return 0;
    }
}


double calculate_corners(std::string act_game) {
    int my_coins = 0, opp_coins = 0;
    char opp = (my_player.character == 'O') ? ('X') : ('O');
    std::vector<int> corners = { 0, SIZE - 1, SIZE * (SIZE - 1), SIZE * SIZE - 1 };
    
    for (int i = 0;i < corners.size();i++) {
        if (act_game[corners[i]] == my_player.character) {
            my_coins++;
        }
        else if (act_game[corners[i]] == opp) {
            opp_coins++;
        }
    }

    return 25 * (my_coins - opp_coins); //HODIT ASI ZDROJ SKADE TIE VAHY IDK
}


double calculate_close_corners(std::string act_game) {
    int my_coins = 0, opp_coins = 0;
    char opp = (my_player.character == 'O') ? ('X') : ('O');
    std::vector<int> corners = { 0, SIZE - 1, SIZE * (SIZE - 1), SIZE * SIZE - 1 };
    std::vector<int> close_corners = {  1, SIZE + 1, SIZE,
                                        SIZE - 2, 2 * SIZE - 2, 2 * SIZE - 1, 
                                        (SIZE - 2) * SIZE, (SIZE - 2) * SIZE + 1, (SIZE - 1) * SIZE + 1, 
                                        (SIZE - 1) * SIZE - 1, (SIZE - 1) * SIZE - 2, SIZE * SIZE - 2 
                                    };
    
    for (int i = 0;i < corners.size();i++) {
        if (act_game[corners[i]] == '-') {
            for (int j = 0;j < 3;j++) {
                if (act_game[close_corners[i * 3 + j]] == my_player.character) {
                    my_coins++;
                }
                else if (act_game[close_corners[i * 3 + j]] == opp) {
                    opp_coins++;
                }
            }
        }
    }

    return -12.5 * (my_coins - opp_coins); //HODIT ASI ZDROJ SKADE TIE VAHY IDK
}


double heuristic(std::string act_game) {
    // 1. COIN PARITY: max-player coins - min-player coins
    double parity = calculate_parity(act_game);

    // 2. MOBILITY: possible number of moves for max/min player
    double mobility = calculate_mobility(act_game);

    // 3.1 CORNERS
    double corners = calculate_corners(act_game);

    // 3.2 CLOSE CORNERS
    double close_corners = calculate_close_corners(act_game);

    // 4. STABILITY

    // FINAL SCORE
    return parity + mobility + corners + close_corners + stability;
}


// AI program which returns optimal value for current player
double minimax(int depth, minmax_tree *node, bool maximizing, int alpha, int beta) {
    if (depth == 0) { //TU TREBA DODAT ZE KED BUDE GAME OVER
        //TU BUDU HEURISTIKY
        return heuristic();
    }

    if (maximizing) {
        int best_eval = INT_MIN;
        // Recur for left and right children
        for (int i = 0; i < node->leaves_count; i++) {
            int eval = minimax(depth - 1, node->leaves[i], false, alpha, beta);
            best_eval = std::max(best_eval, eval);
            alpha = std::max(alpha, best_eval);

            // Alpha Beta Pruning
            if (beta <= alpha) {
                break;
            }
        }
        return best_eval;
    }
    else {
        int best_eval = INT_MAX;
        // Recur for left and right children
        for (int i = 0; i < 2; i++) {
            int eval = minimax(depth - 1, node->leaves[i], true, alpha, beta);
            best_eval = std::min(best_eval, eval);
            beta = std::min(beta, best_eval);

            // Alpha Beta Pruning
            if (beta <= alpha) {
                break;
            }
        }
        return best_eval;
    }
}



// MAIN PROGRAM
int main(){
    std::string command;
    bool assignedStart = false;

    while (std::getline(std::cin, command)) {
        std::vector<std::string> parameters;        
        parse_command(command, parameters);
        
        if (parameters.size() > 0) {
            if (parameters[0] == "START") {
                // validation
                check_length(parameters, 3);
                check_color(parameters[1]);
                check_number(parameters[2]);
                // assigning
                my_player.color = parameters[1][0];   // string to char
                (my_player.color == 'W') ? (my_player.character = 'O') : (my_player.character = 'X');
                my_player.time = stoi(parameters[2]);
                assignedStart = true;
                std::cout << "1" << std::endl;
            }
            else if (parameters[0] == "STOP") {
                // validation
                check_length(parameters, 1);
                return 0;
            }
            else if (parameters[0] == "MOVE") {
                if (assignedStart) {
                    // validation
                    check_length(parameters, 2);
                    check_game(parameters[1]);
                    std::string act_game = parameters[1];
                    // for every turn create game tree
                    minmax_tree *tree = create_tree(MINMAX_DEPTH, act_game);
                    bool maximizer = (my_player.color == 'B') ? true : false;
                    // run minmax algorithm
                    int return_val = minimax(MINMAX_DEPTH, tree, INT_MIN, INT_MAX, maximizer);
                    // TU BUDE ESTE DO_MOVE
                    // PTM TREBA ODPOVEDAT TAHOM - TO MAM VPODTSTE V STRUKTURE MOVE
                    // TREBA DOIMPLEMENTOVAT CAS
                }
                else {
                    throw_error("START parameters not assigned.");
                }
            }
            else {
                throw_error("Unsupported command.");
            }
        }
        else {
            throw_error("Unsupported command.");
        }
    }

    return 0;
}

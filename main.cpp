#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

const int SIZE = 8;
const int MINMAX_DEPTH = 4;

struct player {
    char color;
    char character;
    int time;
};
player my_player;


struct move {
    size_t end;
    std::vector<size_t> flippable_coors;
};

void find_tile(size_t tile);


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
    int num = stoi(number);
    if (num < 2 || num > 30) {
        throw_error("Too large number.");
    }
}


void check_color(std::string color) {
    if (!(color == "B" || color == "W")) {
        throw_error("Invalid color.");
    }
}


void check_length(std::vector<std::string>& parameters, size_t length) {
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
bool out_of_board(size_t act_spot, int neighbour) {
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


void check_neighbours(std::string& act_game, size_t position, std::vector<move>& possible_moves, char player) {
    std::vector<int> neighbours{ -SIZE - 1, -SIZE, -SIZE + 1 , -1, +1, +SIZE - 1, +SIZE , +SIZE + 1 };
    std::vector<size_t> flippable_temp;
    flippable_temp.push_back(position); // add start position

    for (size_t i = 0;i < neighbours.size();i++) {
        size_t curr_position = position + neighbours[i];
        size_t counter = 0;

        do {            
            // check out of board
            if (out_of_board(curr_position - neighbours[i], neighbours[i])) {
                break;
            }
            // if next spot is also empty, nothing to flip            
            if (empty_spot(act_game[curr_position])) {
                // if there is a row with same color and then empty spot -> cannot flip
                flippable_temp.erase(flippable_temp.end() - counter, flippable_temp.end());
                break;
            }
            // if opposite color count sum, else break bcs found start point
            if (act_game[curr_position] != player) {
                flippable_temp.push_back(curr_position);
                counter++;
            }
            else {
                break;
            }
            curr_position += neighbours[i];
        } while (true);
    }

    // if is at least one flipable spot, add to possible moves
    if (flippable_temp.size() > 1) {
        move movement;
        movement.end = position;
        movement.flippable_coors = flippable_temp;
        possible_moves.push_back(movement);
    }
}


std::vector<move> find_possible_moves(std::string& act_game, char player) {
    std::vector<move> possible_moves;

    for (size_t i = 0; i < act_game.size();i++) {
        if (empty_spot(act_game[i])) {
            check_neighbours(act_game, i, possible_moves, player);
        }
    }

    return possible_moves;
}


void do_move(std::string& temp_game, move movement, char player) {
    for (size_t i = 0;i < movement.flippable_coors.size();i++) {
        size_t coor = movement.flippable_coors[i];
        (player == 'O') ? (temp_game[coor] = 'O') : (temp_game[coor] = 'X');
    }
}


// NEFUNGUJE GAME OVER IDEM SPAT MAM PICI
bool game_over(std::string& act_game) {
    char opp = (my_player.character == 'O') ? ('X') : ('O');
    std::vector<move> my_moves = find_possible_moves(act_game, my_player.character);
    std::vector<move> opp_moves = find_possible_moves(act_game, opp);

    return my_moves.size() == 0 && opp_moves.size() == 0;
}


// MINMAX TREE THINGS
struct minmax_tree {
    std::vector<minmax_tree> leaves;
    size_t leaves_count;
    std::string game;
    double value;
    size_t tile;
};


minmax_tree create_tree(int depth, std::string act_game, size_t act_tile, char player) {
    minmax_tree node;
    std::vector<move> act_move_list = find_possible_moves(act_game, player);
    node.leaves_count = act_move_list.size();
    node.game = act_game;
    node.tile = act_tile;

    if (depth == 3) {
        find_tile(node.tile);
    }

    // create leaves if we're not too deep and exists at least one move
    if (depth > 0 && node.leaves_count > 0) {

        char next_player = (player == 'O') ? ('X') : ('O');
        // for every leaf create nodes with updated board
        for (size_t i = 0;i < node.leaves_count;i++) {
            std::string temp_game = act_game;

            // change board to actual state
            do_move(temp_game, act_move_list[i], player);
            node.leaves.push_back(create_tree(depth - 1, temp_game, act_move_list[i].end, next_player));
        }
    }

    return node;
}


// heuristic functions to calculate value of movement
void calculate_parity_stability(std::string act_game, double& parity, double& empty_spot, double& disk_squares) {
    size_t my_coins = 0, opp_coins = 0, my_neighbour_empty = 0, opp_neighbour_empty = 0;
    disk_squares = 0;

    std::vector<int> neighbours{ -SIZE - 1, -SIZE, -SIZE + 1 , -1, +1, +SIZE - 1, +SIZE , +SIZE + 1 };

    std::vector<int> vec = {    20, -3, 11, 8, 8, 11, -3, 20,
                                -3, -7, -4, 1, 1, -4, -7, -3,
                                11, -4, 2, 2, 2, 2, -4, 11,
                                8, 1, 2, -3, -3, 2, 1, 8,
                                8, 1, 2, -3, -3, 2, 1, 8,
                                11, -4, 2, 2, 2, 2, -4, 11,
                                -3, -7, -4, 1, 1, -4, -7, -3,
                                20, -3, 11, 8, 8, 11, -3, 20    };

    for (int i = 0; i < SIZE * SIZE; i++) {
        if (act_game[i] != '-') {
            // calculates number of coins & weight of tiles
            if (act_game[i] == my_player.character) {
                my_coins++;
                disk_squares += vec[i];
            }
            else {
                opp_coins++;
                disk_squares -= vec[i];
            }
            // calculates empty spots around tile
            for (int j = 0; j < SIZE; j++) {
                int neighbour = i + neighbours[j];
                if (!out_of_board(i, neighbours[j]) && act_game[neighbour] == '-') {
                    if (act_game[i] == my_player.character) {
                        my_neighbour_empty++;
                    }
                    else {
                        opp_neighbour_empty++;
                    }
                }
            }
        }
    }

    // calculates difference between coins
    if (my_coins > opp_coins) {
        parity = (100.0 * my_coins) / (my_coins + opp_coins);
    }
    else if (my_coins < opp_coins) {
        parity = -(100.0 * opp_coins) / (my_coins + opp_coins);
    }
    else {
        parity = 0;
    }

    // calculates empty spots around tile
    if (my_neighbour_empty > opp_neighbour_empty) {
        empty_spot = -(100.0 * my_neighbour_empty) / (my_neighbour_empty + opp_neighbour_empty);
    }
    else if (my_neighbour_empty < opp_neighbour_empty) {
        empty_spot = (100.0 * opp_neighbour_empty) / (my_neighbour_empty + opp_neighbour_empty);
    }
    else {
        empty_spot = 0;
    }
}


void calculate_mobility(std::string act_game, double& mobility) {
    char opp = (my_player.character == 'O') ? ('X') : ('O');
    std::vector<move> my_moves = find_possible_moves(act_game, my_player.character);
    std::vector<move> opp_moves = find_possible_moves(act_game, opp);

    if (my_moves.size() > opp_moves.size()) {
        mobility = (100.0 * my_moves.size()) / (my_moves.size() + opp_moves.size());
    }
    else if (my_moves.size() < opp_moves.size()) {
        mobility =  -(100.0 * opp_moves.size()) / (my_moves.size() + opp_moves.size());
    }
    else {
        mobility = 0;
    }
}


void calculate_corners(std::string act_game, double& corners, double& close_corners) {
    int my_corners = 0, opp_corners = 0, my_close_corners = 0, opp_close_corners = 0;
    char opp = (my_player.character == 'O') ? ('X') : ('O');

    std::vector<size_t> corners_coor = { 0, SIZE - 1, SIZE * (SIZE - 1), SIZE * SIZE - 1 };
    std::vector<size_t> close_corners_coor = {  1, SIZE + 1, SIZE,
                                                SIZE - 2, 2 * SIZE - 2, 2 * SIZE - 1,
                                                (SIZE - 2) * SIZE, (SIZE - 2) * SIZE + 1, (SIZE - 1) * SIZE + 1,
                                                (SIZE - 1) * SIZE - 1, (SIZE - 1) * SIZE - 2, SIZE * SIZE - 2
                                             };
    
    for (size_t i = 0;i < corners_coor.size();i++) {
        if (act_game[corners_coor[i]] == my_player.character) {
            my_corners++;
        }
        else if (act_game[corners_coor[i]] == opp) {
            opp_corners++;
        }
        else {
            for (size_t j = 0;j < 3;j++) {
                if (act_game[close_corners_coor[i * 3 + j]] == my_player.character) {
                    my_close_corners++;
                }
                else if (act_game[close_corners_coor[i * 3 + j]] == opp) {
                    opp_close_corners++;
                }
            }
        }
    }

    corners = 25. * (my_corners - opp_corners);
    close_corners = -12.5 * (my_close_corners - opp_close_corners);
}


double heuristic(std::string act_game) {
    double parity, mobility, corners, close_corners, empty_spots, disk_squares;

    // 1.1 COIN PARITY: difference between max-player and min-player coins
    // 1.2 STABILITY: how stable coins are
    calculate_parity_stability(act_game, parity, empty_spots, disk_squares);

    // 2. MOBILITY: possible number of moves for max/min player
    calculate_mobility(act_game, mobility);

    // 3. CORNERS + CLOSE CORNERS
    calculate_corners(act_game, corners, close_corners);

    // FINAL SCORE - adding different weights to different evaluations (from source)
    // std::cout << "PARITY " << parity << std::endl << "CORNERS " << corners << std::endl << "CLOSE CORNERS " << close_corners << std::endl << "MOBILITY " << mobility << std::endl << "EMPTY SPOTS " << empty_spots << std::endl << "DISK SQUARES " << disk_squares << std::endl << "-----" << std::endl;
    return (10 * parity) + (801.724 * corners) + (382.026 * close_corners) + (78.922 * mobility) + (74.396 * empty_spots) + (10 * disk_squares);
}


// AI program which returns optimal value for current player
double minmax(int depth, minmax_tree& node, bool maximizing, double alpha, double beta) {
    if (depth == 0 || game_over(node.game)) {
        return heuristic(node.game);
    }

    if (maximizing) {
        double max_val = -DBL_MAX;

        // recur for every leaf
        for (size_t i = 0; i < node.leaves_count; i++) {
            double eval = minmax(depth - 1, node.leaves[i], false, alpha, beta);
            max_val = std::max(max_val, eval);

            // alpha beta pruning
            alpha = std::max(alpha, max_val);
            if (beta <= alpha) {
                break;
            }
        }
        node.value = max_val;
        return max_val;
    }
    else {
        double min_val = DBL_MAX;

        // recur for every leaf
        for (int i = 0; i < 2; i++) {
            double eval = minmax(depth - 1, node.leaves[i], true, alpha, beta);
            min_val = std::min(min_val, eval);
            beta = std::min(beta, min_val);

            // alpha beta pruning
            if (beta <= alpha) {
                break;
            }
        }
        node.value = min_val;
        return min_val;
    }
}


// helper function to answer with tile
void find_tile(size_t tile) {
    size_t row = tile / SIZE;
    size_t col_num = tile % SIZE;
    char col = '-';

    switch (col_num) {
    case 0:
        col = 'A';
        break;
    case 1:
        col = 'B';
        break;
    case 2:
        col = 'C';
        break;
    case 3:
        col = 'D';
        break;
    case 4:
        col = 'E';
        break;
    case 5:
        col = 'F';
        break;
    case 6:
        col = 'G';
        break;
    case 7:
        col = 'H';
        break;
    }

    std::cout << col << row + 1 << std::endl;
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
                //std::cout << "MOVE ---------------------------OX------XO---------------------------";
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
                    minmax_tree tree = create_tree(MINMAX_DEPTH, act_game, SIZE*SIZE, my_player.character);  // bcs SIZE^2 is out of board
                    bool maximizer = (my_player.color == 'B') ? true : false;

                    // run minmax algorithm
                    double return_val = minmax(MINMAX_DEPTH, tree, maximizer, -DBL_MAX, DBL_MAX);

                    // traverse through first layer of tree to find node with optimal value
                    for (size_t i = 0;i < tree.leaves_count;i++) {
                        if (tree.leaves[i].value == return_val) {

                            // find tile and answer
                            std::cout << "-----------" << std::endl;
                            find_tile(tree.leaves[i].tile);
                            break;
                        }
                    }

                    parameters.clear();
                    // TREBA DOIMPLEMENTOVAT CAS + KED RAC NEMA TAH DAVA PASS

                    // TU BUDE ODPOVED POCITACA PRE OTESTOVANIE
                    std::string pc_game;
                    std::cout << "PCs turn" << std::endl;
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

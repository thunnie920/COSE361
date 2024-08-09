#pragma warning(disable : 4996)
#include <stdio.h>
#include <Windows.h>
#include <math.h>
#include <conio.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>
#include <float.h>
#include <stdbool.h>
#include <string.h>

#define BOARD_SIZE 19
#define C sqrt(2.0) //UCB1 constant
#define BLOCKED_FOUR_SCORE 5000 // ������ �ݼ�(5�� ���ӵ� ���� ����)�� �ο��Ǵ� ����
#define FOUR_SCORE 1000 // �ڽ��� �ݼ�(5�� ���ӵ� ��)�� �ο��Ǵ� ����
#define BLOCKED_THREE_SCORE 100 // ������ ���γ� ���� 4�� �� �ϳ��� ������ �ο��Ǵ� ����
#define THREE_SCORE 10 // �ڽ��� ���γ� ���� 4�� �� �ϳ��� �ϼ��Կ� �ο��Ǵ� ����
#define BLOCKED_TWO_SCORE 5 // ������ ���γ� ���� 3�� �� �ϳ��� ������ �ο��Ǵ� ����
#define TWO_SCORE 1 // �ڽ��� ���γ� ���� 3�� �� �ϳ��� �ϼ��Կ� �ο��Ǵ� ����

wchar_t board[BOARD_SIZE][BOARD_SIZE];
void omokBoard();
char gameWin(wchar_t board[BOARD_SIZE][BOARD_SIZE]);
int setTimeLimit;
char playerChoice;
char aiChoice;
void gameMove(char playerChoice, char aiChoice, int setTimeLimit);

typedef enum {
    REAL_PLAYER = 1,
    AI_PLAYER = 2
} Player;

struct State {
    wchar_t board[BOARD_SIZE][BOARD_SIZE];
    Player currentPlayer;
    int previousOpponentMoveX;
    int previousOpponentMoveY;
} typedef State;

State currentState;

struct Node {
    State* state;
    struct Node* parent; //�θ� ���
    struct Node** children; //�ڽ� ���
    int numChildren; //�ڽ� ��� ��
    int wins;
    int visits;
    int moveX; // ��忡�� ����� ������ board[y][x] ���� x����
    int moveY; // ��忡�� ����� ������ board[y][x] ���� y����
} typedef Node;

struct Move {
    int x;
    int y;
} typedef Move;

struct BestMove {
    int x;
    int y;
} typedef BestMove;

Node* createNode(State state) { //tree ����
    Node* newNode = (Node*)malloc(sizeof(Node));
    State* newState = (State*)malloc(sizeof(State));
    memcpy(newState, &state, sizeof(State));

    newNode->state = newState;
    newNode->moveX = -1; // ���� ����� �̵��� ����
    newNode->moveY = -1; // ���� ����� �̵��� ����
    newNode->wins = 0;
    newNode->visits = 0;
    newNode->parent = NULL;
    newNode->children = NULL;
    newNode->numChildren = 0;

    return newNode;
}

Node* selectNode(Node* node, int prevOpponentMoveX, int prevOpponentMoveY) {
    while (node->numChildren > 0) {
        double maxUCB1 = -DBL_MAX;
        Node* bestChild = NULL;
        for (int i = 0; i < node->numChildren; i++) {
            Node* child = node->children[i];
            double UCB1;
            if (child->visits > 0) {
                double explorationTerm = C * sqrt(log(node->visits) / child->visits);
                double exploitationTerm = ((double)child->wins / (double)child->visits);
                double proximityTerm = 0.0;
                if (prevOpponentMoveX != -1 && prevOpponentMoveY != -1) {
                    double distance = sqrt(pow(child->moveX - prevOpponentMoveX, 2) + pow(child->moveY - prevOpponentMoveY, 2));
                    proximityTerm = 1.0 / (1.0 + distance); // �ʿ信 ���� �� ��� ����
                }
                UCB1 = exploitationTerm + explorationTerm + proximityTerm;
            }
            else {
                UCB1 = DBL_MAX;
            }
            if (UCB1 > maxUCB1) {
                maxUCB1 = UCB1;
                bestChild = child;
            }
        }
        if (bestChild != NULL) {
            node = bestChild;
        }
        else {
            break;
        }
    }
    return node;
}

Move* generateNextMove(Node* node, int* numMoves) {
    Move* moves = (Move*)malloc(BOARD_SIZE * BOARD_SIZE * sizeof(Move)); //�ִ� ������ �̵��� ��
    int count = 0;
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (node->state->board[y][x] == L'\u253C') {
                moves[count].x = x;
                moves[count].y = y;
                count++;
            }
        }
    }
    *numMoves = count; // ������ �̵��� ���� �����մϴ�.
    return moves; // ������ ��� �̵��� �迭�� ��ȯ�մϴ�.
}

State* applyMove(Node* node, Move move, char playerChoice) {
    State* newState = (State*)malloc(sizeof(State));
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            newState->board[y][x] = node->state->board[y][x];
        }
    } //��������� ����
    newState->currentPlayer = node->state->currentPlayer;
    wchar_t playerDol;
    wchar_t aiDol;
    playerDol = (playerChoice == 'B') ? L'��' : L'��';
    aiDol = (playerChoice == 'B') ? L'��' : L'��';

    if (newState->currentPlayer == REAL_PLAYER) { //�̵�
        newState->board[move.y][move.x] = playerDol;
    }
    else {
        newState->board[move.y][move.x] = aiDol;
    }

    if (newState->currentPlayer == REAL_PLAYER) { //�÷��̾���ȯ
        newState->currentPlayer = AI_PLAYER;
    }
    else {
        newState->currentPlayer = REAL_PLAYER;
    }

    return newState;
}

int horizontalCount(wchar_t board[BOARD_SIZE][BOARD_SIZE], int y, int x, wchar_t playerDol) { //����
    int count = 0;

    for (int i = x; i >= 0; i--) {
        if (board[y][i] == playerDol) {
            count++;
        }
        else {
            break;
        }
    }
    for (int i = x + 1; i < BOARD_SIZE; i++) {
        if (board[y][i] == playerDol) {
            count++;
        }
        else {
            break;
        }
    }
    return count;
}

int verticalCount(wchar_t board[BOARD_SIZE][BOARD_SIZE], int y, int x, wchar_t playerDol) { //����
    int count = 0;

    for (int i = y; i >= 0; i--) {
        if (board[i][x] == playerDol) {
            count++;
        }
        else {
            break;
        }
    }
    for (int i = y + 1; i < BOARD_SIZE; i++) {
        if (board[i][x] == playerDol) {
            count++;
        }
        else {
            break;
        }
    }
    return count;
}

int diagonalCount(wchar_t board[BOARD_SIZE][BOARD_SIZE], int y, int x, wchar_t playerDol) { //�밢��
    int count = 0;

    for (int i = y, j = x; i >= 0 && j >= 0; i--, j--) {
        if (board[i][j] == playerDol) {
            count++;
        }
        else {
            break;
        }
    }
    for (int i = y + 1, j = x + 1; i < BOARD_SIZE && j < BOARD_SIZE; i++, j++) {
        if (board[i][j] == playerDol) {
            count++;
        }
        else {
            break;
        }
    }
    return count;
}

int antiDiagonalCount(wchar_t board[BOARD_SIZE][BOARD_SIZE], int y, int x, wchar_t playerDol) { //�ݴ�밢��
    int count = 0;

    for (int i = y, j = x; i >= 0 && j < BOARD_SIZE; i--, j++) {
        if (board[i][j] == playerDol) {
            count++;
        }
        else {
            break;
        }
    }
    for (int i = y + 1, j = x - 1; i < BOARD_SIZE && j >= 0; i++, j--) {
        if (board[i][j] == playerDol) {
            count++;
        }
        else {
            break;
        }
    }
    return count;
}

void calculateScores(wchar_t board[BOARD_SIZE][BOARD_SIZE], char playerChoice, int scores[BOARD_SIZE][BOARD_SIZE]) {
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (board[y][x] != L'\u253C') {// �� ĭ�� �ƴ� ��� ���� ����� ���� ����
                scores[y][x] = 0;
                continue;
            }
            int score = 0;
            wchar_t playerDol = (playerChoice == 'B') ? L'��' : L'��';
            int horizontalScore = horizontalCount(board, y, x, playerDol); //���ι�������
            int verticalScore = verticalCount(board, y, x, playerDol); //���ι�������
            int diagonalScore = diagonalCount(board, y, x, playerDol); //�밢��
            int antiDiagonalScore = antiDiagonalCount(board, y, x, playerDol); //�ݴ�밢��

            if (horizontalScore >= 5 || verticalScore >= 5 || diagonalScore >= 5 || antiDiagonalScore >= 5) { //���� 5�� ���ӵ� ����
                score += BLOCKED_FOUR_SCORE;
            }
            else if (horizontalScore == 5 || verticalScore == 5 || diagonalScore == 5 || antiDiagonalScore == 5) { //�ڽ� 5�� ���ӵ� �α�
                score += FOUR_SCORE;
            }
            else if (horizontalScore == 4 || verticalScore == 4 || diagonalScore == 4 || antiDiagonalScore == 4) { //���� 4�� �� �ϳ��� ����
            }
            else if (horizontalScore >= 3 || verticalScore >= 3 || diagonalScore >= 3 || antiDiagonalScore >= 3) { //�ڽ� 4�� �� �ϳ� �ϼ�
                score += THREE_SCORE;
            }
            else if (horizontalScore == 3 || verticalScore == 3 || diagonalScore == 3 || antiDiagonalScore == 3) { //���� 3�� �� �ϳ��� ����
                score += BLOCKED_TWO_SCORE;
            }
            else if (horizontalScore >= 2 || verticalScore >= 2 || diagonalScore >= 2 || antiDiagonalScore >= 2) { //�ڽ� 3�� �� �ϳ� �ϼ�
                score += TWO_SCORE;
            }
            scores[y][x] = score;
        }
    }
}

void expandNode(Node* node, char playerChoice) {
    wchar_t playerDol = (playerChoice == 'B') ? L'��' : L'��';
    int numMoves;
    Move* possibleMoves = generateNextMove(node, &numMoves);
    node->children = (Node**)malloc(numMoves * sizeof(Node*));
    int numValidMoves = 0;

    for (int i = 0; i < numMoves; i++) { //���� �÷��̾ �� �� �ֺ��� Ž��
        bool isValidMove = false;
        int moveX = possibleMoves[i].x;
        int moveY = possibleMoves[i].y;

        int distanceToPrevMoveX = abs(moveX - node->state->previousOpponentMoveX);
        int distanceToPrevMoveY = abs(moveY - node->state->previousOpponentMoveY);
        int maxDistance = 7; // �ִ� ��� �Ÿ�

        if (distanceToPrevMoveX <= maxDistance && distanceToPrevMoveY <= maxDistance) {// ���� �ܰ���� �Ÿ��� ��� ���� �̳��� �ִ��� Ȯ��
            for (int dy = -3; dy <= 3; dy++) { // �÷��̾ �� �� �ֺ��� Ž��
                for (int dx = -3; dx <= 3; dx++) {
                    int x = moveX + dx;
                    int y = moveY + dy;
                    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && node->state->board[y][x] == playerDol) {
                        isValidMove = true;
                    }
                }
            }
        }
        if (isValidMove) {
            Node* child = (Node*)malloc(sizeof(Node));
            State* newState = applyMove(node, possibleMoves[i], playerChoice);
            child->state = newState;
            child->parent = node;
            child->children = NULL;
            child->numChildren = 0;
            child->wins = 0;
            child->visits = 0;
            child->moveX = moveX;
            child->moveY = moveY;

            int scores[BOARD_SIZE][BOARD_SIZE];
            calculateScores(newState->board, playerChoice, scores);

            // ������ ���� ������� �ڽ� ��带 Ȯ��
            Node* bestChild = selectNode(child, newState->previousOpponentMoveX, newState->previousOpponentMoveY);
            node->children[numValidMoves] = bestChild;
            numValidMoves++;
        }
    }

    node->numChildren = numValidMoves; // ������ Ȯ��� �ڽ� ����� ���� ������Ʈ
    free(possibleMoves);
}

bool isBoardFull(wchar_t board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == L'\u253C') {
                return false;
            }
        }
    }
    return true;
}


double simulateNode(Node* node, char playerChoice) { //simulation
    State* currentState = node->state;
    wchar_t playerDol = (playerChoice == 'B') ? L'��' : L'��';
    wchar_t aiDol = (playerChoice == 'B') ? L'��' : L'��';
    while (!isBoardFull(currentState->board)) { //���尡 �� �� ���� �ʴٸ�
        int bestX = -1;
        int bestY = -1;
        for (int y = 9; y < BOARD_SIZE; ++y) {
            for (int x = 9; x < BOARD_SIZE; ++x) {
                if (currentState->board[y][x] == L'\u253c') {
                    currentState->board[y][x] = (currentState->currentPlayer == REAL_PLAYER) ? playerDol : aiDol;
                    if (gameWin(currentState->board) == playerChoice) {
                        bestX = x;
                        bestY = y;
                    }
                    currentState->board[y][x] = L'\u253c';
                }
            }
        }
        if (bestX == -1) {
            do {
                bestX = rand() % BOARD_SIZE;
                bestY = rand() % BOARD_SIZE;
            } while (currentState->board[bestY][bestX] != L'\u253c');
        }
        currentState->board[bestY][bestX] = (currentState->currentPlayer == REAL_PLAYER) ? playerDol : aiDol;
        char winner = gameWin(currentState->board);
        if (winner != ' ') {
            return (winner == playerChoice) ? -1.0 : 1.0;
        }
    }
    // �� ������ ���� ��� ���ºη� ó��
    return 0.0;
}

void backPropagate(double result, Node* node) { //backPropagation
    // ���� ������ ��Ʈ ������ �Ž��� �ö󰡸鼭 �÷��� Ƚ���� ������ ������Ʈ
    while (node != NULL) {
        node->visits++;
        node->wins += result;
        node = node->parent;
    }
}

BestMove selectMostPlayedNode(Node* node) {
    BestMove bestMove = { .x = -1, .y = -1 }; // �⺻������ 9 ����
    if (node->numChildren == 0) {
        return bestMove; // �ڽ� ��尡 ������ �⺻�� ��ȯ
    }
    int maxPlays = -1; // �ִ� �÷��� Ƚ���� �����ϱ� ���� ����
    for (int i = 0; i < node->numChildren; i++) {
        Node* childNode = node->children[i];
        if (childNode->visits > maxPlays) {
            maxPlays = childNode->visits;
            bestMove.x = childNode->moveX;
            bestMove.y = childNode->moveY;
        }
    }
    return bestMove;
}

bool findBlockingPositionForThreeInRow(Node* node, Move* blockMove, wchar_t playerDol) {
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE - 3; x++) { // ���� ���� ���� Ȯ��
            int count = 0;
            for (int i = 0; i < 3; i++) {
                if (node->state->board[y][x + i] == playerDol) {
                    count++;
                }
                else if (node->state->board[y][x + i] != L'\u253C') { // �� ĭ�� �ƴ϶��
                    break; // �ٸ� ���� �����Ƿ� �� ��ġ������ ���ӵ� 3���� �� �� ����
                }
            }
            if (count == 3) {
                // ���ӵ� 3���� ���� ã������, ���� ���� �� ĭ���� Ȯ��
                if (x > 0 && node->state->board[y][x - 1] == L'\u253C') {
                    blockMove->x = x - 1;
                    blockMove->y = y;
                    return true;
                }
                else if (x + 3 < BOARD_SIZE && node->state->board[y][x + 3] == L'\u253C') {
                    blockMove->x = x + 3;
                    blockMove->y = y;
                    return true;
                }
            }
        }
    }
    for (int x = 0; x < BOARD_SIZE; x++) { // ���� ���� Ȯ��
        for (int y = 0; y < BOARD_SIZE - 3; y++) { // ���� ���� ���� Ȯ��
            int count = 0;
            for (int i = 0; i < 3; i++) {
                if (node->state->board[y + i][x] == playerDol) {
                    count++;
                }
                else if (node->state->board[y + i][x] != L'\u253C') { // �� ĭ�� �ƴ϶��
                    break; // �ٸ� ���� �����Ƿ� �� ��ġ������ ���ӵ� 3���� �� �� ����
                }
            }
            if (count == 3) {
                // ���ӵ� 3���� ���� ã������, ��, �Ʒ��� �� ĭ���� Ȯ��
                if (y > 0 && node->state->board[y - 1][x] == L'\u253C') {
                    blockMove->x = x;
                    blockMove->y = y - 1;
                    return true;
                }
                else if (y + 3 < BOARD_SIZE && node->state->board[y + 3][x] == L'\u253C') {
                    blockMove->x = x;
                    blockMove->y = y + 3;
                    return true;
                }
            }
        }
    }
    for (int x = 0; x < BOARD_SIZE - 2; x++) { // ����� �밢�� ���� Ȯ��
        for (int y = 2; y < BOARD_SIZE; y++) { // ����� �밢�� ���� Ȯ��
            int count = 0;
            for (int i = 0; i < 3; i++) {
                if (node->state->board[y - i][x + i] == playerDol) {
                    count++;
                }
                else if (node->state->board[y - i][x + i] != L'\u253C') { // �� ĭ�� �ƴ϶��
                    break; // �ٸ� ���� �����Ƿ� �� ��ġ������ ���ӵ� 3���� �� �� ����
                }
            }
            if (count == 3) {
                // ���ӵ� 3���� ���� ã������, ���� ���� �� ĭ���� Ȯ��
                if (y + 1 < BOARD_SIZE && x > 0 && node->state->board[y + 1][x - 1] == L'\u253C') {
                    blockMove->x = x - 1;
                    blockMove->y = y + 1;
                    return true;
                }
                else if (y - 3 >= 0 && x + 3 < BOARD_SIZE && node->state->board[y - 3][x + 3] == L'\u253C') {
                    blockMove->x = x + 3;
                    blockMove->y = y - 3;
                    return true;
                }
            }
        }
    }
    for (int x = 2; x < BOARD_SIZE; x++) { // �»��� �밢�� ���� Ȯ��
        for (int y = 2; y < BOARD_SIZE; y++) { // �»��� �밢�� ���� Ȯ��
            int count = 0;
            for (int i = 0; i < 3; i++) {
                if (node->state->board[y - i][x - i] == playerDol) {
                    count++;
                }
                else if (node->state->board[y - i][x - i] != L'\u253C') { // �� ĭ�� �ƴ϶��
                    break; // �ٸ� ���� �����Ƿ� �� ��ġ������ ���ӵ� 3���� �� �� ����
                }
            }
            if (count == 3) {
                // ���ӵ� 3���� ���� ã������, ���� ���� �� ĭ���� Ȯ��
                if (y + 1 < BOARD_SIZE && x + 1 < BOARD_SIZE && node->state->board[y + 1][x + 1] == L'\u253C') {
                    blockMove->x = x + 1;
                    blockMove->y = y + 1;
                    return true;
                }
                else if (y - 3 >= 0 && x - 3 >= 0 && node->state->board[y - 3][x - 3] == L'\u253C') {
                    blockMove->x = x - 3;
                    blockMove->y = y - 3;
                    return true;
                }
            }
        }
    }
    return false; // ������ ��ġ�� ã�� ����
}

bool findWinningPositionForThreeInRow(Node* node, Move* winningMove, wchar_t aiDol) {
    for (int y = 0; y < BOARD_SIZE; y++) { // ���� �������� ���ӵ� ���� 3�� �ִ��� Ȯ��
        for (int x = 0; x <= BOARD_SIZE - 4; x++) { // ���� ���� Ȯ��
            int count = 0; // ���ӵ� ���� ������ ���� ����
            for (int i = 0; i < 4; i++) {
                if (node->state->board[y][x + i] == aiDol) {
                    count++;
                }
                else {
                    break; // ���ӵ��� ������ �ٷ� ���� ��ġ Ȯ��
                }
            }
            if (count >= 3) {
                // ���ӵ� �� ���� ���� ã��. ���� ���� ���� Ȯ���Ͽ� �¸��� ��ġ ã��
                if (x > 0 && node->state->board[y][x - 1] == L'\u253C') { // ���ʿ� �� ĭ�� �ִ� ���
                    winningMove->x = x - 1;
                    winningMove->y = y;
                    return true; // �¸��� �� �ִ� ��ġ�� ã�����Ƿ� true ��ȯ
                }
                if (x + 3 < BOARD_SIZE && node->state->board[y][x + 3] == L'\u253C') { // �����ʿ� �� ĭ�� �ִ� ���
                    winningMove->x = x + 3;
                    winningMove->y = y;
                    return true; // �¸��� �� �ִ� ��ġ�� ã�����Ƿ� true ��ȯ
                }
            }
        }
    }
    for (int x = 0; x < BOARD_SIZE; x++) { // ���� �������� ���ӵ� ���� 3�� �ִ��� Ȯ��
        for (int y = 0; y <= BOARD_SIZE - 4; y++) { // ���� ���� Ȯ��
            int count = 0; // ���ӵ� ���� ������ ���� ����
            for (int i = 0; i < 4; i++) {
                if (node->state->board[y + i][x] == aiDol) {
                    count++;
                }
                else {
                    break; // ���ӵ��� ������ �ٷ� ���� ��ġ Ȯ��
                }
            }

            if (count >= 3) {
                // ���ӵ� �� ���� ���� ã��. ���� ���� �Ǵ� �Ʒ����� Ȯ���Ͽ� �¸��� ��ġ ã��
                if (y > 0 && node->state->board[y - 1][x] == L'\u253C') { // ���ʿ� �� ĭ�� �ִ� ���
                    winningMove->x = x;
                    winningMove->y = y - 1;
                    return true; // �¸��� �� �ִ� ��ġ�� ã�����Ƿ� true ��ȯ
                }
                if (y + 3 < BOARD_SIZE && node->state->board[y + 3][x] == L'\u253C') { // �Ʒ��ʿ� �� ĭ�� �ִ� ���
                    winningMove->x = x;
                    winningMove->y = y + 3;
                    return true; // �¸��� �� �ִ� ��ġ�� ã�����Ƿ� true ��ȯ
                }
            }
        }
    }
    for (int y = BOARD_SIZE - 1; y >= 3; y--) { // ������ �Ʒ��ʿ��� ���� // ����� �밢�� �������� ���ӵ� ���� 3�� �ִ��� Ȯ��
        for (int x = 0; x <= BOARD_SIZE - 4; x++) { // ���ʿ��� ����
            int count = 0; // ���ӵ� ���� ������ ���� ����
            for (int i = 0; i < 4; i++) {
                if (node->state->board[y - i][x + i] == aiDol) {
                    count++;
                }
                else {
                    break; // ���ӵ��� ������ �ٷ� ���� ��ġ Ȯ��
                }
            }

            if (count >= 3) {
                // ���ӵ� �� ���� ���� ã��. ���� Ȯ���� �� �ִ� ��ġ ã��
                if (y < BOARD_SIZE - 1 && x > 0 && node->state->board[y + 1][x - 1] == L'\u253C') { // ���� �Ʒ��� �� ĭ�� �ִ� ���
                    winningMove->x = x - 1;
                    winningMove->y = y + 1;
                    return true; // �¸��� �� �ִ� ��ġ�� ã�����Ƿ� true ��ȯ
                }
                if (y - 3 >= 0 && x + 3 < BOARD_SIZE && node->state->board[y - 3][x + 3] == L'\u253C') { // ������ ���� �� ĭ�� �ִ� ���
                    winningMove->x = x + 3;
                    winningMove->y = y - 3;
                    return true; // �¸��� �� �ִ� ��ġ�� ã�����Ƿ� true ��ȯ
                }
            }
        }
    }
    for (int y = BOARD_SIZE - 1; y >= 3; y--) { // ������ �Ʒ��ʿ��� ���� // �»��� �밢�� �������� ���ӵ� ���� 3�� �ִ��� Ȯ��
        for (int x = BOARD_SIZE - 1; x >= 3; x--) { // �����ʿ��� ����
            int count = 0; // ���ӵ� ���� ������ ���� ����
            for (int i = 0; i < 4; i++) {
                if (node->state->board[y - i][x - i] == aiDol) {
                    count++;
                }
                else {
                    break; // ���ӵ��� ������ �ٷ� ���� ��ġ Ȯ��
                }
            }

            if (count >= 3) {
                // ���ӵ� �� ���� ���� ã��. ���� Ȯ���� �� �ִ� ��ġ ã��
                if (y < BOARD_SIZE - 1 && x < BOARD_SIZE - 1 && node->state->board[y + 1][x + 1] == L'\u253C') { // ������ �Ʒ��� �� ĭ�� �ִ� ���
                    winningMove->x = x + 1;
                    winningMove->y = y + 1;
                    return true; // �¸��� �� �ִ� ��ġ�� ã�����Ƿ� true ��ȯ
                }
                if (y - 3 >= 0 && x - 3 >= 0 && node->state->board[y - 3][x - 3] == L'\u253C') { // ���� ���� �� ĭ�� �ִ� ���
                    winningMove->x = x - 3;
                    winningMove->y = y - 3;
                    return true; // �¸��� �� �ִ� ��ġ�� ã�����Ƿ� true ��ȯ
                }
            }
        }
    }


    return false; // �¸��� �� �ִ� ��ġ�� ã�� ����
}

BestMove mctSearch(State state, int setTimeLimit, char playerChoice, int iterations) {
    clock_t startTime = clock(); // ���� �ð� ����
    Node* tree = createNode(state);
    int timeLimit = setTimeLimit * 1000; // clock() ��ȯ���� �°� �ð� ���� ����

    for (int i = 0; (i < iterations) && ((clock() - startTime) < timeLimit); i++) {
        Node* leaf = selectNode(tree, tree->state->previousOpponentMoveX, tree->state->previousOpponentMoveY); // ���� ����� �̵� ��ǥ ����
        expandNode(leaf, playerChoice); // ��� Ȯ��
        Node* child = selectNode(leaf, leaf->state->previousOpponentMoveX, leaf->state->previousOpponentMoveY); // Ȯ��� ��� �� �ϳ� ����


        double result = simulateNode(child, playerChoice); // �ùķ��̼� ����
        backPropagate(result, child); // ��� ������
    }

    Move blockMove;
    Move winMove;
    wchar_t playerDol = (playerChoice == 'B') ? L'��' : L'��';
    wchar_t aiDol = (playerChoice == 'B') ? L'��' : L'��';
    wchar_t currentDol = (state.currentPlayer == REAL_PLAYER) ? playerDol : aiDol;
    if (findWinningPositionForThreeInRow(tree, &blockMove, aiDol)) {
        BestMove bestMove = { .x = blockMove.x, .y = blockMove.y };
        return bestMove;
    }
    else if (findBlockingPositionForThreeInRow(tree, &winMove, playerDol)) {
        BestMove bestMove = { .x = winMove.x, .y = winMove.y };
        return bestMove;
    }
    else {
        // ���� ���� �湮�� ��� ����
        Node* mostVisitedNode = tree->children[0]; // �ʱ�ȭ
        for (int i = 1; i < tree->numChildren; i++) {
            if (tree->children[i]->visits > mostVisitedNode->visits) {
                mostVisitedNode = tree->children[i];
            }
        }

        // ���õ� ����� ��ǥ ��ȯ
        BestMove bestMove = { .x = mostVisitedNode->moveX, .y = mostVisitedNode->moveY };
        return bestMove;
    }
}

int main() {
    setlocale(LC_ALL, "");
    printf("\n               OMOK GAME!\n"); //title
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            currentState.board[i][j] = L'\u253C'; // '��'�� �����ڵ� �ڵ� ����Ʈ
        }
    }
    omokBoard();
    printf("------------------------------------------\n");

    printf("Set time limit for each move (E.g. 10 for 10 seconds):\n");
    int scanfResult = scanf("%d", &setTimeLimit);
    while (scanfResult != 1 || setTimeLimit < 1) {
        printf("Invalid time limit. Time limit should be greater than 1 second.\n");
        while (getchar() != '\n');
        printf("Set time limit:\n");
        scanfResult = scanf("%d", &setTimeLimit);
    }
    printf("Time limit set to: %d seconds\n", setTimeLimit);

    printf("Choose the color you want. Type black as B, white as W. Black player starts first:\n");
    scanf(" %c", &playerChoice); //�浹 �鵹 ���ϱ�
    while (playerChoice != 'B' && playerChoice != 'W') {
        printf("Invalid input. Choose between B and W.\n");
        printf("Choose the color you want. Type black as B, white as W. Black player starts first:\n");
        scanf(" %c", &playerChoice);
    }
    aiChoice = (playerChoice == 'B') ? 'W' : 'B';
    currentState.currentPlayer = (playerChoice == 'B') ? 1 : 2; //�÷��̾ �浹�̶�� currentPlayer = 1, ai�� �浹�̶�� currentPlayer = 2 
    gameMove(playerChoice, aiChoice, setTimeLimit);
    return 0;
}

void omokBoard() { // ������ �迭 ���� �� �ʱ�ȭ �� ���
    int i, j;
    printf("   ");
    for (i = 0; i < BOARD_SIZE; i++) {
        printf("%c ", 'a' + i);
    }
    printf("\n");
    for (i = 0; i < BOARD_SIZE; i++) {
        printf("%02d ", i + 1);
        for (j = 0; j < BOARD_SIZE; j++) {
            wprintf(L"%lc\u2500", currentState.board[i][j]);
        }
        printf("\n");
    }
}

bool isBoardEmpty(wchar_t board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] != L'\u253C') {
                return false; // ���忡 ���� �ϳ��� �ִ� ��� false ��ȯ
            }
        }
    }
    return true; // ��� ĭ�� ����ִ� ��� true ��ȯ
}

void gameMove(char playerChoice, char aiChoice, int setTimeLimit) {
    //printf("------------------------------------------\n");
    //currentState.currentPlayer = (playerChoice == 'B') ? 1 : 2;
    int keyin; //��ǥ �Է� ����
    int timeLimit = setTimeLimit * 1000;
    char x;
    int y;
    wchar_t playerDol = (playerChoice == 'B') ? L'��' : L'��';
    wchar_t aiDol = (playerChoice == 'B') ? L'��' : L'��';
    char winner = ' ';
    while (winner == ' ') {
        if (currentState.currentPlayer == 1) { //player����
            printf("------------------------------------------\n");
            printf("Player (%c)'s turn. Make your move. (E.g. j 10 for (j, 10)):\n", playerChoice);
            int selectedPosition = 0; //already selected position?
            int invalidPosition = 0;  //invalid position?
            while (1) {
                time_t startTime = clock();
                keyin = 1;
                do {
                    time_t currentTime = clock();
                    if (difftime(currentTime, startTime) > timeLimit) { //�ð��� �ʰ��Ѵٸ�
                        keyin = 0; //Ű �Է��� ����
                        break;
                    }
                } while (!kbhit()); //Ű�� �� ������
                if (keyin == 1) {
                    scanf(" %c %d", &x, &y); // ascii code�� x y �޾ƿ�
                    int numX = x - 'a';
                    y--;
                    if (numX >= 0 && numX < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
                        if (currentState.board[y][numX] == L'\u253C') {
                            currentState.board[y][numX] = playerDol;
                            omokBoard();
                            gameWin(currentState.board);
                            currentState.previousOpponentMoveX = numX;
                            currentState.previousOpponentMoveY = y;
                            break;
                        }
                        else {
                            printf("Already selected position. Select again.\n");
                            continue;
                        }
                    }
                    else {
                        printf("Invalid position. Select again.\n");
                        continue;
                    }
                }
                else {
                    int randX, randY;
                    do {
                        randX = rand() % BOARD_SIZE;
                        randY = rand() % BOARD_SIZE;
                    } while (currentState.board[randY][randX] != L'\u253C');
                    printf("Time limit exceeded. Position (%c, %d) is randomly selected.\n", 'a' + randX, randY + 1);
                    currentState.board[randY][randX] = playerDol;
                    omokBoard();
                    gameWin(currentState.board);
                    currentState.previousOpponentMoveX = randX;
                    currentState.previousOpponentMoveY = randY;
                    break;
                }
            }
            currentState.currentPlayer = 2;
        }
        else { //ai����
            printf("------------------------------------------\n");
            printf("AI (%c)'s turn.\n", aiChoice);
            srand(time(NULL));
            Node* root = createNode(currentState);
            if (isBoardEmpty(currentState.board)) { // ������ ó�� ���۵Ǿ����� Ȯ��
                int centerX = BOARD_SIZE / 2;
                int centerY = BOARD_SIZE / 2;
                currentState.board[centerY][centerX] = aiDol;
                printf("AI selected (%c, %d).\n", 'a' + centerX, centerY + 1);
                omokBoard();
                currentState.currentPlayer = 1;
            }
            else {
                BestMove bestMove = mctSearch(currentState, setTimeLimit, playerChoice, 1000);
                int x = bestMove.x;
                int y = bestMove.y;
                currentState.board[y][x] = aiDol;
                printf("AI selected (%c, %d).\n", 'a' + x, y + 1);
                omokBoard();
                gameWin(currentState.board);
                currentState.currentPlayer = 1;
            }
        }
        winner = gameWin(currentState.board);
    }
    if (winner != ' ') { // ���ڰ� ������ ��� ��� ���
        if (winner == playerChoice) {
            printf("\nGAME END. PLAYER (%c) won. CONGRATS!\n", winner);
        }
        else {
            printf("\nGAME END. AI (%c) won. Try once more!\n", winner);
        }
    }
}

char gameWin(wchar_t board[BOARD_SIZE][BOARD_SIZE]) {
    char winner = ' '; // ���ڸ� ������ ����
    for (int i = 0; i < BOARD_SIZE; i++) { // ����
        for (int j = 0; j < BOARD_SIZE - 4; j++) {
            if (board[i][j] == L'��' && board[i][j + 1] == L'��' && board[i][j + 2] == L'��' && board[i][j + 3] == L'��' && board[i][j + 4] == L'��') {
                winner = 'B';
            }
            if (board[i][j] == L'��' && board[i][j + 1] == L'��' && board[i][j + 2] == L'��' && board[i][j + 3] == L'��' && board[i][j + 4] == L'��') {
                winner = 'W';
            }
        }
    }
    for (int i = 0; i < BOARD_SIZE; i++) { // ����
        for (int j = 0; j < BOARD_SIZE - 4; j++) {
            if (board[i][j] == L'��' && board[i + 1][j] == L'��' && board[i + 2][j] == L'��' && board[i + 3][j] == L'��' && board[i + 4][j] == L'��') {
                winner = 'B';
            }
            if (board[i][j] == L'��' && board[i + 1][j] == L'��' && board[i + 2][j] == L'��' && board[i + 3][j] == L'��' && board[i + 4][j] == L'��') {
                winner = 'W';
            }
        }
    }
    for (int i = 0; i < BOARD_SIZE - 4; i++) { // ������
        for (int j = 0; j < BOARD_SIZE - 4; j++) {
            if (board[i][j] == L'��' && board[i + 1][j + 1] == L'��' && board[i + 2][j + 2] == L'��' && board[i + 3][j + 3] == L'��' && board[i + 4][j + 4] == L'��') {
                winner = 'B';
            }
            if (board[i][j] == L'��' && board[i + 1][j + 1] == L'��' && board[i + 2][j + 2] == L'��' && board[i + 3][j + 3] == L'��' && board[i + 4][j + 4] == L'��') {
                winner = 'W';
            }
        }
    }
    for (int i = 0; i < BOARD_SIZE - 4; i++) { // ������
        for (int j = 0; j < BOARD_SIZE - 4; j++) {
            if (board[i][j + 4] == L'��' && board[i + 1][j + 3] == L'��' && board[i + 2][j + 2] == L'��' && board[i + 3][j + 1] == L'��' && board[i + 4][j] == L'��') {
                winner = 'B';
            }
            if (board[i][j + 4] == L'��' && board[i + 1][j + 3] == L'��' && board[i + 2][j + 2] == L'��' && board[i + 3][j + 1] == L'��' && board[i + 4][j] == L'��') {
                winner = 'W';
            }
        }
    }
    return winner;
}
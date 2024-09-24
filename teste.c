#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <unistd.h> // Para usleep()
#include "vga.h"

#define WIDTH 10
#define HEIGHT 20
#define TETROMINOES_QUANT 13

// Matriz de tetrominos
int tetrominoes[TETROMINOES_QUANT][4][4] = {
    // falta colocar/mudar as peças 
    { 
        {1, 1},
        {0, 0}
    },{ 
        {1, 0},
        {1, 0}
    },{ 
        {0, 1},
        {1, 1}
    }, { 
        {1, 0},
        {1, 1}
    },{ 
        {1, 1},
        {1, 0}
    },{ 
        {1, 1},
        {0, 1}
    },{ 
        {1, 1},
        {1, 1}
    },
    { 
        {0, 0, 0},
        {1, 1, 1},
        {0, 0, 0}
    },{ 
        {0, 1, 0},
        {0, 1, 0},
        {0, 1, 0}
    },{
        {0, 1, 0},
        {1, 1, 1},
        {0, 0, 0}
    },{
        {0, 1, 0},
        {0, 1, 1},
        {0, 1, 0}
    },{
        {0, 1, 0},
        {1, 1, 0},
        {0, 1, 0}
    },{
        {0, 0, 0},
        {1, 1, 1},
        {0, 1, 0}
    }
};

// Campo de jogo
int board[HEIGHT][WIDTH] = {0};


int currentX, currentY; // Posição atual da peça
int currentPiece;   // Índice da peça atual
int nextPiece;      // Índice da próxima peça
bool isPaused = false;
int score = 0;

// Função para escolher um índice de peça aleatório
int getRandomPiece() {
    return rand() % TETROMINOES_QUANT; // Escolhe aleatoriamente um índice entre 0 e 6
}

// Inicializa a próxima peça
void initializeNextPiece() {
    nextPiece = getRandomPiece();
    printw("%d", nextPiece);
}

// Inicializa a peça atual
void initializeCurrentPiece() {
    currentPiece = nextPiece; // A peça atual é a próxima peça
    initializeNextPiece();    // Define a próxima peça para a próxima vez
}

/* Função para verificar colisão  
Rebebe: a posicao x e y da peca
*/
bool checkCollision(int x, int y) {
    int pieceHeight = (currentPiece == 1) ? 2 : 4; // Peça O tem altura 2
    for (int i = 0; i < pieceHeight; i++) {
        for (int j = 0; j < 4; j++) {
            if (tetrominoes[currentPiece][i][j] && 
                (x + j < 0 || x + j >= WIDTH || y + i >= HEIGHT || board[y + i][x + j])) {
                return true;
            }
        }
    }
    return false;
}

/* Função que remove as linhas que estão completas da matriz do jogo e move tudo que está em cima para baixo */
void removeFullLines() {
    for (int i = HEIGHT - 1; i >= 0; i--) {
        bool fullLine = true;
        for (int j = 0; j < WIDTH; j++) {
            if (board[i][j] == 0) {
                fullLine = false;
                break;
            }
        }
        if (fullLine) {
            // Remove a linha e atualiza a pontuação
            score += 100; // Adiciona 100 pontos por linha completa
            // Move todas as linhas acima uma linha para baixo
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < WIDTH; j++) {
                    board[k][j] = board[k - 1][j];
                }
            }
            // Limpa a linha superior
            for (int j = 0; j < WIDTH; j++) {
                board[0][j] = 0;
            }
            i++; // Verifica a linha novamente
        }
    }
}


/* Função para desenhar o campo de jogo (com a peça temporária)
*/
void drawBoard() {
    clear(); // Usa ncurses para limpar a tela
    // Desenha o tabuleiro fixo
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (board[i][j] == 0) {
                printw("."); // Ponto vazio
            } else {
                printw("#"); // Bloco preenchido
            }
        }
        printw("\n");
    }
    
    // Desenha a peça temporariamente (antes de colidir)
    int pieceHeight = (currentPiece == 1) ? 2 : 4;
    for (int i = 0; i < pieceHeight; i++) {
        for (int j = 0; j < 4; j++) {
            if (tetrominoes[currentPiece][i][j]) {
                mvprintw(currentY + i, currentX + j, "#");
            }
        }
    }

    // Desenha a próxima peça
    mvprintw(2, 11, "PROXIMA PECA:");
    int nextPieceHeight = sizeof(tetrominoes[nextPiece]) / sizeof(tetrominoes[nextPiece][0]);
    for (int i = 0; i < nextPieceHeight; i++) {
        for (int j = 0; j < 4; j++) {
            if (tetrominoes[nextPiece][i][j]) {
                mvprintw(3 + i, 12 + j, "@"); // Ajuste as coordenadas (30, 12) conforme necessário
            }
        }
    }

    mvprintw(0, 11, "PONTUACAO: %d", score);

    refresh(); // Atualiza a tela ncurses
}

// Função para adicionar a peça ao campo (ajustada para lidar com tamanhos de peças menores)
void placePiece() {
    int pieceHeight = (currentPiece == 1) ? 2 : 4; // Peça O tem altura 2
    for (int i = 0; i < pieceHeight; i++) {
        for (int j = 0; j < 4; j++) {
            if (tetrominoes[currentPiece][i][j]) {
                board[currentY + i][currentX + j] = 1;
            }
        }
    }
    removeFullLines();
}

/* Função para mover a peça 
Recebe: o valor da variação de x e/ou y para movimentar a peça
*/
void movePiece(int dx, int dy) {
    if (!checkCollision(currentX + dx, currentY + dy)) {
        currentX += dx;
        currentY += dy;
    } else if (dy != 0) {
        placePiece();
        initializeCurrentPiece();
        currentX = WIDTH / 2 - 2;
        currentY = 0;
    }
}

// Função para inicializar o jogo, definindo as posições das peças e o valor randomico delas
void initGame() {
    srand(time(NULL));
    initializeCurrentPiece();
    currentX = WIDTH / 2 - 2;
    currentY = 0;
}

// Função para verificar se o jogo acabou
bool isGameOver() {
    return checkCollision(currentX, currentY); // Se houver colisão no topo, o jogo acaba
}

// Função principal do jogo
void gameLoop() {
    int ch;
    initGame();
    
    // Controle de tempo para a animação
    int speed = 500; // Velocidade de queda (500 milissegundos)
    clock_t lastMoveTime = clock();
    
    nodelay(stdscr, TRUE); // Faz com que getch() não bloqueie o loop (para entradas com o teclado)
    while (1) {
        if (isGameOver()) { // Verifica se o jogo acabou
            clear();
            mvprintw(HEIGHT / 2, WIDTH / 2 - 5, "GAME OVER");
            refresh();
            usleep(3000000); // Pausa de 3 segundos para exibir o Game Over
            endwin(); // Fecha a janela ncurses
            exit(0); // Encerra o programa
        }

        if (!isPaused) {
            drawBoard(); // Desenha o tabuleiro enquanto não está pausado
            
            // Controle do tempo para a peça descer automaticamente
            clock_t currentTime = clock();
            if ((currentTime - lastMoveTime) * 100000 / CLOCKS_PER_SEC > speed) {
                movePiece(0, 1); // Move a peça para baixo automaticamente
                lastMoveTime = currentTime; // Atualiza o tempo da última queda
            }
        } else {
            mvprintw(HEIGHT / 2, WIDTH / 2 - 5, "PAUSED");
            refresh();
        }

        ch = getch(); // Captura a tecla pressionada (não bloqueia o programa)
        
        // Verifica se o usuário pressionou alguma tecla
        if (ch != ERR) {
            switch (ch) {
                case 'a': // Mover para a esquerda
                    if (!isPaused) movePiece(-1, 0);
                    break;
                case 'd': // Mover para a direita
                    if (!isPaused) movePiece(1, 0);
                    break;
                case 's': // Mover mais rápido para baixo
                    if (!isPaused) movePiece(0, 1);
                    break;
                case 'p': // Pausar ou retomar o jogo
                    isPaused = !isPaused; // Alterna o estado de pausa
                    break;
                case 'q': // Sair do jogo
                    endwin(); // Fecha a janela ncurses
                    exit(0);
            }
        }

        usleep(50000); // Pequeno atraso para evitar uso excessivo de CPU
    }
}

int main() {
    initscr(); // Inicializa a tela ncurses
    noecho(); // Não exibe as teclas digitadas
    cbreak(); // Desativa o buffer de linha
    keypad(stdscr, TRUE); // Habilita teclas especiais
    curs_set(0); // Oculta o cursor
    initializeNextPiece();
    gameLoop(); // Inicia o loop do jogo
    endwin(); // Finaliza a janela ncurses
    return 0;
}
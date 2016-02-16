#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ctime>


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <FTGL/ftgl.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

using namespace std;

struct VAO {
  GLuint VertexArrayID;
  GLuint VertexBuffer;
  GLuint ColorBuffer;
  GLuint TextureBuffer;
  GLuint TextureID;
  GLenum PrimitiveMode; // GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY
  GLenum FillMode; // GL_FILL, GL_LINE
  int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID; // For use with normal shader
  GLuint TexMatrixID; // For use with texture shader
} Matrices;

struct FTGLFont {
  FTFont* font;
  GLuint fontMatrixID;
  GLuint fontColorID;
} GL3Font;

GLuint programID, fontProgramID, textureProgramID;

GLuint textureID[300];

char buffer[100];

static const GLfloat texture_buffer_data [] = {
  0,1, // TexCoord 1 - bot left
  1,1, // TexCoord 2 - bot right
  1,0, // TexCoord 3 - top right

  1,0, // TexCoord 3 - top right
  0,0, // TexCoord 4 - top left
  0,1  // TexCoord 1 - bot left
};

VAO *cube, *player, *timer[16], *stars[3], *hearts[22], *background, *star, *heart, *menu, *banner, *head, *limbs, *cube2, *eyes;
VAO *sphere, *spikes, *coin, *water[25], *square[2], *tree, *speedy[2], *throne, *soldier[120], *dragon[12], *grass, *wood, *sigil[10];

float eyeHeight = 10, blockCoordY = -1, playerCoordY = 4.2, playerCoordZ = -10, playerCoordX = -8;
float playerRotation = 0, sphereRotation = 90;

float animateX = -10;
float animateY = 3;

bool starAnimate = false, heartAnimate = false, lightOn = false, onMenu = true;
int playerHouse = 0;
float speed = 1;
double xpos, ypos, prevXpos, prevYpos;
float zoom_flag = 0;

time_t magicStamp = 0;

float viewsX[5][10] = {
  {
    -18, -13, -8, -3, 16, 20, 5, -23
  },
  {
    -5
  },
  {
    -12
  },
  {
    -8
  }

};

float viewsY[5][10] = {
  {
    16, 16, 18, 14, 16, 16, 16, 18
  },
  {
    28
  },
  {
    15
  },
  {
    6.2
  }
};

float viewsZ[5][10] = {
  {
    10, 15, 20, 25, 20, -25, -30, -10
  },
  {
    0
  },
  {
    -14
  },
  {
    -10
  }
};

time_t currentTime, timeStamp, gameStart = 0, winTime = 0, loseTime = 0;

vector< pair<int, int> > obstacles;
vector< pair<int, int> > coins;
int level = 1;

int playerX = 0, playerZ = 0, shiftX = -8, shiftZ = -10, frames = 0, blockMotion = 0, lives = 3, points = 0;
int prevPlayerX = 0, prevPlayerZ = 0;
int hours, minutes, seconds;

bool playerJumpUp = false, playerJumpDown = false, playerJumpRight = false, playerJumpLeft = false;
bool playerMoveUp = false, playerMoveDown = false, playerMoveRight = false, playerMoveLeft = false;
bool playerWin = false, playerLose = false;
int playerDirection = 3;


bool playerFall = false, playerFallOff = false, playerAnimate = false;

int numViews = 4, currentView = 0, viewPtr[5];

int numSubViews[] = {8, 1, 1, 1};

bool isPresent[10][10] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 
  0, 1, 1, 1, 1, 1, 0, 1, 0, 1,
  1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 
  1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 
  0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 
  0, 1, 1, 0, 1, 1, 0, 1, 1, 1,
  1, 0, 1, 1, 1, 0, 1, 0, 1, 1,
  1, 1, 1, 0, 1, 1, 1, 1, 0, 1
};

bool isMoving[10][10] = {
  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 0, 0, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 0, 0, 0
};

void boardReset() {
   for(int i=0; i<10; i++) for(int j=0;j<10;j++) {
  int temp = rand() % (40/level);
  isPresent[i][j] = temp != 0;

  if(i==j || i+j==9) isPresent[i][j] = 1;
}
for(int i=0; i<10; i++) for(int j=0;j<10;j++) {
  int temp = rand() % (40/level);

  isMoving[i][j] = temp == 0;
  if(i==j || i+j==9 || !isPresent[i][j]) isMoving[i][j] = 0;
}

}

void gameReset() {
  level++;
  points+=100;

  boardReset();
 
  if(playerLose) {
      lives = 3;
  points = 0;

  }
  playerLose = false;
  loseTime = 0;

  gameStart = glfwGetTime();
  playerWin = false;
  playerFall = false, playerFallOff = false; 
  playerX = 0;
  playerZ = 0;
  playerCoordY = 4.2;
  playerCoordX = shiftX;
  playerCoordZ = shiftZ;
  playerJumpRight = playerJumpUp = playerJumpLeft = playerJumpDown = false;
  playerDirection = 3;
  playerMoveDown=  playerMoveLeft = playerMoveRight = playerMoveUp = false;

}

void magicLife() {
  if(!(playerX==playerZ or playerZ+playerX == 9)) return;
  int temp = rand() % 500;
  if(temp == 0) lives++;


}

void updatePos() {
   if(glfwGetTime() - magicStamp >) {
          magicLife();
          magicStamp = glfwGetTime();
        }
  for(int i = 0; i < 10; i++) {
    for(int j = 0; j< 10; j++) {
      float a = i*2+shiftX - 1.0;
      float b = i*2+shiftX + 1.0;
      float c = j*2+shiftZ - 1.0;
      float d = j*2+shiftZ + 1.0;
      if(playerCoordX > a && playerCoordX < b && playerCoordZ > c && playerCoordZ < d) {
        playerX = i;
        playerZ = j;


        break;
      }
    }
  }
  if(playerX == 9 && playerZ == 9) {
    playerWin = true;


    if(winTime == 0) winTime = glfwGetTime();
    if(glfwGetTime() - winTime > 5) {
      gameReset();
      winTime = 0;
    }
  }
  else if(playerCoordX < shiftX - 1) playerFallOff = true, playerX = -1;
  else if(playerCoordX >= 9*2+shiftX + 2) playerFallOff = true, playerX = 10;
  else if(playerCoordZ < shiftZ - 2) playerFallOff = true, playerZ = -1;
  else if(playerCoordZ >= 9*2+shiftZ +2) playerFallOff = true, playerZ = 10;
}


void playerReset(int f = 1) {
  if(!playerFall && !playerAnimate && f!=2) {
    timeStamp = glfwGetTime();
    playerAnimate = true;
  }
  if(playerAnimate) return;


  if(!playerLose) lives--;


  heartAnimate = true;
  if(lives==0) {
    level = 1;
  boardReset();

    playerLose = true;
    if(loseTime == 0) loseTime = glfwGetTime();
    return;
    playerLose = false, loseTime = 0;
    lives = 3;
    points = 0;
    gameStart = glfwGetTime();

  }

  playerFall = false, playerFallOff = false; 
  playerX = 0;
  playerZ = 0;
  playerCoordY = 4.2;
  playerCoordX = shiftX;
  playerCoordZ = shiftZ;
  playerJumpRight = playerJumpUp = playerJumpLeft = playerJumpDown = false;
  playerDirection = 3;
}


void checkCollision() {
  if(find(obstacles.begin(), obstacles.end(), make_pair(playerX, playerZ)) != obstacles.end()) playerReset();
  if(find(coins.begin(), coins.end(), make_pair(playerX, playerZ)) != coins.end()) {
    points += 20;
    starAnimate = true;
    coins.erase(std::remove(coins.begin(), coins.end(), make_pair(playerX, playerZ)), coins.end());
  }
  if(isPresent[playerX][playerZ] == 0 && !playerJumpUp && !playerJumpDown && !playerJumpRight && !playerJumpLeft) playerFall = true;
  if(isMoving[playerX][playerZ] && playerCoordY - 1 <= blockCoordY + 3) playerReset();
  else if(isMoving[playerX][playerZ] && !playerJumpRight && !playerJumpUp && ! playerJumpLeft && !playerJumpDown) playerReset(); 
}

void genObstacles() {
  obstacles.clear();
  int r, c;
  for(int i=0; i<15; i++) {
    r = rand()%10;
    c = rand()%10;
    if(!(r==9&&c==9) && !(r==0&&c==0) && isPresent[r][c] && !isMoving[r][c] && !(playerX == r && playerZ == c) && find(coins.begin(), coins.end(), make_pair(r, c)) == coins.end())
      obstacles.push_back(make_pair(r,c));
  }
}

void genCoins() {
  coins.clear();
  int r, c;
  for(int i=0; i<20; i++) {
    r = rand()%10;
    c = rand()%10;
    if(isPresent[r][c] && !isMoving[r][c] && !(playerX == r && playerZ == c) && find(obstacles.begin(), obstacles.end(), make_pair(r, c)) == obstacles.end())
      coins.push_back(make_pair(r,c));
  }

}


void drawFall() {
  if(playerFallOff && playerX == -1) playerCoordX -=0.05;
  if(playerFallOff && playerZ == 10) playerCoordZ +=0.05;
  if(playerFallOff && playerZ == -1) playerCoordZ -=0.05;
  if(playerFallOff && playerX == 10) playerCoordX +=0.05;


  if(playerFall) {
    playerCoordY -= 0.1;
  }
  if(playerCoordY <= -10) {
    playerReset();
  }
}

void drawJump() {
  if(playerAnimate) return;
  float  fixed = 0.05;
  if(playerCoordX >= 2*prevPlayerX+shiftX+2 and fixed>0) fixed*=-1;
  if(playerCoordZ >= prevPlayerZ*2+2+shiftZ and fixed>0) fixed*=-1;
  if(playerCoordX <= 2*prevPlayerX+shiftX-2 and fixed>0) fixed*=-1;
  if(playerCoordZ <= 2*prevPlayerZ+shiftZ-2 and fixed>0) fixed*=-1;
  if(playerJumpUp) { 
    playerCoordX += 0.05;
    playerCoordY += fixed;
    if(playerCoordY<4.2 && !playerFall) playerCoordY = 4.2;
  }
  else if(playerJumpDown) {
    playerCoordX -= 0.05;
    playerCoordY += fixed;
    if(playerCoordY<4.2 && !playerFall) playerCoordY = 4.2;


  }
  else if(playerJumpRight) {
    playerCoordZ += 0.05;
    playerCoordY += fixed;
    if(playerCoordY<4.2 && !playerFall) playerCoordY = 4.2;

  }
  else if(playerJumpLeft) {
    playerCoordZ -= 0.05;
    playerCoordY += fixed;
    if(playerCoordY<4.2 && !playerFall) playerCoordY = 4.2;

  }
  updatePos();
  if(playerCoordX >= 2*prevPlayerX+shiftX+4 && playerJumpUp) playerCoordX = 2*playerX+shiftX, playerCoordY = 4.2, playerJumpUp = false;
  if(playerCoordZ >= prevPlayerZ*2+shiftZ+4 && playerJumpRight) playerCoordZ = 2*playerZ+shiftZ, playerCoordY = 4.2, playerJumpRight = false;
  if(playerCoordX <= 2*prevPlayerX+shiftX-4 && playerJumpDown) playerCoordX = 2*playerX+shiftX, playerCoordY = 4.2, playerJumpDown = false;
  if(playerCoordZ <= 2*prevPlayerZ+shiftZ-4 && playerJumpLeft) playerCoordZ = 2*playerZ+shiftZ, playerCoordY = 4.2, playerJumpLeft = false;
  checkCollision();
}

void updateObstacles() {
  if(frames == 200) frames = 0;
  if(frames == 0) {
    genObstacles();
    genCoins();
  }
  frames++;
}

void updateBlockMotion() {
  blockMotion++;
  if(blockMotion < 300 && blockMotion >= 0) {
    blockCoordY += 0.01;
  }
  else if(blockMotion==300) {
    blockMotion = -301;
  }
  else if(blockMotion<=0) {
    blockCoordY -= 0.01;
  }
}



void drawMove() {
  if(playerAnimate) return;
  if(!playerMoveUp and !playerMoveLeft and !playerMoveRight and !playerMoveDown) return;
  if(playerMoveUp and !playerFall) playerCoordX += 0.1*speed;
  else if(playerMoveDown and !playerFall) playerCoordX -= 0.1*speed;
  else if(playerMoveRight and !playerFall) playerCoordZ += 0.1*speed;
  else if(playerMoveLeft and !playerFall) playerCoordZ -= 0.1*speed;
  for(int i = 0; i < 10; i++) {
    for(int j = 0; j< 10; j++) {
      float a = i*2+shiftX - 1.0;
      float b = i*2+shiftX + 1.0;
      float c = j*2+shiftZ - 1.0;
      float d = j*2+shiftZ + 1.0;
      if(playerCoordX > a && playerCoordX < b && playerCoordZ > c && playerCoordZ < d) {
        playerX = i;
        playerZ = j;
        break;
      }
    }
  }
  updatePos();
  if(!isPresent[playerX][playerZ] || playerX > 9 || playerX < 0 || playerZ > 9 || playerZ < 0) {
    playerFall = true;
    if(!isPresent[playerX][playerZ]) {
      playerCoordX = playerX*2+shiftX;
      playerCoordZ = playerZ*2+shiftZ;
    }
  }
  checkCollision();
}



// Program Name: ARBIPLAT
// Author(s): SomeCoolGuy
// Description: A Platformer
/* Keep these headers */
#include <tice.h>

/* Standard headers - it's recommended to leave them included */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>
#include <keypadc.h>

#define copysign(x, y) x * (fabs(y)/y)
//#include "obj.h"


/*Player Stuff*/
typedef enum colors {
    black = 0x00,
    white = 0xFF,
    red = 0xE0,
    green = 0x07,
    blue = 0x1C,
};
typedef struct {
    uint8_t color;
    int x;
    int y;
    int8_t xvel;
    int8_t yvel;
    bool falling;
    int width;
    int height;
    int spawn[2];
} newPlayer;
/*Camera is currently set up, all that is
 needed is to implement it.*/
typedef struct {
    int x;
    int y;
    int xllim; //lower limit (0)
    int yllim; //lower limit (0)
    int xulim; //upper limit (LCD_WIDTH)
    int yulim; //upper limit (LCD_HEIGHT)
    int (*pFunc) (newPlayer n,int x,int y);
    int (*bFunc) (int **blocks,int x,int y,unsigned int length);
} camera;
/*void initCamera(camera * c,int xy[2],int lims[2][2]){
 c->x = xy[0];
 c->y = xy[1];
 c->xllim = lims[0][0];
 c->xulim = lims[0][1];
 c->yllim = lims[1][0];
 c->yulim = lims[1][1];
 }*/
void change(newPlayer *n,char str[7],int value) {
    if (strncmp(str,"color",5)==0){
        n->color = value;
    } else if (strncmp(str,"x",1)==0){
        n->x = value;
    } else if (strncmp(str,"y",1)==0){
        n->y = value;
    } else if (strncmp(str,"xvel",4)==0){
        n->xvel = value;
    } else if (strncmp(str,"yvel",4)==0){
        n->yvel = value;
    } else if (strncmp(str,"width",5)==0){
        n->width = value;
    } else if (strncmp(str,"height",6)==0){
        n->height = value;
    }
}
void drawPlayer(newPlayer n){
    gfx_Rectangle(n.x,n.y,n.width,n.height);
}
void drawBlock(int block[4]){
    gfx_Rectangle(block[0],block[1],block[2],block[3]);
}
//Usage: drawBlocks(levels[curLevel],nBlocks[curLevel]);
void drawBlocks(int **blocks,unsigned int length){
    unsigned int i;
    for (i = 0;i < length;i++){
        drawBlock(blocks[i]);
    }
}
int * getRect_player(newPlayer n){
    int dim[4];
    dim[0] = n.x;
    dim[1] = n.y;
    dim[2] = n.width;
    dim[3] = n.height;
    return dim;
}

int * playerCAdjust(newPlayer n,int x,int y){
    int *dim;
    dim = getRect_player(n);
    dim[0] -= x;
    dim[1] -= y;
    return dim;
}

int * blockCAdjust(int block[4],int x,int y){
    int dim[4];
    dim[0] = block[0] - x;
    dim[1] = block[1] - y;
    dim[2] = block[2];
    dim[3] = block[3];
    return dim;
}
//Usage: blockCAdjust(levels[curLevel],camera.x,camera.y,nLevels[curLevel]);
//blocksCAdjust(int blocks[][4],int x,int y,unsigned int length)
int ** blocksCAdjust(int **blocks,int x,int y,unsigned int length){
    //int level[length][4];
    int **level;
    unsigned int i;
    for (i = 0;i<length;i++){
        level[i] = blockCAdjust(blocks[i],x,y);
    }
    return level;
}
//bool collision(newPlayer n, int block[4]){
bool collision(newPlayer n, int block[4]){
    /*return n.x < block[0] + block[2] &&
    n.x + n.width > block[2] &&
    n.y < block[1] + block[3] &&
    n.y + n.height > block[1];*/
    return gfx_CheckRectangleHotspot(n.x,n.y,n.width,n.height,block[0],block[1],block[2],block[3]);
}
//Usage: levelCollision(player,levels[curLevel],nBlocks[curLevel]);
//levelCollision(newPlayer n, int *blocks,unsigned int length)
bool levelCollision(newPlayer n, int **blocks ,unsigned int length){
    unsigned int i;
    for (i = 0;i < length;i++){
        if (collision(n,blocks[i])) {
            return true;
        }
    }
    return false;
}
//Usage: levelCollisionOffset(player,levels[curLevel],nBlocks[curLevel],x,y);
bool levelCollisionOffset(newPlayer n, int **blocks, unsigned int length, int8_t x, int8_t y){
    bool ret;
    newPlayer temp = n;
    temp.x += x;
    temp.y -= y;
    ret = levelCollision(temp,blocks,length);
    return ret;
}

void respawn(newPlayer *n){
    n->x = n->spawn[0];
    n->y = n->spawn[1];
}
void initLevel(uint8_t index) {
    gfx_FillScreen(index);
}
/*void nextLevel(void) {
    curLevel += 1;
    sprintf(levelstr,"Level %d",curLevel);
}*/
/* Other available headers */
// including stdarg.h, setjmp.h, assert.h, ctype.h, float.h, iso646.h, limits.h, errno.h

/* Main program */
void main(void) {
    newPlayer player;
    camera Camera;
    kb_key_t key;
    char levelstr[10];
    char posstr[10];
    int i, j;
    unsigned int curLevel;
    uint8_t nLevels = 1;
    unsigned int nBlocks[1] = {5};
    int levels[1][5][4] = {
        {
            {0.0,224.0,320.0,16.0},
            {0.0,80.0,48.0,80.0},
            {80.0,144.0,160.0,16.0},
            {240.8,0.0,32.0,160.0},
            {160.0,118.4,45.6,8.0},
        },
    };
    // {lower x,higher x,lower y,higher y}
    int levelLimits[1][2][2] = {
        {
            {0,320},{0,240}
        }
    };
    int levelWin[1][4] = {
        {304.0,0.0,16.0,224.0},
    };
    int pSpawn[1][2] = {
        {0,0}
    };
    /*First element is the background color.
     *everything else is the block's colors.
     *Levels can have an arbitrary number of blocks,
     *So this might change.
     */
    uint8_t levelColors[1][6] = {
        {0xFF,0x00,0x00,0x00,0x00,0x00}
    };
    
    bool jumppressed = false;
    bool gameRunning = true;
    sprintf(levelstr,"Level %d",curLevel);
    player.x = 0;
    player.y = 0;
    player.xvel = 0;
    player.yvel = 0;
    player.width = 20;
    player.height = 20;
    player.spawn[0] = 0;
    player.spawn[1] = 0;
    /*newPlayer player = {
        0x07, // color
        0, 0, // x, y
        0, 0, // x velocity, y velocity
        20, 20, // width, height
        { 0, 0 }
    };*/
    
    Camera.xllim = 0;
    Camera.xulim = 0;
    Camera.yllim = 0;
    Camera.yulim = 0;
    //Camera.pFunc = playerCAdjust;
    //Camera.bFunc = blocksCAdjust;
    
    
    gfx_Begin();
    gfx_SetTextScale(2,2);
    gfx_SetDrawBuffer();
    
    initLevel(*levelColors[curLevel]);
    while (gameRunning){
        gfx_SetColor(*levelColors[curLevel]);
        gfx_FillRectangle(player.x,player.y,player.width,player.height);
        player.falling = !levelCollisionOffset(player,levels[curLevel],nBlocks[curLevel],0,1);
        if (!levelCollisionOffset(player,levels[curLevel],nBlocks[curLevel],0,player.yvel)){
            player.y += player.yvel;
            player.yvel +=1;
        } else {
            i = 0;
            while (levelCollisionOffset(player,levels[curLevel],nBlocks[curLevel],0,i))
                i += (int) copysign(1,player.yvel);
            player.y -= i;
            player.yvel = 0;
        }
        kb_Scan();
        key = kb_Data[7];
        if (!(key & kb_Up))
            jumppressed = false;
        if (key & kb_Left) {
            i=0;
            while (!(levelCollisionOffset(player,levels[curLevel],nBlocks[curLevel],-1,0) || i >= 4)) {
                if (player.x > levelLimits[curLevel][0][0])
                    player.x -= 1;
                i+=1;
            }
        }
        if (key & kb_Right) {
            i=0;
            while (!(levelCollisionOffset(player,levels[curLevel],nBlocks[curLevel],1,0) || i >= 4)) {
                if (player.x > levelLimits[curLevel][0][1])
                    player.x += 1;
                i+=1;
            }
        }
        if ((key & kb_Up)&!jumppressed) {
            if (!(player.falling&!(player.yvel))){
                for(i=-12;i<0;i++) {
                    if (!levelCollisionOffset(player,levels[curLevel],nBlocks[curLevel],0,i)){
                        player.yvel = i;
                        jumppressed = true;
                        break;
                    }
                }
            }
        }
        gfx_SetColor(*levelColors[curLevel]);
        gfx_FillRectangle(0,0,strlen(levelstr)*16,16);
        
        gfx_SetColor(0x00);
        gfx_SetTextScale(2,2);
        gfx_PrintStringXY(levelstr,0,0);
        
        for (i = 0;i < nBlocks[curLevel];i++){
            gfx_SetColor(levelColors[curLevel][i+1]);
            gfx_FillRectangle(levels[curLevel][i][0],levels[curLevel][i][1],levels[curLevel][i][2],levels[curLevel][i][3]);
        }
        gfx_SetColor(player.color);
        gfx_FillRectangle(player.x,player.y,player.width,player.height);
        if (collision(player,levelWin[curLevel])){
            curLevel += 1;
            if (curLevel + 1 > nLevels) {
                gameRunning = false;
                gfx_SetColor(0x00);
                gfx_SetTextScale(3,3);
                gfx_PrintStringXY("YOU WIN!",strlen("YOU WIN!")*24-(LCD_WIDTH/2),96);
            } else {
                curLevel += 1;
                sprintf(levelstr,"Level %d",curLevel);
                initLevel(*levelColors[curLevel]);
                player.spawn[0] = pSpawn[curLevel][0];
                player.spawn[1] = pSpawn[curLevel][1];
                respawn(player);
            }
        }
        gfx_BlitBuffer();
    }
    while (!kb_AnyKey()) kb_Scan();
    //free(levels);levels=NULL;
    gfx_End();
    
}

/* Other functions */

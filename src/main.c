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

//#include "Rect.h"
#ifndef LCD_WIDTH
#ifndef LCD_HEIGHT
#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#endif
#endif
/* Other available headers */
// including stdarg.h, setjmp.h, assert.h, ctype.h, float.h, iso646.h, limits.h, errno.h

/* Put your function prototypes here */
/*int16_t playerCAdjust(newPlayer n);
int16_t blockCAdjust(int16_t block[4]);
bool collision(newPlayer n,int16_t block[4]);
bool levelCollision(newPlayer n,int16_t * level);*/
/* Put all your globals here. */

/*In Python, a level-set would look like this:
 [
    [ #Level 1
        Platform(0,280,400,20), #one Platform is a 4-element list (Rect).
        Platform(0,100,60,100),
        Platform(100,180,200,20),
        Platform(301,0,40,200),
        Platform(200,148,57,10),
    ]
    [ #Level 2
        Platform(0,300,400,0),
        Platform(300,240,30,60),
    ]
    #More levels...
 ]
 But because TI-84+ CE programs don't include Pygame,
 We have to use 4-element arrays to be able to replicate Rects in Pygame.
 Unfortunately, this is complicated by the fact that arrays are technically
 pointers to data, and that expanding the array means having to
 allocate, organize, and manage the memory for these arrays dynamically.
 */

/*for (char i = 0;i < sizeof(level)/sizeof(level[0]);i++){
    // allocate space for each Rect (4-element array) item
    level[i] = (int16_t[4] *) malloc(10 * sizeof(int16_t[4]));
}
level[i][0] = {0,220,320,20};*/
/* Main program */
void main(void) {
    uint8_t nLevels = 1;
    uint8_t curLevel = 0;
    uint16_t nBlocks[1] = {5};
    int16_t levels[1][5][4] = {
        {
            {0.0,224.0,320.0,16.0},
            {0.0,80.0,48.0,80.0},
            {80.0,144.0,160.0,16.0},
            {240.8,0.0,32.0,160.0},
            {160.0,118.4,45.6,8.0},
        },
    };
    // {lower x,higher x,lower y,higher y}
    int16_t levelLimits[1][2][2] = {
        {
            {0,320},{0,240}
        }
    };
    int16_t levelWin[1][4] = {
        {304.0,0.0,16.0,224.0},
    };
    int16_t pSpawn[1][2] = {
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
        int16_t x;
        int16_t y;
        int8_t xvel;
        int8_t yvel;
        bool falling;
        int16_t width;
        int16_t height;
        int16_t spawn[2];
    } newPlayer;
    /*Camera is currently set up, all that is
     needed is to implement it.*/
    typedef struct {
        int16_t x;
        int16_t y;
        int16_t xllim; //lower limit (0)
        int16_t yllim; //lower limit (0)
        int16_t xulim; //upper limit (LCD_WIDTH)
        int16_t yulim; //upper limit (LCD_HEIGHT)
        int16_t (*pFunc) (newPlayer n,int16_t x,int16_t y);
        int16_t (*bFunc) (int16_t **blocks,int16_t x,int16_t y,uint16_t length);
    } camera;
    /*void initCamera(camera * c,int16_t xy[2],int16_t lims[2][2]){
     c->x = xy[0];
     c->y = xy[1];
     c->xllim = lims[0][0];
     c->xulim = lims[0][1];
     c->yllim = lims[1][0];
     c->yulim = lims[1][1];
     }*/
    void change(newPlayer *n,char str[7],int16_t value) {
        if (str == "color"){
            n->color = value;
        } else if (str == "x"){
            n->x = value;
        } else if (str == "y"){
            n->y = value;
        } else if (str == "xvel"){
            n->xvel = value;
        } else if (str == "yvel"){
            n->yvel = value;
        } else if (str == "width"){
            n->width = value;
        } else if (str == "height"){
            n->height = value;
        }
    }
    void drawPlayer(newPlayer n){
        gfx_Rectangle(n.x,n.y,n.width,n.height);
    }
    void drawBlock(int16_t block[4]){
        gfx_Rectangle(block[0],block[1],block[2],block[3]);
    }
    //Usage: drawBlocks(levels[curLevel],nBlocks[curLevel]);
    void drawBlocks(int16_t **blocks,uint16_t length){
        uint16_t i;
        for (i = 0;i < length;i++){
            drawBlock(blocks[i]);
        }
    }
    int16_t * getRect_player(newPlayer n){
        int16_t dim[4];
        dim[0] = n.x;
        dim[1] = n.y;
        dim[2] = n.width;
        dim[3] = n.height;
        return dim;
    }
    
    int16_t * playerCAdjust(newPlayer n,int16_t x,int16_t y){
        int16_t *dim;
        dim = getRect_player(n);
        dim[0] -= x;
        dim[1] -= y;
        return dim;
    }
    
    int16_t * blockCAdjust(int16_t block[4],int16_t x,int16_t y){
        int16_t dim[4];
        dim[0] = block[0] - x;
        dim[1] = block[1] - y;
        dim[2] = block[2];
        dim[3] = block[3];
        return dim;
    }
    //Usage: blockCAdjust(levels[curLevel],camera.x,camera.y,nLevels[curLevel]);
    //blocksCAdjust(int16_t blocks[][4],int16_t x,int16_t y,uint16_t length)
    int16_t ** blocksCAdjust(int16_t **blocks,int16_t x,int16_t y,uint16_t length){
        //int16_t level[length][4];
        int16_t **level;
        uint16_t i;
        for (i = 0;i<length;i++){
            level[i] = blockCAdjust(blocks[i],x,y);
        }
        return level;
    }
    bool collision(newPlayer n, int16_t block[4]){
        return n.x < block[0] + block[2] &&
        n.x + n.width > block[2] &&
        n.y < block[1] + block[3] &&
        n.y + n.height > block[1];
    }
    //Usage: levelCollision(player,levels[curLevel],nBlocks[curLevel]);
    //levelCollision(newPlayer n, int16_t *blocks,uint16_t length)
    bool levelCollision(newPlayer n, int16_t **blocks,uint16_t length){
        uint16_t i;
        for (i = 0;i < length;i++){
            if (collision(n,blocks[i])) {
                return true;
            }
        }
        return false;
    }
    //Usage: levelCollisionOffset(player,levels[curLevel],nBlocks[curLevel],x,y)
    bool levelCollisionOffset(newPlayer n, int16_t ** blocks, uint16_t length, int8_t x, int8_t y){
        bool ret;
        newPlayer temp = n;
        temp.x += x;
        temp.y -= y;
        ret = levelCollision(temp,blocks,length);
        return ret;
    }
    char levelstr[10];
    char posstr[10];
    int16_t i;
    int16_t j;
    bool jumppressed = false;
    sprintf(levelstr,"Level %d",curLevel);
    colors my_color;
    my_color = green;
    newPlayer player = {
        my_color,
        0, 0, // x, y
        0, 0, // x velocity, y velocity
        20, 20, // width, height
        .spawn = { 0, 0 }
    };
    void respawn(void){
        player.x = player.spawn[0];
        player.y = player.spawn[1];
    }
    void initLevel(void) {
        gfx_FillScreen(*levelColors[curLevel]);
        player.spawn = pSpawn[curLevel];
    }
    void nextLevel(void) {
        curLevel += 1;
        sprintf(levelstr,"Level %d",curLevel);
    }
    camera Camera;
    Camera.xllim = 0;
    Camera.xulim = 0;
    Camera.yllim = 0;
    Camera.yulim = 0;
    Camera.pFunc = &playerCAdjust;
    Camera.bFunc = &blocksCAdjust;
    
    kb_key_t key;
    gameRunning = true;
    
    gfx_SetTextScale(2,2);
    gfx_Begin();
    gfx_SetDrawBuffer();
    initLevel();
    while (gameRunning){
        gfx_SetColor(*levelColors[curLevel]);
        gfx_FillRectangle(player.x,player.y,player.width,player.height);
        player.falling = !levelCollisionOffset(player,levels[curLevel],nBlocks[curLevel],0,1);
        if !levelCollisionOffset(player,levels[curLevel],0,player.yvel){
            player.y += player.yvel;
            yvel +=1;
        } else {
            i = 0;
            while (levelCollisionOffset(player,levels[curLevel],nBlocks[curLevel],0,i))
                i += (int16_t) copysign(1,player.yvel);
            player.y -= i;
            player.yvel = 0;
        }
        kb_Scan();
        key = kb_Data[7];
        if (!(key & kb_Up))
            jumppressed = false;
        if (key & kb_Left) {
            i=0;
            while (!(levelCollisionOffset(player,levels[curLevel],-1,0) || i >= 4)) {
                if (player.x > levelLimit[curLevel][0][0])
                    player.x -= 1;
                i+=1;
            }
        }
        if (key & kb_Right) {
            i=0;
            while (!(levelCollisionOffset(player,levels[curLevel],1,0) || i >= 4)) {
                if (player.x > levelLimit[curLevel][0][1])
                    player.x += 1;
                i+=1;
            }
        }
        if ((key & kb_Up)&!jumppressed) {
            if (!(player.falling&!(player.yvel))){
                for(i=-12;i<0;i++) {
                    if (!levelCollisionOffset(player,levels[curLevel],0,i)){
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
        for (i=0;i<nBlocks[curLevel],i++){
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
                gfx_PrintStringXY("YOU WIN!",strlen("YOU WIN!")*24-(LCD_WIDTH/2));
            } else {
                nextLevel();
                initLevel();
                respawn();
            }
        }
        gfx_BlitBuffer();
    }
    i=1;
    while (!kb_AnyKey());kb_Scan();
    free(levels);levels=NULL;
    gfx_End();
    
}

/* Other functions */

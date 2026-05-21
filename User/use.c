#include "use.h"
void user_init(){
    delay_init(168);      
    uart_init(115200); 
    LCD_Init();    

}
void print_str(int x,int y,char *p){
    POINT_COLOR = BLUE;   
    LCD_ShowString(x*16, y*16, 210, 16, 16, p);
}
void print_int(int x,int y,int value){
    POINT_COLOR = BLUE;   
    LCD_ShowNum(x*16==0?1:x*16, y*16==0?1:y*16, value, 6, 16);
}
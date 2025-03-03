#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
enum Suit {
    HEARTS,
    DIAMONDS,
    SPADES,
    CLUBS
};

Color suit_colors[] = {RED, RED, BLACK, BLACK};
char suit_chars[] = {'S', 'H', 'D', 'C'}; 
enum Rank {
    ACE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING
};
char rank_chars[] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K'};


typedef struct Card{
    float x;
    float y;
    float width;
    float height;
    enum Suit suit;
    enum Rank rank;
    bool isFlipped;
    float selectionOffset;
} Card;


bool mouseOnCard(Card card){
    return (GetMouseX() >= card.x &&
            GetMouseX() <= card.x + card.width &&
            GetMouseY() >= card.y &&
            GetMouseY() <= card.y + card.height);
}

void renderCards(Card* cards, int n){
    int border_weight = 3;
    Color colors[5] = {BLUE, PURPLE, RED, ORANGE, GREEN};
    for(int i=n; i>=0; i--){
        if(cards[i].isFlipped){
            DrawRectangle(cards[i].x - border_weight, cards[i].y - border_weight, cards[i].width + border_weight*2, cards[i].height + border_weight*2, suit_colors[cards[i].suit]);
            DrawRectangle(cards[i].x, cards[i].y, cards[i].width, cards[i].height, WHITE);
            const char text[] = {suit_chars[cards[i].suit],rank_chars[cards[i].rank], '\0'};
            DrawText(text, cards[i].x + cards[i].width/2, cards[i].y + cards[i].height/2, 20, BLACK);
        }
        else{
            DrawRectangle(cards[i].x - border_weight, cards[i].y - border_weight, cards[i].width + border_weight*2, cards[i].height + border_weight*2, PURPLE);
            DrawRectangle(cards[i].x, cards[i].y, cards[i].width, cards[i].height, BLUE);
        }
        
    }
}

void renderCardsFromTexture(Card* cards, int n, Texture2D cards_texture){
    for(int i=n; i>=0; i--){

        if(cards[i].isFlipped){
            Rectangle source = {cards[i].rank*cards[i].width, cards[i].suit*cards[i].height, cards[i].width, cards[i].height};
            Rectangle destination = {cards[i].x, cards[i].y, cards[i].width, cards[i].height};
            Vector2 origin = {0, 0};
            DrawTexturePro(cards_texture, source, destination, origin, 0, WHITE);
        }
        else{
            Rectangle source = {0, cards[i].height*4, cards[i].width, cards[i].height};
            Rectangle destination = {cards[i].x, cards[i].y, cards[i].width, cards[i].height};
            Vector2 origin = {0, 0};
            DrawTexturePro(cards_texture, source, destination, origin, 0, WHITE);
        }
        
    }
}

int getTopCardCollidingWithMouse(Card* cards, int n){
    for(int i=0; i<n; i++){
        if(mouseOnCard(cards[i])){ return i; }
    }
    return -1;
}

int getTopCardCollidingWithMouseSkipHeldCard(Card* cards, int n){
    for(int i=1; i<n; i++){
        if(mouseOnCard(cards[i])){ return i; }
    }
    return -1;
}

void moveClickedCardToTop(Card* cards, int n, int clicked_card_index){
    Card clicked_card = cards[clicked_card_index];

    for(int i=clicked_card_index-1; i>=0; i--){
        cards[i+1] = cards[i]; // move everything over 1
    }
    cards[0] = clicked_card;

}

void moveCardToTopOfCardCursorIsOn(Card* cards, int n){
    int card_below_cursor_index = getTopCardCollidingWithMouseSkipHeldCard(cards, n);
    
    // [top], [second_top], [3rd top], 3rd bot, 2nd bot, bot, new bot
    Card first_card = cards[0];
    if(card_below_cursor_index == -1){
        for(int i=0; i<n-1; i++){
            cards[i] = cards[i+1];
        }
        cards[n-1] = first_card;
    }
    else{
        for(int i=0; i<card_below_cursor_index; i++){
            cards[i] = cards[i+1];
        }       
        cards[card_below_cursor_index-1] = first_card;
    }   
    
    
}

void attractAllCardsToCursor(Card* cards, int n, int drag_speed){
    for(int i=0; i<n; i++){
        cards[i].x += ((GetMouseX() - cards[i].width/2) - cards[i].x)*drag_speed*GetFrameTime();
        cards[i].y += ((GetMouseY() - cards[i].height/2) - cards[i].y)*drag_speed*GetFrameTime();
    }
}


/*
    o select tool: grouping, ungrouping
    o move selected cards, make selected cards a "stack"
    o cards snapping onto a stack of cards
    o OR number showing number of cards close to top of stack
    
    
    select tool
    ~~~~~~~~~~~
    - need 2 corners, top left
    - first click MOUSE_BUTTON_LEFT with ctrl pressed
    - set first corner there
    - when MOUSE_BUTTON_LEFT 
    released, set second corner
    - until mouse is released, draw an outline rectangle of the area being selected
    - once mouse released and second corner is set, find all cards in selection ()
    - if any part of card overlaps with selection, it is selected (AABB collision)
    - perhaps use a "isSelected" flag to determine whether to move a card in selection
    - cards need to be moved to a point relative to their starting point... how tho
        > find offset from center of selection and maintain it?
        > everytime you move card, (destination - current)*frametime*speed
        > destination = mousepos + offset from center of selection
        > where do i save the offset if at all?
        > selectionOffset member of Card
*/

int max(int a, int b){
    if(a > b){ return a; }
    else{ return b; }
}
int min(int a, int b){
    if(a < b){ return a; }
    else{ return b; }
}

void drawSelection(Vector2 corner1, Vector2 corner2, Color selection_color){
    int posX = min(corner1.x, corner2.x);
    int posY = min(corner1.y, corner2.y);
    int width = max(corner1.x, corner2.x) - posX;
    int height = max(corner1.y, corner2.y) - posY;
    Color rect_color = {selection_color.r, selection_color.g, selection_color.b, selection_color.a/10};
    DrawRectangle(posX, posY, width, height, rect_color);
    DrawRectangleLines(posX, posY, width, height, selection_color);
    
}

bool isCardInSelection(Card card, Vector2 corner1, Vector2 corner2){
    return (card.x >= min(corner1.x, corner2.x) &&
            card.x + card.width <= max(corner1.x, corner2.x) &&
            card.y >= min(corner1.y, corner2.y) &&
            card.y + card.height <= max(corner1.y, corner2.y));
}

int addSelectedCardsToSelection(Card* cards, Card** selected_cards, int num_cards, Vector2 corner1, Vector2 corner2){
    int selected_cards_index = 0;
    for(int i=0; i<num_cards; i++){
        if(isCardInSelection(cards[i], corner1, corner2)){
            
            selected_cards[selected_cards_index] = &(cards[i]);
            selected_cards_index++;
        }
    }
    
    return selected_cards_index;
}

void shuffleSelectedCards(Card** selected_cards, int num_selected_cards){
    bool drawn[num_selected_cards] = {};
    Card new_order[num_selected_cards]; 
    for(int i = 0; i < num_selected_cards; i++){
        int random_num = GetRandomValue(0, num_selected_cards - 1);
        while(drawn[random_num]){
            random_num = GetRandomValue(0, num_selected_cards - 1);
        }
        new_order[i] = *(selected_cards[random_num]);
        drawn[random_num] = true;
    }
    
    for(int i=0; i < num_selected_cards; i++){
        *(selected_cards[i]) = new_order[i];
    }
}

int main(){
    InitWindow(1400, 1000, "cards");
    bool holding = false;
    int drag_speed = 20;
    int border_weight = 3;
    int clicked_card = -1;
    Color bg_color = {0x57, 0x34, 0x73, 0xFF};
    
    /*
        click on card
        swap card with top card
        move all cards above that one down
    */
    int num_cards = 52;
    Card cards[num_cards];
    Card* selected_cards[num_cards] = {}; // initialized as NULL
    int num_selected_cards = 0;
    
    for(int suit = HEARTS; suit<=CLUBS; suit++){
        for(int rank = ACE; rank<=KING; rank++){
            int index = suit*(KING+1) + rank;
            cards[index].x = index*5;
            cards[index].y = 0;
            cards[index].width = 96;
            cards[index].height = 128;
            cards[index].suit = suit;
            cards[index].rank = rank;
            cards[index].isFlipped = false;
        }
    }
    // load card texture
    Texture2D card_texture = LoadTexture("res/cards.png");
    
    
    // sound effects
    InitAudioDevice();
    Sound card_sound = LoadSound("card.ogg");
    SetSoundVolume(card_sound, 3);
    SetTargetFPS(144);
    
    
    // selection
    Vector2 corner1;
    Vector2 corner2;
    bool isSelecting = false;
    Color selection_color = GREEN;
    while(!WindowShouldClose()){
        
        
        if(IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            corner1 = GetMousePosition();
            isSelecting = true;
        }
        else if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            num_selected_cards = 0;
            clicked_card = getTopCardCollidingWithMouse(cards, num_cards);
            if(clicked_card != -1){
                holding = !holding;
                PlaySound(card_sound);
                moveClickedCardToTop(cards, num_cards, clicked_card);
                if(holding){
                    SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                }
                else{
                    moveCardToTopOfCardCursorIsOn(cards, num_cards);
                    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                }
            }

        }        
        if(isSelecting){
            corner2 = GetMousePosition();
        }
        
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && isSelecting){
            isSelecting = false;
            num_selected_cards = addSelectedCardsToSelection(cards, selected_cards, num_cards, corner1, corner2);         
            
        }
        
        // SELECTED CARDS OPERATIONS START ==========================================================================
         
        if(IsKeyPressed(KEY_F) && num_selected_cards != 0){
            PlaySound(card_sound);
            for(int i=0; i<num_selected_cards; i++){
                
                selected_cards[i]->isFlipped = !(selected_cards[i]->isFlipped);
            }
        }
        
        if(IsKeyDown(KEY_C) && num_selected_cards != 0){
            for(int i=0; i<num_selected_cards; i++){
                selected_cards[i]->x += ((GetMouseX() - selected_cards[i]->width/2) - selected_cards[i]->x)*drag_speed*GetFrameTime();
                selected_cards[i]->y += ((GetMouseY() - selected_cards[i]->height/2) - selected_cards[i]->y)*drag_speed*GetFrameTime();
            }
        }
        
        if(IsKeyDown(KEY_M) && num_selected_cards != 0){
            for(int i=0; i<num_selected_cards; i++){
                selected_cards[i]->x += GetMouseDelta().x;
                selected_cards[i]->y += GetMouseDelta().y;
            }
        }
        
        if(IsKeyDown(KEY_D)){
            for(int i=0; i<num_selected_cards; i++){
                selected_cards[i]->x += ((GetMouseX() - selected_cards[i]->width/2) - selected_cards[i]->x)*drag_speed*GetFrameTime();
                selected_cards[i]->y += ((GetMouseY() + 3*i - selected_cards[i]->height/2) - selected_cards[i]->y)*drag_speed*GetFrameTime();
            }
        }
        
        if(IsKeyPressed(KEY_S) && num_selected_cards != 0){
            PlaySound(card_sound);
            shuffleSelectedCards(selected_cards, num_selected_cards);
        }
        
        // SELECTED CARDS OPERATIONS END =================================================================================
        
        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !holding){
            clicked_card = getTopCardCollidingWithMouse(cards, num_cards);
            if(clicked_card != -1){
                PlaySound(card_sound);
                cards[clicked_card].isFlipped = !cards[clicked_card].isFlipped;
            }
        }
        
        if(IsKeyDown(KEY_R)){
            attractAllCardsToCursor(cards, num_cards, drag_speed/2);
        }
        
        


        // logic for card following mouse (works well)
        if(holding){
            cards[0].x += ((GetMouseX() - cards[0].width/2) - cards[0].x)*drag_speed*GetFrameTime();
            cards[0].y += ((GetMouseY() - cards[0].height/2) - cards[0].y)*drag_speed*GetFrameTime();
        }
        
        
        // misc buttons
        if(IsKeyPressed(KEY_F11)){
            ToggleFullscreen();
        }

    BeginDrawing();
        ClearBackground(bg_color);
        renderCardsFromTexture(cards, num_cards, card_texture);
        if(isSelecting){ drawSelection(corner1, corner2, selection_color); }
    EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
}
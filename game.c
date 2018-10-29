#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
#include <time.h>
#include <malloc.h>

#define FIELD_SIZE 8
#define MEM_FIELD_SIZE (sizeof(FIELD) * FIELD_SIZE * FIELD_SIZE)
#define DISTANCE_BETWEEN ( (FIELD_SIZE) / 4 + (FIELD_SIZE) % 2)
#define FILLING_ROWS ( ( (FIELD_SIZE) - (DISTANCE_BETWEEN) ) / 2 )
#define NUMBER_OF_CRLS ( FILLING_ROWS * FIELD_SIZE / 2 )

#define MAX_POSSIBLE_ADD_KING_TAKES 30
#define MAX_TAKE_OR_MOVE_PER_TURN 100

#define TURNS_CALCULATE 3
#define MAX_DEPTH TURNS_CALCULATE * 2

#define elt(y, x) ( (y) * FIELD_SIZE + x )
#define is_on_diagonal(row1, col1, row2, col2) ( ((row1 ) != (row2)) && ((col1 ) != (col2)) && ( abs( (row2) -  (row1) ) == abs( (col2) -  (col1) ) ) )
#define is_CRL(el) ( el == WHITE_CRL || el == BLACK_CRL )
#define is_KING(el) ( el == WHITE_KING || el == BLACK_KING )
#define is_white(val) ( (val) == WHITE_CRL || (val) == WHITE_KING )
#define is_black(val) ( (val) == BLACK_CRL || (val) == BLACK_KING )
#define is_color(el, color) ( (color) == WHITE ? is_white(el) : is_black(el) )
#define is_anybody_here(el) ( (el != BLANK) && (el != CELL) )
#define is_between(more, num, less_eq) ( ((more) > (num)) && ((num) >= (less_eq)) )
#define module(num) ( ((num) > 0) ? (num) : -(num) )
#define KING_to_CRL(king_val) ( (king_val) + 4 )
#define for_field(row_cnt, col_cnt) \
    BOX row_cnt, col_cnt;\
    for(row_cnt=0;row_cnt<FIELD_SIZE;row_cnt++)\
        for(col_cnt=0;col_cnt<FIELD_SIZE;col_cnt++)
// for move type
#define TAKE 1
#define MOVE 0

// for find_poss_moves
#define WHITE 1
#define BLACK 0

// it's for field array
#define CELL 1
#define BLANK 0
#define WHITE_CRL 6 //CRL = circle
#define BLACK_CRL 9
#define WHITE_KING 2 // 6 - 2 = 4
#define BLACK_KING 5 // 9 - 5 = 4

// for print_field
#define CELL_STR "%c%c%c", 0xB1, 0xB1, 0xB1
#define BLANK_STR "%c%c%c", 0xDB, 0xDB, 0xDB
#define WHITE_CRL_STR " 1 "
#define BLACK_CRL_STR " 0 "
#define WHITE_KING_STR " 2 "
#define BLACK_KING_STR " 5 "


//lol printf
//lal puts

const char LOG_FILENAME[] = "log.txt";

typedef short BOX; // aka cell
typedef BOX* FIELD;
typedef int RATING;

typedef struct{
    BOX y;
    BOX x;
}Position;

typedef struct Move_struct{
    char move_type; // move_type: 1 - take, 0 - move
    Position from;
    Position to;
    Position taked;
    struct Move_struct* next_take; // if type is take, if it's last take = NULL
} Move;

typedef struct{
    RATING value;
    short index;
} Find_Result;

/*inline*/ BOX fake_take(Position from, Position to, FIELD field);
/*inline*/ void cancel_fake_take(Position from, Position to, BOX original_thing, FIELD field);
/*inline*/ Move* copy_move(Move* src_move_ptr);
/*inline*/ void mem_free_move(Move* move_root);
bool is_already_taked(BOX y_pos, BOX x_pos, Move* multi_take_list_to_check);
FIELD create_field();
Find_Result find_max(Find_Result* arr, int arr_size);
Find_Result find_min(Find_Result* arr, int arr_size);
void print_field(FIELD field);
void filling_field(FIELD fd);
BOX make_move(Move m, BOX* taked, FIELD fd);
void anti_move(Move m, BOX is_transformation, BOX* taked_draughts, FIELD fd);
char* Move_to_str(Move* m);
char* full_Move_to_str(Move* m);
Move* find_possible_moves(bool side, short* cnt_ptr, FIELD fd);
Move* get_prev_take(Move* root_take, Move* res_take);
Move* get_last_move(Move* root_take);
int help_with_take(BOX y, BOX x, bool side, Move* takes, short* takes_cnt_ptr, Move* this_take, FIELD fd);
char is_end(FIELD fd);
void game_loop();
short choose_move(Move* poss_moves, int moves_cnt, bool side, FIELD fd);
FIELD read_from_file(const char* filename);
Find_Result rate_func(bool side, char me, FIELD fd);
Find_Result min_max(char depth, bool side, char me, FILE* log_file, FIELD fd);
short choose_move(Move* poss_moves, int moves_cnt, bool side, FIELD fd);
Move* get_n_take(Move* root_take, short take_num);
bool is_curr_equal(Move* m1, Move* m2);
bool is_full_equal(Move* m1, Move* m2);
void swap_field(FIELD fd);
void print_field_log(FIELD fd, FILE* log_file){
    int i, j;
    for(i=0;i<FIELD_SIZE;i++){
        for(j=0;j<FIELD_SIZE;j++){
            putc('0' + fd[elt(i, j)], log_file);
            putc(' ', log_file);
        }
        putc('\n', log_file);
    }
}

void test(){
    char *box_str;
    bool side = WHITE;
    short poss_moves_cnt;
    Move m1, m2,
        *box_move, *one_move,
        *one_move_COPY = NULL,
        *poss_moves;
    Position from_pos, to_pos,
        taked_pos = {-1, -1};
/*
    BOX fd[] = {
        0,1,0,1,0,1,0,1,
        1,0,9,0,1,0,1,0,
        0,1,0,1,0,9,0,1,
        1,0,9,0,1,0,1,0,
        0,1,0,1,0,1,0,1,
        1,0,9,0,9,0,1,0,
        0,1,0,9,0,6,0,1,
        6,0,1,0,1,0,1,0,
        };
*/

    FIELD fd = read_from_file("field.txt");
    print_field(fd);
    from_pos.y = 0;
    from_pos.x = 5;
    to_pos.y = 1;
    to_pos.x = 4;
    m1.from  = from_pos;
    m1.to = to_pos;
    m1.taked = taked_pos;
    m1.move_type = MOVE;
    m1.next_take = NULL;
    from_pos.y = 2;
    from_pos.x = 3;
    to_pos.y = 3;
    to_pos.x = 4;
    m2.from = from_pos;
    m2.to = to_pos;
    m2.taked = taked_pos;
    m2.move_type = MOVE;
    m2.next_take = NULL;
    make_move(m1, NULL, fd);
    print_field(fd);
    make_move(find_possible_moves(WHITE, &poss_moves_cnt, fd)[0], NULL, fd);
    print_field(fd);
    make_move(m2, NULL, fd);
    print_field(fd);


//    poss_moves = find_possible_moves(side, &poss_moves_cnt, fd);
//    one_move = &poss_moves[0];
//    //print all move
//    for(int j=0;j<poss_moves_cnt;j++){
//        box_move = &poss_moves[j];
//        while(box_move){
//            box_str = Move_to_str(box_move);
//            printf("%s->", box_str);
//            free(box_str);
//            box_move = box_move->next_take;
//        }
//        puts("");
//    }
/*
    puts("TEST");
    puts("Press any button to start test.");
//    getch();
    int i, k,
        K = 6,
        N = (int)pow(10, K),
        M = N/1000;
    short takes_cnt;
    Move takes[MAX_TAKE_OR_MOVE_PER_TURN];
    BOX y = 6,
        x = 5;
    for(i=0;i<N;i++){
        takes_cnt = 0;
        help_with_take(y, x, WHITE, takes, &takes_cnt, NULL, fd);
//        puts("BEF mem_free\n");
//        if(i == 10){
//            printf("%d\n", i);
//            getch();
//            return 666;
//        }
        if(! (i % M)){
            printf("%d %d\n", i, takes_cnt);
            for(k=0;k<takes_cnt;k++){
                box_move = &takes[k];
                while(box_move){
                    box_str = Move_to_str(box_move);
                    printf("%s->", box_str);
                    free(box_str);
                    box_move = box_move->next_take;
                }
                puts("");
            }
        }

       for(k=0;k<takes_cnt;k++){
            printf("%d %d\t", i, k);
            mem_free_move(&takes[k]);
        }
    }
*/

    puts("THE END");
//    getch();
}

/*

int main(){
    srand(time(NULL));
//    test();
    game_loop();
//    print_field(field);
}

*/
FIELD create_field(){
    FIELD res = (FIELD)malloc(MEM_FIELD_SIZE);
    for_field(i, j){
        if((i+j) % 2)
            res[elt(i, j)] = CELL;
        else
            res[elt(i, j)] = BLANK;
    }
    return res;
}

void print_field(FIELD field){
    short i, j, len, _size;

//    COORD pos = {0, 0};
//    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);

    for(len=0;pow(10, len) <= FIELD_SIZE;len++);

    puts("");
    for(i=0;i<FIELD_SIZE;i++){
        printf("%*d%c ", len, FIELD_SIZE - i, 0xB3);
        for(j=0;j<FIELD_SIZE;j++){
            switch(field[elt(i, j)]){
            case CELL:
                 printf(CELL_STR);
                break;
            case BLANK:
                 printf(BLANK_STR);
                break;
            case WHITE_CRL:
                 printf(WHITE_CRL_STR);
                break;
            case BLACK_CRL:
                 printf(BLACK_CRL_STR);
                break;
            case WHITE_KING:
                 printf(WHITE_KING_STR);
                break;
            case BLACK_KING:
                 printf(BLACK_KING_STR);
                break;
            }
        }
        puts("");
    }
    _size = FIELD_SIZE*3+2;
    char *_str = (char*)memset(malloc(_size * sizeof(_size)), 0xC4, _size * sizeof(_size));
    _str[_size] = '\0';
    _str[0] = 0xC0;

    printf("%*c%*s", len, ' ', _size, _str);
    printf("\n%*c", len+2, ' ');
    for(j=0;j<FIELD_SIZE;j++)
         printf(" %c ", 'A'+j);
    puts("");

    free(_str);
}

BOX make_move(Move m, BOX* taked_draughts, FIELD fd){
/*
    printf("MOVE start args %s| %p\n", Move_to_str(&m), taked_ptr);
    Move* box_lol_kek = get_last_move(&m);
    if (m.move_type == TAKE && fd[elt(m.from.y, m.from.x)] == fd[elt(box_lol_kek->to.y, box_lol_kek->to.x)]){
        print_field(fd);
        //getch();
    }
*/
    short cnt;
    BOX transformation = FALSE; // it's when CRL became KING
    Move *curr_move, *last_move;
    BOX original_val = fd[elt(m.from.y, m.from.x)];
        //*taked_draughts;
/*
    printf("in MOVE: %s|%p\n", Move_to_str(&m), taked_ptr);
    print_field(fd);
*/
    curr_move = &m;
    if(m.move_type == MOVE){
        fd[elt(curr_move->from.y, curr_move->from.x)] = CELL;
        if( (curr_move->to.y == 0) && (original_val == WHITE_CRL) ){
            transformation = WHITE_KING;
        }
        else if( (curr_move->to.y == (FIELD_SIZE - 1)) && (original_val == BLACK_CRL) ){
            transformation = BLACK_KING;
        }
        last_move = curr_move;
    }
    else{
        if(taked_draughts){
            //taked_draughts = malloc(NUMBER_OF_CRLS * sizeof(BOX));
            cnt = 0;
        }
        do{
            if(taked_draughts){
                taked_draughts[cnt++] = fd[elt(curr_move->taked.y, curr_move->taked.x)];
                //cnt++;
            }
            fd[elt(curr_move->taked.y, curr_move->taked.x)] = CELL;
            if( (curr_move->to.y == 0) && (original_val == WHITE_CRL) ){
                transformation = WHITE_KING;
            }
            else if( (curr_move->to.y == (FIELD_SIZE - 1)) && (original_val == BLACK_CRL) ){
                transformation = BLACK_KING;
            }
            if(curr_move->next_take == NULL){
                last_move = curr_move;
                break;
            }
            curr_move = curr_move->next_take;
        }while(curr_move);

        fd[elt(m.from.y, m.from.x)] = CELL;
    }
    fd[elt(last_move->to.y, last_move->to.x)] = transformation ? transformation : original_val;
/*
    if(transformation){
    }
    else{
        fd[elt(last_move->to.y, last_move->to.x)] = original_val;
    }
    if(taked_ptr){
        *taked_ptr = taked_draughts;
    }
*/
    return transformation;
}

void anti_move(Move m, BOX transformation, BOX* taked_draughts, FIELD fd){
/*
    Move* box_lol_kek = get_last_move(&m);
    if (m.move_type == TAKE && fd[elt(m.from.y, m.from.x)] == fd[elt(box_lol_kek->to.y, box_lol_kek->to.x)]){
        printf("ANTI start args %s| %d | %p\n", Move_to_str(&m), is_transformation, taked);
        print_field(fd);
        //getch();
    }
*/
    short cnt;
    BOX original_val = fd[elt(m.to.y, m.to.x)];
    Move *curr_move;
    if(m.move_type == MOVE){
        //fd[elt(m.from.y, m.from.x)] = ;
        fd[elt(m.to.y, m.to.x)] = CELL;
    }
    else{
        cnt = 0;
        curr_move = &m;
        for(cnt=0;curr_move;curr_move = curr_move->next_take, cnt++);
        curr_move = get_last_move(&m);
        original_val = fd[elt(curr_move->to.y, curr_move->to.x)];
        fd[elt(curr_move->to.y, curr_move->to.x)] = CELL;
        do{
            //original_val = fd[elt(curr_move->from.y, curr_move->from.x)];
            //printf("%s<-", )
            fd[elt(curr_move->taked.y, curr_move->taked.x)] = taked_draughts[--cnt];
            curr_move = get_prev_take(&m, curr_move);
        }while(cnt);
    }
    fd[elt(m.from.y, m.from.x)] = transformation ? KING_to_CRL(transformation) : original_val;
/*
    if(transformation){
    }
    else{
        fd[elt(m.from.y, m.from.x)] = original_val;
    }
*/
}

void filling_field(FIELD fd){
    // printf("betw: %d rows: %d\n", dist_betw, filling_rows);
    for_field(i, j){
        if(i < FILLING_ROWS && fd[elt(i, j)] == CELL)
            fd[elt(i, j)] = BLACK_CRL;
        else if (is_between(FIELD_SIZE, i, FIELD_SIZE - FILLING_ROWS) && fd[elt(i, j)] == CELL)
            fd[elt(i, j)] = WHITE_CRL;
    }
}

Move* find_possible_moves(bool side, short* cnt_ptr, FIELD fd){
/*
    printf("%d %p\n", side, cnt_ptr);
    print_field(fd);
    puts("F_P_M");
*/
    //сначала ищем бои, если их число = 0, ищем обычные ходы, иначе не ищем
    short _k, _l, add_y, add_x, emp_k, curr_elt, direction,
        me, enemy,
        add_arr[] = {1, 1, -1, -1, 1, -1, -1, 1},
        moves_cnt = 0;
    Move *moves = (Move*)malloc(MAX_TAKE_OR_MOVE_PER_TURN * sizeof(Move));

    if (side == WHITE){
        me = WHITE;
        enemy = BLACK;
    }
    else{
        me = BLACK;
        enemy = WHITE;
    }
    //it's for taking
    for_field(i, j){
        // printf("%d %d\n", i, j);
        curr_elt = fd[elt(i, j)];
        if (is_color(curr_elt, me)){
            //printf("\tYEA: %d %d\n", i, j);
            help_with_take(i, j, side, moves, &moves_cnt, NULL, fd);
            //puts("HELP WAS TAKEN");
/*
            Move *box_move;
            for(int ik=0;ik<takes_cnt;ik++){
                printf("TAK %d/%d\n", ik+1, takes_cnt);
                box_move = &takes[ik];
                while(box_move){
                    printf("%s->", Move_to_str(box_move));
                    box_move = box_move->next_take;
                }
                puts("");
            }
*/
        }
    }
    if(moves_cnt){ // if it was one or more takes not necessary to check possibility of moves
        *cnt_ptr = moves_cnt;
        return moves;
    }
    for_field(k, l){
        curr_elt = fd[elt(k, l)];
        if (is_color(curr_elt , me)){
            if(is_CRL(curr_elt)){
                if(me == WHITE)
                    _k = k - 1;
                else
                    _k = k + 1;
                for(_l=l-1;_l<l+2;_l+=2){
                    if (is_between(FIELD_SIZE, _k, 0) && is_between(FIELD_SIZE, _l, 0) &&\
                        fd[elt(_k, _l)] == CELL){
                            moves[moves_cnt].move_type = MOVE;
                            moves[moves_cnt].from.y = k;
                            moves[moves_cnt].from.x = l;
                            moves[moves_cnt].to.y = _k;
                            moves[moves_cnt].to.x = _l;
                            moves[moves_cnt].taked.y = -1;
                            moves[moves_cnt].taked.x = -1;
                            moves[moves_cnt].next_take = NULL;
                            // printf("\t%d %d\t%d %d %d %d\n", side, moves_cnt, moves[moves_cnt].from.y,\
                                   moves[moves_cnt].from.x, moves[moves_cnt].to.y, moves[moves_cnt].to.x);
                            //printf("\tfpm M: %d %d\t%c%d %c%d\n", side, moves_cnt, 'A' + moves[moves_cnt].from.x,\
                                   FIELD_SIZE - moves[moves_cnt].from.y, 'A' + moves[moves_cnt].to.x, FIELD_SIZE - moves[moves_cnt].to.y);
                            moves_cnt++;
                    }
                }
            }
            else{
                for(direction=0;direction<4;direction++){
                    add_y = add_arr[direction*2];
                    add_x = add_arr[direction*2+1];

                    for(emp_k=1;emp_k<FIELD_SIZE;emp_k++){
                        _k = k + add_y * emp_k;
                        _l = l + add_x * emp_k;
                        // _k, _l - coordknates of empty cell
//                        printf("FPM MOVE INFO: (%d %d) %d %d %d\t", k, l, add_y, add_x, emp_k);
//                        printf("\t\tKING MB MOVE: %d %d\n", _k, _l);
                        if(is_between(FIELD_SIZE, _k, 0) && is_between(FIELD_SIZE, _l, 0) &&\
                           (fd[elt(_k, _l)] == CELL)){
                            moves[moves_cnt].move_type = MOVE;
                            moves[moves_cnt].from.y = k;
                            moves[moves_cnt].from.x = l;
                            moves[moves_cnt].to.y = _k;
                            moves[moves_cnt].to.x = _l;
                            moves[moves_cnt].taked.y = -1;
                            moves[moves_cnt].taked.x = -1;
                            moves[moves_cnt].next_take = NULL;
                            // printf("\t%d %d\t%d %d %d %d\n", side, moves_cnt, moves[moves_cnt].from.y,\
                                   moves[moves_cnt].from.x, moves[moves_cnt].to.y, moves[moves_cnt].to.x);
                             //printf("\tfpm M: %d %d\t%c%d %c%d\n", side, moves_cnt, 'A' + moves[moves_cnt].from.x,\
                                   FIELD_SIZE - moves[moves_cnt].from.y, 'A' + moves[moves_cnt].to.x, FIELD_SIZE - moves[moves_cnt].to.y);
                            moves_cnt++;
                        }
                        else
                            break;
                    }
                }
            }
        }
    }

    *cnt_ptr = moves_cnt;

    return moves;
}

char is_end(FIELD fd){
    char white_cnt = 0,
        black_cnt = 0,
        curr_val;
    for_field(i, j){
        curr_val = fd[elt(i, j)];
        if (is_white(curr_val))
            white_cnt++;
        else if (is_black(curr_val))
            black_cnt++;
    }
    if (white_cnt && black_cnt)
        return 0;
    else
        return 1;
}

void game_loop(){
    char move_num;
    bool side = WHITE;
    short i, moves_cnt;
    Move curr_move,
        *poss_moves;
    FIELD field = create_field();
    filling_field(field);

    //test
    /*
    side = BLACK;
    field = read_from_file("field.txt");
    */

    print_field(field);
    do{
        puts("Wait for press some button...");
        getchar();
        poss_moves = find_possible_moves(side, &moves_cnt, field);
/*
        puts("POSS_MOVES IN MAIN LOOP");
        for (i=0;i<moves_cnt;i++)
            printf("%d (%d %d | %d %d)  ", side, poss_moves[i].from.y, poss_moves[i].from.x, poss_moves[i].to.y, poss_moves[i].to.x);
        puts("");
*/
        if (moves_cnt == 0){
            puts("NO WAY!!!!!!!!!!!!!!!!!!!!");
            goto no_way_end;
        }
        move_num = choose_move(poss_moves, moves_cnt, side, field);
        curr_move = poss_moves[move_num];
//         printf("%d %d\t%d %d %d %d\n", side, move_num, poss_moves[move_num].from.y,\
                 poss_moves[move_num].from.x, poss_moves[move_num].to.y, poss_moves[move_num].to.x);
//        printf("MAIN LOOP: %s %d %d\t%c%d %c%d\n", (curr_move.move_type == MOVE ? "MOVE" : "TAKE"), side, move_num, 'A' + curr_move.from.x,\
                FIELD_SIZE - curr_move.from.y, 'A' + curr_move.to.x, FIELD_SIZE - curr_move.to.y);
        make_move(curr_move, NULL, field);
        side = !side;
        print_field(field);


        for(i=0;i<moves_cnt;i++){
            mem_free_move(&(poss_moves[i]));
        }
    } while (!is_end(field));
    no_way_end:;
    puts("");
//    getch();

//    free();
}

FIELD read_from_file(const char* filename){
    char i, j;
    BOX val;
    FIELD fd = (FIELD)malloc(MEM_FIELD_SIZE);
    FILE *f = fopen(filename, "r");
    for (i=0;i<FIELD_SIZE;i++){
        for (j=0;j<FIELD_SIZE;j++){
            val = getc(f) - '0';
            fd[elt(i, j)] = val;
            getc(f);
        }
    }
    fclose(f);
    return fd;
}

Find_Result find_max(Find_Result* arr, int arr_size){
    short i;
    Find_Result res = arr[0];
    for (i=1;i<arr_size;i++)
        if (arr[i].value > res.value){
            res.value = arr[i].value;
            res.index = i;
        }
    return res;
}

Find_Result find_min(Find_Result* arr, int arr_size){
    short i;
    Find_Result res = arr[0];
    for (i=1;i<arr_size;i++)
        if (arr[i].value < res.value){
            res.value = arr[i].value;
            res.index = i;
        }
    return res;
}

char* Move_to_str(Move* m){
    if(!m)
        return NULL;

    char text_Move[] = "%s|(%d %d) (%d %d) (%d %d) (%p)",
        *type_text, //[] = (m->move_type == TAKE ? "TAKE" : "MOVE"),
        size_of_text = strlen(text_Move) + 10 * 7 + 7 + 10, // razmer stroki + dlina chisla * vsego chisel + dlya tipa hoda + na vsyakiu sluchai
        *res_str = (char*)malloc(size_of_text * sizeof(char));
    //printf("strlen: %d\n", strlen(text_Move));
    type_text = strcpy((char*)malloc(4 * sizeof(char)), (m->move_type == TAKE ? "TAKE" : "MOVE"));
    sprintf(res_str, text_Move, type_text, m->from.y, m->from.x, m->to.y, m->to.x,\
            m->taked.y, m->taked.x, m->next_take);
    free(type_text);
    return res_str;
}

Find_Result rate_func(bool side, char me, FIELD fd){
    BOX white_cnt = 0,
        black_cnt = 0;
    for_field(i, j){
        switch (fd[elt(i, j)]){
        case WHITE_KING:
            white_cnt++;
        case WHITE_CRL:
            white_cnt++;
            break;
        case BLACK_KING:
            black_cnt++;
        case BLACK_CRL:
            black_cnt++;
            break;
        }
    }
    Find_Result res = {0, -1};
    if(me == WHITE)
        res.value = white_cnt - black_cnt;
    else
        res.value = black_cnt - white_cnt;
    //printf("END NODE: %d\n", res.value);
    //getch();
    return res;
}

Find_Result min_max(char depth, bool side, char me, FILE* log_file, FIELD fd){
/*
    printf("START MIN_MAX %d %d %d\n", depth, side, me);
    print_field(fd);
    if(depth < MAX_DEPTH){
    }
*/
    if (depth == MAX_DEPTH){
        //print_field(fd);
     //rate_position
        return rate_func(side, me, fd);
    }
    short i, j, k, poss_cnt;
    BOX transformation,
        taked[NUMBER_OF_CRLS];
    FIELD *box_ptr;
        //test_fd = malloc(MEM_FIELD_SIZE);
    Find_Result box_result,
        ratings[MAX_TAKE_OR_MOVE_PER_TURN]; //= malloc(poss_cnt * sizeof(Find_Result));
    Move *box_move,
        //*curr_move,
        *poss_moves;

    poss_moves = find_possible_moves(side, &poss_cnt, fd);

    //printf("CNT: %d\n", poss_cnt);
    //printf("DEPTH %d MIN_MAX MOVES(%d): ", depth, poss_cnt);
/*
    for(k=0;k<poss_cnt;k++){
        printf("(%d %d) (%d %d)| ", poss_moves[k].from.y, poss_moves[k].from.x,\
               poss_moves[k].to.y, poss_moves[k].to.x);
    }
    puts("");
*/
    if(poss_cnt == 0){
        return rate_func(side, me, fd);
/*
        puts("NO poss_cnt in MIN_MAX");
        print_field(fd);
        getch();
*/
    }

    for(i=0;i<poss_cnt;i++){
        transformation = make_move(poss_moves[i], taked, fd);
/*
*/
        if(depth == 0){
            puts("aft MOVE");
            print_field(fd);
        }
        else{
            putc('\t', log_file);
            putc('0' + depth, log_file);
            putc(' ', log_file);
            int l;
            char aft_move[] = "aft MOVE\n";
            for(l=0;l<strlen(aft_move);l++)
                putc(aft_move[l], log_file);
            //print_field_log(fd, log_file);
        }
        box_result = min_max(depth+1, !side, me, log_file, fd);
        box_result.index = i;
        ratings[i] = box_result;
/*
*/
        if(depth == 0){
            printf("HELLO THERE rt[%d/%d] (%d %d)\n", i+1,\
                   poss_cnt, ratings[i].index, ratings[i].value);
        }
        else{
            int l;
            char hello_there[50];
            sprintf(hello_there, "HELLO THERE rt[%d/%d] (%d %d)\n", i+1,\
                   poss_cnt, ratings[i].index, ratings[i].value);
            for(l=0;l<strlen(hello_there);l++)
                putc(hello_there[l], log_file);
            //print_field_log(fd, log_file);
        }
        anti_move(poss_moves[i], transformation, taked, fd);

/*
*/
        if(depth == 0){
            puts("aft ANTI");
            print_field(fd);
            //getchar();
        }
        else{
            int l;
            char aft_anti[20];
            sprintf(aft_anti, "aft ANTI %d\n", depth);
            for(l=0;l<strlen(aft_anti);l++)
                putc(aft_anti[l], log_file);
            //print_field_log(fd, log_file);
        }
    }
/*
*/
    for(i=0;i<poss_cnt;i++){
        mem_free_move(&(poss_moves[i]));
    }
    if (side == me){
        box_result = find_max(ratings, poss_cnt);
    }
    else{
        box_result = find_min(ratings, poss_cnt);
    }

    free(poss_moves);
    return box_result;
}

int help_with_take(BOX y, BOX x, bool side, Move* takes, short* takes_cnt_ptr, Move* this_take, FIELD fd){
    //lal("\n\nI SEE YOU NEED MY HALP?!");

    /////
    //lal("now you have such takelist:");
//    //lolf("CHECK this_take %p\n", this_take);
    ////lal(full_Move_to_str(this_take));
   // //lolf("CHECK this_take %p\n", this_take);
/*
    Move *box_M0ve = this_take;
    char *box_str;// = Move_to_str(box_M0ve);
    while(box_M0ve){
        box_str = Move_to_str(box_M0ve);
        //lolf("%s", box_str);
        free(box_str);
        //lolf("->");
        box_M0ve  = box_M0ve ->next_take;
    }
    //lal("");
*/
    /////

    //lal("              | ");
    //lal("You are HERE \\|/");
    ////lol_field(fd);
    //getch();
    char test_add_cnt,
        test_add_dir = 2;
    short i, _y, _x, direction, add_y, add_x, _add,
        empty_y, empty_x, emp_i, me, enemy,
        cont_take_y, cont_take_x, test_add_y, test_add_x, start_dir,
        add_taking_K_arr[4][FIELD_SIZE], // 4 directions
        add_taking_K_cnt = 0,
        test_i, test_emp, test_empty_y, test_empty_x,
        takes_cnt = *takes_cnt_ptr,
        poss_takes_cnt = 0,
        add_arr[] = {1, 1, -1, -1, 1, -1, -1, 1};
    bool is_helpful = FALSE, // was take or not in this help_with_take
        is_add_taking_K = FALSE;
    Move *curr_take, *previous_take, *root_take,
        *this_take_COPY,
        poss_takes[MAX_TAKE_OR_MOVE_PER_TURN];
    Position from_take, to_take;
    BOX original;

    if(side == WHITE){
        me = WHITE;
        enemy = BLACK;
    }
    else{
        me = BLACK;
        enemy = WHITE;
    }

    if(is_CRL(fd[elt(y, x)])){
        //lal("OH! IT'S CRL");
        for(_y=y-1;_y<y+2;_y+=2){
            for(_x=x-1;_x<x+2;_x+=2){
                // _y, _x - coordinates of enemy
                // empty_y, empty_x - coordinates of distanation empty cell for take
                //lolf("\t%d %d\n", _y, _x);
                empty_y = y + (_y - y) * 2;
                empty_x = x + (_x - x) * 2;
                //lolf("\t\t%d %d\n", empty_y, empty_x);
                ////lolf("CHECK CRL in HELP: %d %d %d %d\n",\
                    is_between(FIELD_SIZE-1, _y, 1), is_between(FIELD_SIZE-1, _x, 1) ,\
                    is_color(fd[elt(_y, _x)], enemy) , fd[elt(empty_y, empty_x)] == CELL);
                if (is_between(FIELD_SIZE-1, _y, 1) && is_between(FIELD_SIZE-1, _x, 1) &&\
                    is_color(fd[elt(_y, _x)], enemy) && fd[elt(empty_y, empty_x)] == CELL){


                        if(is_already_taked(_y, _x, this_take))
                            continue;

                        //lal("PAU!");
                        from_take.y = y;
                        from_take.x = x;
                        to_take.y = empty_y;
                        to_take.x = empty_x;

                        poss_takes[poss_takes_cnt].from = from_take;
                        poss_takes[poss_takes_cnt].to = to_take;
                        poss_takes[poss_takes_cnt].taked.y = _y;
                        poss_takes[poss_takes_cnt].taked.x = _x;
                        poss_takes[poss_takes_cnt].move_type = TAKE;
                        poss_takes[poss_takes_cnt].next_take = NULL;
                        poss_takes_cnt++;
//                        //lolf("\t%d %d\t%d %d %d %d\n", side, takes_cnt, takes[takes_cnt].from.y,\
                               takes[takes_cnt].from.x, takes[takes_cnt].to.y, takes[takes_cnt].to.x);
//                        //lolf("\tfpm T: %d %d\t%c%d %c%d\n", side, takes_cnt, 'A' + takes[takes_cnt].from.x,\
                               FIELD_SIZE - takes[takes_cnt].from.y, 'A' + takes[takes_cnt].to.x, FIELD_SIZE - takes[takes_cnt].to.y);
                }
            }
        }
    }
    else{
        //lal("OH! IT'S MA KING");
        //lolf("\t|\t|\t|\tKING ON %d %d\n", y, x);
        for(direction=0;direction<4;direction++){
            add_y = add_arr[direction*2];
            add_x = add_arr[direction*2+1];
            add_taking_K_cnt = 0;
            is_add_taking_K = FALSE;
            _add = 1;
            do{
                _y = y + add_y * _add;
                _x = x + add_x * _add;
                // _y, _x - coordinates of enemy
                // empty_y, empty_x - coordinates of distanation empty cell for take
                //lolf("\t\tKING MB TAKE: %d %d\n", _y, _x);
                // //lolf("\t\t%d %d\n", empty_y, empty_x);
                if (is_color(fd[elt(_y, _x)], enemy)){



                    if(is_already_taked(_y, _x, this_take))
                        break;

                    if(direction < 2){
                        start_dir = 4;
                    }
                    else{
                        start_dir = 0;
                    }
                    for(emp_i=1;emp_i<FIELD_SIZE;emp_i++){
                        empty_y = _y + (_y - y) / module(_y - y) * emp_i;
                        empty_x = _x + (_x - x) / module(_x - x) * emp_i;
                        //lolf("BEF empts: %d %d\n", empty_y, empty_x);
                        add_taking_K_arr[direction][emp_i] = FALSE;
                        if(is_between(FIELD_SIZE, empty_y, 0) && is_between(FIELD_SIZE, empty_x, 0) &&\
                           fd[elt(empty_y, empty_x)] == CELL){
                               for(test_add_cnt=0;test_add_cnt<test_add_dir;test_add_cnt++){
                                   test_add_y = add_arr[ start_dir + test_add_cnt * 2 ];
                                   test_add_x = add_arr[ (start_dir + 1) + test_add_cnt * 2 ];
                                   for(test_i=1;test_i<FIELD_SIZE-1;test_i++){
                                       cont_take_y = empty_y + test_add_y * test_i;
                                       cont_take_x = empty_x + test_add_x * test_i;
                                       //lolf("TEST ADD%d %d %d\n", test_add_cnt+1, cont_take_y, cont_take_x);
                                       if(is_between(FIELD_SIZE-1, cont_take_y, 1) && is_between(FIELD_SIZE-1, cont_take_x, 1)){
                                            if( fd[elt(cont_take_y, cont_take_x)] != CELL ){
                                                if( (!is_color(fd[elt(cont_take_y, cont_take_x)], enemy)) ||\
                                                   is_already_taked(cont_take_y, cont_take_x, this_take)){
                                                       break;
                                                }

                                                for(test_emp=1;test_emp<FIELD_SIZE-2;test_emp++){
                                                    test_empty_y = cont_take_y + test_add_y * test_emp;
                                                    test_empty_x = cont_take_x + test_add_x * test_emp;
                                                    if(is_between(FIELD_SIZE, test_empty_y, 0) && is_between(FIELD_SIZE, test_empty_x, 0) &&\
                                                       fd[elt(test_empty_y, test_empty_x)] == CELL){
                                                           add_taking_K_arr[direction][emp_i] = TRUE;
                                                           is_add_taking_K = TRUE;
                                                           add_taking_K_cnt++;
                                                           //lolf("\t%d: from |%d %d| to |%d %d| taking |%d %d| cnt %d\n", test_add_cnt+1,\
                                                                  empty_y, empty_x, test_empty_y, test_empty_x,\
                                                                  cont_take_y, cont_take_x, add_taking_K_cnt);
                                                    }
                                                }
                                            }
                                       }
                                       else break;
                                   }
                               }
                        }
                        else
                            break;

                    }
                    for(emp_i=1;emp_i<FIELD_SIZE;emp_i++){
                        empty_y = _y + (_y - y) / module(_y - y) * emp_i;
                        empty_x = _x + (_x - x) / module(_x - x) * emp_i;
                        //lolf("\t\tis %d %d TRUE?\n", direction, emp_i);
                        //lolf("CHECK (%d %d) %d | %d %d %d  (%d %d) %d |\n", empty_y, empty_x, direction,\
                            is_between(FIELD_SIZE, empty_y, 0), is_between(FIELD_SIZE, empty_x, 0),\
                            fd[elt(empty_y, empty_x)] == CELL,\
                            add_taking_K_arr[direction][emp_i], is_add_taking_K,\
                            add_taking_K_arr[direction][emp_i] == is_add_taking_K);
                        if(is_between(FIELD_SIZE, empty_y, 0) && is_between(FIELD_SIZE, empty_x, 0) &&\
                           fd[elt(empty_y, empty_x)] == CELL){
                            if(add_taking_K_arr[direction][emp_i] == is_add_taking_K){

                                //lal("KING GOING IS DOMINATING!!1");

                                ////////////////////////
                                from_take.y = y;
                                from_take.x = x;
                                to_take.y = empty_y;
                                to_take.x = empty_x;

                                poss_takes[poss_takes_cnt].from = from_take;
                                poss_takes[poss_takes_cnt].to = to_take;
                                poss_takes[poss_takes_cnt].taked.y = _y;
                                poss_takes[poss_takes_cnt].taked.x = _x;
                                poss_takes[poss_takes_cnt].move_type = TAKE;
                                poss_takes[poss_takes_cnt].next_take = NULL;
                                poss_takes_cnt++;
                            }
                        }
                        else
                            goto change_dir;
                    }
                }
                else if (is_color(fd[elt(_y, _x)], me))
                    goto change_dir;
                _add++;
            } while (is_between(FIELD_SIZE-1, _y, 1) && is_between(FIELD_SIZE-1, _x, 1));
            change_dir:;
        }
    }
    //lolf("LAST FOR IN HELP: %d\n", poss_takes_cnt);
    for(i=0;i<poss_takes_cnt;i++){
        //lolf("FOR %d/%d\n", i+1, poss_takes_cnt);

        //lal("now you have such takelist in LAST FOR:");
        ////lal(full_Move_to_str(this_take));
/*
        Move *box = this_take;
        char *box_str;
        while(box){
            //box_str = Move_to_str(box);
            //lolf("%s->", Move_to_str(box));
            box = box->next_take;
            //free(box_str);
        }
        //lal("");
*/

        //lolf("                    | (%d %d)\n", poss_takes[i].to.y, poss_takes[i].to.x);
        //lal("AND NOW CHECK THIS \\|/");
        ////lol_field(fd);
        original = fake_take(poss_takes[i].from, poss_takes[i].to, fd);

        curr_take = &poss_takes[i];
        curr_take->next_take = NULL;

        previous_take = this_take ? get_last_move(this_take) : curr_take; // this_take ? this_curr_take : curr_take;
        if(this_take){
            //lolf("prev: %p\n", previous_take);
            previous_take->next_take = curr_take;
            //this_take_COPY = malloc(sizeof(Move));
            this_take_COPY = copy_move(this_take);
            root_take = this_take_COPY;
        }
        else
            root_take = curr_take;

        is_helpful = help_with_take(poss_takes[i].to.y, poss_takes[i].to.x, side, takes, &takes_cnt, root_take, fd);
        cancel_fake_take(curr_take->to, curr_take->from, original, fd);

        //lolf("HELPFULNESS %d BY %s\n", is_helpful, Move_to_str(curr_take));
        if(!is_helpful){
            //lal("-----------------");
            //lal("\t\t\tIT'S NOT HELPFUL :(");
            //lal("-----------------");
            //previous_take->next_take = NULL;
            takes[takes_cnt] = *root_take; // if it's last poss take in current multi-taking
            takes_cnt++;
        }
        else{
            //free(for_curr_take);
            if(this_take){
                mem_free_move(this_take_COPY);
            }
        }
        previous_take->next_take = NULL;
        //cancel_fake_Take(curr_take->to, previous_take->from, fd);
        ////lolf("cancel_fake_Take: (%d %d) (%d %d)\n", curr_take->to.y, curr_take->to.x, previous_take->from.y, previous_take->from.x);

    }
    //lolf("POSS_TAKES_CNT WAS %d\t", poss_takes_cnt);
    poss_takes_cnt = takes_cnt - *takes_cnt_ptr; // final number of taked draughts
    //lolf("AND BECOME %d\n", poss_takes_cnt);

    //lolf("*TAKES_CNT_PTR WAS %d\t", *takes_cnt_ptr);
    *takes_cnt_ptr = takes_cnt;
    //lolf("AND BECOME %d\n", *takes_cnt_ptr);

    if(!poss_takes_cnt)
        //lal("NOTHING WAS FINDED :(");
        ;
    return poss_takes_cnt;
}

BOX fake_take(Position from, Position to, FIELD field){
    BOX original_thing = field[ elt(from.y, from.x) ];
    if( (to.y == (FIELD_SIZE - 1)) && (field[ elt(from.y, from.x) ] == BLACK_CRL) ){
        field[ elt(to.y, to.x) ] = BLACK_KING;
    }
    else if( (to.y == 0) && (field[elt(from.y, from.x)] == WHITE_CRL) ){
        field[ elt(to.y, to.x) ] = WHITE_KING;
    }
    else field[ elt(to.y, to.x) ] = field[ elt(from.y, from.x) ];
    field[elt(from.y, from.x)] = CELL;
    return original_thing;
}

void cancel_fake_take(Position from, Position to, BOX original_thing, FIELD field){
    /*
    if( (field[ (from.y == FIELD_SIZE-1) && elt(from.y, from.x) ] == BLACK_KING) && is_transformation)
        field[ elt(to.y, to.x) ] = BLACK_CRL;
    else if( (from.y == 0) && (field[ elt(from.y, from.x) ] == WHITE_KING) && is_transformation)
        field[ elt(to.y, to.x) ] = WHITE_CRL;
    else field[ elt(to.y, to.x) ] = field[ elt(from.y, from.x) ];
    */
    field[ elt(to.y, to.x) ] = original_thing;
    field[ elt(from.y, from.x) ] = CELL;
}

Move* copy_move(Move* src_move_ptr){
    if(!src_move_ptr) return NULL;

    Move *result = (Move*)malloc(sizeof(Move)),
        *box_src = src_move_ptr,
        *box_dest = result,
        *prev_box = NULL;
    do{
        memcpy(box_dest, box_src, sizeof(Move));
        if(prev_box)
            prev_box->next_take = box_dest;
        prev_box = box_dest;
        box_dest = (Move*)malloc(sizeof(Move));
        box_src = box_src->next_take;
    }while(box_src);
    free(box_dest);
    //prev_box->next_take = NULL;
    return result;
}

void mem_free_move(Move* move_root){
//    printf("m_fr: %p|", move_root);
    Move *take = move_root,
        *temp_box;

    while(take){
//        printf("%p->", take);
        temp_box = take->next_take;
        free(take);
        take = temp_box;
    }
//    printf("\n");
}

bool is_already_taked(BOX y_pos, BOX x_pos, Move* multi_take_list_to_check) {
    bool already_taked = FALSE;
    Move* box_move = multi_take_list_to_check;
    while(box_move){
        if(box_move->taked.y == y_pos &&
           box_move->taked.x == x_pos){
            already_taked = TRUE;
            break;
        }
        if(box_move->next_take)
            box_move = box_move->next_take;
        else{
            break;
        }
    }
    return already_taked;
}

Move* get_prev_take(Move* root_take, Move* res_take){
    Move *temp_move_box = root_take;
    //printf("get_prev %s %s\n", Move_to_str(root_take), Move_to_str(res_take));
    while(temp_move_box){
//        printf("%s->", Move_to_str(temp_move_box));
        if(temp_move_box->next_take == res_take){
            //puts("");
            return temp_move_box;
        }
        temp_move_box = temp_move_box->next_take;
    }
    //puts("");
    return NULL;
}

Move* get_last_move(Move* root_take){
//    if(root_take->move_type == MOVE)
//        return NULL;
    Move *temp_move_box = root_take;
    while(temp_move_box){
        if(temp_move_box->next_take == NULL){
            return temp_move_box;
        }
        temp_move_box = temp_move_box->next_take;
    }
    return NULL;
}

int get_move_len(Move* root_take){
    Move *temp_move_box = root_take;
    int cnt = 0;
    while(temp_move_box){
        temp_move_box=temp_move_box->next_take;
        cnt++;
    }
    return cnt;
}

Move* get_n_take(Move* root_take, short take_num){
    if(!root_take) // if root_take is NULL
        return root_take;
    short cnt = 0;
    Move* temp_take = root_take;

    while(temp_take && cnt < take_num){
        temp_take = temp_take->next_take;
        cnt++;
    }
    return temp_take;
}

short choose_move(Move* poss_moves, int moves_cnt, bool side, FIELD fd){
    FILE *log_file = fopen(LOG_FILENAME, "w");
    Find_Result res_move = min_max(0, side, side, log_file, fd);
    fclose(log_file);
    return res_move.index;
    //return rand() % moves_cnt;
}

bool is_curr_equal(Move* m1, Move* m2){
    return (m1->from.y == m2->from.y) && (m1->from.x == m2->from.x) &&\
           (m1->to.y == m2->to.y) && (m1->to.x == m2->to.x) &&\
           (m1->taked.y == m2->taked.y) && (m1->taked.x == m2->taked.x);
}

bool is_full_equal(Move* m1, Move* m2){
    Move *box_m1 = m1,
        *box_m2 = m2;
    if(!m1 || !m2) return FALSE; // if one of them is NULL
    do{
        if(is_curr_equal(box_m1, box_m2)){
               box_m1 = box_m1->next_take;
               box_m2 = box_m2->next_take;
           }
        else{
            return FALSE;
        }
    }while(box_m1);
    return TRUE;
}

void swap_field(FIELD fd){
    BOX temp, i;
    int full_field_size = FIELD_SIZE * FIELD_SIZE;
    for(i=0;i<full_field_size/2;i++){
        //swap fd[i] and fd[(full_field_size-1) - i]
        printf("%d: %d %d\n", i, fd[i], fd[(full_field_size-1) - i]);
        temp = fd[i];
        fd[i] = fd[(full_field_size-1) - i];
        fd[(full_field_size-1) - i] = temp;
    }
}




#include <ncurses.h>
#include <unistd.h>

#define TICK 50000
#define MAX_ENEMIES 10  // 敵の最大数

// 敵や弾を管理する構造体
typedef struct {
    int x, y;
    int active; // 1なら存在、0なら消滅
} Object;

int main() {
    int max_x, max_y, ch;
    int score = 0;
    
    // 自機の位置
    int player_x;
    
    // 弾と敵のデータ
    Object bullet = {0, 0, 0}; 
    Object enemies[MAX_ENEMIES];

    initscr();
    noecho();
    curs_set(0);
    timeout(0);
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, max_y, max_x);
    player_x = max_x / 2;

    // 敵の初期配置（適当に横に並べる）
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].x = (i + 1) * (max_x / (MAX_ENEMIES + 1));
        enemies[i].y = 2; // 上の方
        enemies[i].active = 1;
    }

    while ((ch = getch()) != 'q') {
        // 1. 入力処理
        if (ch == KEY_LEFT && player_x > 0) player_x--;
        if (ch == KEY_RIGHT && player_x < max_x - 1) player_x++;
        if (ch == ' ' && !bullet.active) {
            bullet.x = player_x;
            bullet.y = max_y - 2;
            bullet.active = 1;
        }

        // 2. 弾の移動と当たり判定
        if (bullet.active) {
            bullet.y--;
            if (bullet.y < 0) bullet.active = 0;

            // 敵との当たり判定
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active && bullet.x == enemies[i].x && bullet.y == enemies[i].y) {
                    enemies[i].active = 0; // 敵を倒す
                    bullet.active = 0;    // 弾も消える
                    score += 100;
                }
            }
        }

        // 3. 描画処理
        erase();
        
        // スコア表示
        mvprintw(0, 0, "Score: %d | Press 'q' to quit", score);

        // 自機の描画
        mvaddch(max_y - 1, player_x, '^');

        // 弾の描画
        if (bullet.active) mvaddch(bullet.y, bullet.x, '|');

        // 敵の描画
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                mvaddch(enemies[i].y, enemies[i].x, '*');
            }
        }

        refresh();
        usleep(TICK);
    }

    endwin();
    printf("Final Score: %d\n", score);
    return 0;
}

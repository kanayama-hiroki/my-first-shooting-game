#include <ncurses.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#define PI 3.14159265
#define TICK 50000

// 色ペアの定義
#define COLOR_WALL_FAR   1
#define COLOR_WALL_MID   2
#define COLOR_WALL_NEAR  3
#define COLOR_ZOMBIE    4
#define COLOR_BULLET    5
#define COLOR_UI        6
#define COLOR_FLOOR     7

typedef struct {
    double x, y;
    double vx, vy;
    int active;
} Bullet;

typedef struct {
    double x, y;
    int health;
    int active;
} Zombie;

int main() {
    int max_y, max_x;
    double px = 2.0, py = 2.0; // プレイヤー初期位置
    double pa = 0.0;           // プレイヤー向き
    
    Bullet bullet = {0, 0, 0, 0, 0};
    Zombie zombie = {8.0, 8.0, 30, 1}; // 奥にゾンビを配置

    int map[10][10] = {
        {1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,0,0,0,0,0,1},
        {1,0,1,1,0,0,0,0,0,1},
        {1,0,0,0,0,0,1,1,0,1},
        {1,0,0,0,0,0,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1}
    };

    initscr();
    noecho();
    curs_set(0);
    timeout(0);
    keypad(stdscr, TRUE);

    // --- 色の初期化 ---
    if (has_colors()) {
        start_color();
        // 遠い壁: 白/黒 (灰色に見える)
        init_pair(COLOR_WALL_FAR, COLOR_WHITE, COLOR_BLACK);
        // 中くらいの壁: 白/黒 (少し明るい)
        init_pair(COLOR_WALL_MID, COLOR_WHITE, COLOR_BLACK);
        // 近い壁: 黒/白 (反転して明るく)
        init_pair(COLOR_WALL_NEAR, COLOR_BLACK, COLOR_WHITE);
        // ゾンビ: 赤
        init_pair(COLOR_ZOMBIE, COLOR_RED, COLOR_BLACK);
        // 弾: 黄色
        init_pair(COLOR_BULLET, COLOR_YELLOW, COLOR_BLACK);
        // UI/武器: 白
        init_pair(COLOR_UI, COLOR_WHITE, COLOR_BLACK);
        // 床: 青/黒
        init_pair(COLOR_FLOOR, COLOR_BLUE, COLOR_BLACK);
    }

    while (1) {
        getmaxyx(stdscr, max_y, max_x);
        erase();

        // --- 1. ゾンビのAI (プレイヤーへ近づく) ---
        if (zombie.active) {
            double zdx = px - zombie.x;
            double zdy = py - zombie.y;
            double z_dist = sqrt(zdx*zdx + zdy*zdy);

            // プレイヤーが近く(距離5以内)にいたら追跡開始
            if (z_dist < 5.0 && z_dist > 0.8) {
                double move_speed = 0.03; // ゾンビはゆっくり
                double nx = zombie.x + (zdx / z_dist) * move_speed;
                double ny = zombie.y + (zdy / z_dist) * move_speed;
                
                // 壁衝突判定
                if (map[(int)ny][(int)nx] == 0) {
                    zombie.x = nx;
                    zombie.y = ny;
                }
            }
        }

        // --- 2. 3D壁レンダリング (色付き) ---
        for (int x = 0; x < max_x; x++) {
            double ray_angle = (pa - PI / 4.0) + ((double)x / (double)max_x) * (PI / 2.0);
            double vx = cos(ray_angle);
            double vy = sin(ray_angle);
            double distance = 0;
            int hit_wall = 0;
            
            while (!hit_wall && distance < 20.0) {
                distance += 0.05; // 精度を上げた
                int test_x = (int)(px + vx * distance);
                int test_y = (int)(py + vy * distance);
                if (test_x < 0 || test_x >= 10 || test_y < 0 || test_y >= 10 || map[test_y][test_x] == 1) hit_wall = 1;
            }

            int ceiling = (int)((double)max_y / 2.0 - (double)max_y / distance);
            int floor = max_y - ceiling;

            for (int y = 0; y < max_y; y++) {
                if (y < ceiling) mvaddch(y, x, ' '); // 天井 (黒)
                else if (y >= ceiling && y <= floor) {
                    // 距離に応じて色と文字を変える
                    if (distance <= 2.5) {
                        attron(COLOR_PAIR(COLOR_WALL_NEAR));
                        mvaddch(y, x, ' '); // 近い壁は明るい塊
                        attroff(COLOR_PAIR(COLOR_WALL_NEAR));
                    } else if (distance <= 5.0) {
                        attron(COLOR_PAIR(COLOR_WALL_MID));
                        mvaddch(y, x, '#' | A_BOLD);
                        attroff(COLOR_PAIR(COLOR_WALL_MID));
                    } else {
                        attron(COLOR_PAIR(COLOR_WALL_FAR));
                        mvaddch(y, x, '.');
                        attroff(COLOR_PAIR(COLOR_WALL_FAR));
                    }
                }
                else {
                    attron(COLOR_PAIR(COLOR_FLOOR));
                    mvaddch(y, x, '-'); // 床 (青)
                    attroff(COLOR_PAIR(COLOR_FLOOR));
                }
            }
        }

        // --- 3. ゾンビの描画 (赤色・詳細化) ---
        if (zombie.active) {
            double edx = zombie.x - px;
            double edy = zombie.y - py;
            double e_dist = sqrt(edx*edx + edy*edy);
            double e_angle = atan2(edy, edx) - pa;
            while (e_angle < -PI) e_angle += 2*PI;
            while (e_angle > PI) e_angle -= 2*PI;

            if (fabs(e_angle) < PI / 4.0 && e_dist > 0.5) {
                int ex = (int)((e_angle / (PI / 2.0) + 0.5) * max_x);
                int size = (int)(max_y / e_dist);
                
                attron(COLOR_PAIR(COLOR_ZOMBIE) | A_BOLD);
                if (ex >= 0 && ex < max_x) {
                    mvaddstr(max_y/2, ex - 1, "[XX]"); // ゾンビの顔
                    if (size > 3) mvaddstr(max_y/2 - 1, ex - 2, "ZOMBIE"); 
                }
                attroff(COLOR_PAIR(COLOR_ZOMBIE) | A_BOLD);
            }
        }

        // --- 4. 弾の移動・判定・描画 (黄色) ---
        if (bullet.active) {
            bullet.x += bullet.vx;
            bullet.y += bullet.vy;

            if (map[(int)bullet.y][(int)bullet.x] == 1) {
                bullet.active = 0;
            } 
            else if (zombie.active) {
                double hit_dist = sqrt(pow(bullet.x - zombie.x, 2) + pow(bullet.y - zombie.y, 2));
                if (hit_dist < 0.6) { // 当たり判定を少し広く
                    zombie.active = 0;
                    bullet.active = 0;
                }
            }

            if (bullet.active) {
                double bdx = bullet.x - px;
                double bdy = bullet.y - py;
                double b_dist = sqrt(bdx*bdx + bdy*bdy);
                double b_angle = atan2(bdy, bdx) - pa;
                while (b_angle < -PI) b_angle += 2*PI;
                while (b_angle > PI) b_angle -= 2*PI;

                if (fabs(b_angle) < PI / 4.0) {
                    int bx = (int)((b_angle / (PI / 2.0) + 0.5) * max_x);
                    if (bx >= 0 && bx < max_x) {
                        attron(COLOR_PAIR(COLOR_BULLET) | A_BOLD);
                        mvaddch(max_y/2, bx, 'O'); // 弾を「O」に
                        attroff(COLOR_PAIR(COLOR_BULLET) | A_BOLD);
                    }
                }
            }
        }

        // --- 5. 武器とUIの描画 (白) ---
        attron(COLOR_PAIR(COLOR_UI));
        int gx = max_x - 25;
        int gy = max_y - 1;
        mvaddstr(gy,     gx + 12, "MMMMMMMMMMMM");
        mvaddstr(gy - 1, gx + 10, "/ MMMMMMMMMM");
        mvaddstr(gy - 2, gx + 9,  "| [||||||] |"); 
        mvaddstr(gy - 4, gx + 4,  "________________");
        mvaddstr(gy - 5, gx + 3,  "|  _  _  _  _  |");
        mvaddstr(gy - 6, gx + 3,  "|________________|");
        mvaddstr(gy - 7, gx + 5,  "|__|");
        mvaddstr(gy - 6, gx + 1,  "(O)");
        mvaddch(max_y / 2, max_x / 2, '+' | A_BOLD); // 照準
        
        char status[50];
        sprintf(status, " ZOMBIE: %s ", zombie.active ? "ALIVE" : "DEAD ");
        mvaddstr(0, 0, status);
        attroff(COLOR_PAIR(COLOR_UI));

        refresh();
       // --- ミニマップの描画 (画面左上) ---
attron(COLOR_PAIR(COLOR_UI));

// マップの枠を描画
mvaddstr(0, 1, "+----------+");
for(int i=1; i<=10; i++) {
    mvaddch(i, 1, '|');
    mvaddch(i, 12, '|');
}
mvaddstr(11, 1, "+----------+");

for (int my = 0; my < 10; my++) {
    for (int mx = 0; mx < 10; mx++) {
        int sy = my + 1; // マップの表示y位置
        int sx = mx + 2; // マップの表示x位置

        if (my == (int)py && mx == (int)px) {
            // プレイヤー本体
            attron(COLOR_PAIR(COLOR_BULLET) | A_BOLD);
            mvaddch(sy, sx, 'P');
            
            // --- 視線方向の表示 ---
            // 角度に応じて、隣のマスに視線マークを書く
            int dir_x = (int)round(px + cos(pa));
            int dir_y = (int)round(py + sin(pa));
            
            // 視線の向きを記号で表現
            char view_char;
            double deg = pa * 180.0 / PI;
            // 角度を0-360に補正
            while(deg < 0) deg += 360;
            while(deg >= 360) deg -= 360;

            if (deg >= 337.5 || deg < 22.5)   view_char = '>';
            else if (deg >= 22.5  && deg < 67.5)  view_char = '\\';
            else if (deg >= 67.5  && deg < 112.5) view_char = 'v';
            else if (deg >= 112.5 && deg < 157.5) view_char = '/';
            else if (deg >= 157.5 && deg < 202.5) view_char = '<';
            else if (deg >= 202.5 && deg < 247.5) view_char = '^'; // ターミナル座標は下が+yなので逆転注意
            else if (deg >= 247.5 && deg < 292.5) view_char = '^';
            else view_char = '/';

            // 視線の先がマップ内なら表示
            int vsy = dir_y + 1;
            int vsx = dir_x + 2;
            if (dir_x >= 0 && dir_x < 10 && dir_y >= 0 && dir_y < 10 && (dir_x != (int)px || dir_y != (int)py)) {
                mvaddch(vsy, vsx, view_char);
            }
            attroff(COLOR_PAIR(COLOR_BULLET) | A_BOLD);

        } else if (zombie.active && my == (int)zombie.y && mx == (int)zombie.x) {
            attron(COLOR_PAIR(COLOR_ZOMBIE));
            mvaddch(sy, sx, 'Z');
            attroff(COLOR_PAIR(COLOR_ZOMBIE));
        } else if (map[my][mx] == 1) {
            mvaddch(sy, sx, '#');
        } else {
            // 視線が描画されていない場所のみ空白にする（上書き防止）
            if (mvinch(sy, sx) == ' ') mvaddch(sy, sx, ' ');
        }
    }
}
attroff(COLOR_PAIR(COLOR_UI));
        // --- 6. 入力処理 ---
        int ch = getch();
        if (ch == 'q') break;
        if (ch == ' ' && !bullet.active) {
            bullet.active = 1;
            bullet.x = px; bullet.y = py;
            bullet.vx = cos(pa) * 0.6; bullet.vy = sin(pa) * 0.6; // 弾速アップ
        }
        if (ch == KEY_LEFT) pa -= 0.15;
        if (ch == KEY_RIGHT) pa += 0.15;
        if (ch == KEY_UP) {
            double nx = px + cos(pa) * 0.25;
            double ny = py + sin(pa) * 0.25;
            if (map[(int)ny][(int)nx] == 0) { px = nx; py = ny; }
        }
        if (ch == KEY_DOWN) {
            double nx = px - cos(pa) * 0.2;
            double ny = py - sin(pa) * 0.2;
            if (map[(int)ny][(int)nx] == 0) { px = nx; py = ny; }
        }
        usleep(TICK);
    }

    endwin();
    return 0;
}
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QPixmap>
#include <QColor>
#include <QTimer>
#include <QMouseEvent>
#include<bits/stdc++.h>
#include<QElapsedTimer>
#include <QDebug>
#include<QMessageBox>

#define maxHt 1000
#define maxVt 1000
#define PI 180
#define GridOffset 1
#define fps 20
#define initial_eggs 4
#define prob_of_generation 20
#define Delay delay(1)
#define A 10
#define B 15
#define baseLine_y -240
#define basket_height 100
#define basket_width 100
#define speed_parameter 30
#define min_X -300
#define max_X 300
#define start_Y 300
#define basket_offset 100

using namespace std;
int score=0;
int lives=5;

bool game_closed=false;

Basket bs;
QVector<Egg> egg_array;
int lanes[]={-250,-150,-50,50,150,250};


MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->workArea->setMouseTracking(true);
    this->installEventFilter(this);
    QPixmap canvas = ui->workArea->pixmap(Qt::ReturnByValue);
    if (canvas.isNull()) {
        canvas = QPixmap(ui->workArea->size());
        canvas.fill(Qt::white);
        ui->workArea->setPixmap(canvas);
    }
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(0, 5);

    // Generate random number in the range [min, max]
    int index= distrib(gen);
    egg_array.push_back(Egg(lanes[index],start_Y));
    bs=Basket(50,baseLine_y);
    ui->lives->setText(QString::number(lives));
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::gameLoop);
    timer->start(1000/fps);
}

void MainWindow::delay(int ms){
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

void MainWindow::colorPoint(int x, int y, int r, int g, int b, int penwidth=1) {
    QPixmap canvas=ui->workArea->pixmap();
    QPainter painter(&canvas);
    QPen pen=QPen(QColor(r,g,b),penwidth);
    painter.setPen(pen);
    painter.drawPoint(x, y);
    ui->workArea->setPixmap(canvas);
}

pair<int,int> MainWindow::markBox(int x,int y)
{
    int width = ui->workArea->width();
    int height = ui->workArea->height();
    int centerX=width/2;
    int centerY=height/2;
    clickedPoint.setX(x);
    clickedPoint.setY(y);
    int X=floor((x-centerX)*1.0/GridOffset);
    int Y=floor((centerY-y)*1.0/GridOffset);
    int calcX=X*GridOffset+centerX+GridOffset/2.0;
    int calcY=centerY-Y*GridOffset-GridOffset/2.0;
    return {calcX,calcY};
}

pair<int,int> MainWindow::plotPixel(int x,int y)
{
    int centerX=(ui->workArea->width())/2;
    int centerY=ui->workArea->height()/2;
    int newX=x*GridOffset+centerX;
    int newY=centerY-y*GridOffset;
    return markBox(newX,newY);
}

void MainWindow::resetGame() {
    lives = 5;
    score = 0;
    egg_array.clear();
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(0, 5);

    // Generate a new egg to start the game again
    int index = distrib(gen);
    egg_array.push_back(Egg(lanes[index], start_Y));
    bs = Basket(50, baseLine_y);

    ui->lives->setText(QString::number(lives));
    ui->score->setText(QString::number(score));

    // Restart the timer
    timer->start(1000 / fps);
}

void MainWindow::gameLoop() {
    // Update the game state and repaint the scene
    static int frame_count=1;
    clear_canvas();
    ui->score->setText(QString::number(score));
    QVector<Egg>::Iterator iter=egg_array.begin();
    for(iter=egg_array.begin();iter!=egg_array.end();)
    {
        if(iter->x>bs.x-basket_width/2 && iter->x<bs.x+basket_width/2 && iter->y>bs.y-basket_height/2 && iter->y<bs.y+basket_height/2)
        {
            score++;
            iter=egg_array.erase(iter);
        }
        else if(iter->y<=baseLine_y)
        {
            iter=egg_array.erase(iter);
            lives--;
            ui->lives->setText(QString::number(lives));
            if(lives<=0)
            {
                timer->stop();
                QMessageBox msgBox;
                msgBox.setText("Game Over");
                msgBox.setInformativeText("Better luck next time!");
                msgBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Close);
                msgBox.setDefaultButton(QMessageBox::Retry);

                int ret = msgBox.exec();

                switch (ret) {
                case QMessageBox::Retry:
                    // Reset the game state
                    resetGame();
                    break;
                case QMessageBox::Close:
                    // Close the game
                    game_closed = true;
                    break;
                default:
                    // Should never be reached
                    break;
                }
            }

        }
        else
        {
            iter->fall(speed_parameter);
            iter++;
        }
    }
    if(frame_count%15==0)
    {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> distrib(0, 5);

        // Generate random number in the range [min, max]
        int index= distrib(gen);
        egg_array.push_back(Egg(lanes[index],start_Y));
    }
    frame_count++;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if(event->type()==QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event); // Cast the event to QKeyEvent
        int key = keyEvent->key(); // Get the key code

        switch (key) {
        case Qt::Key_Left:
            if(bs.x-basket_offset>=-250)bs.x-=basket_offset;
            break;
        case Qt::Key_Right:
            if(bs.x+basket_offset<=250)bs.x+=basket_offset;
            break;
        default:
            break;
        }
        return true;  // Mark the event as handled, no further processing

    }
    return QMainWindow::eventFilter(watched, event);
}
void MainWindow::paintEvent(QPaintEvent *event) {

    for(int i=0;i<egg_array.size();i++)
    {
        draw_bressenham_ellipse(egg_array[i].x,egg_array[i].y,A,B);
    }
    draw_bressenham_line(bs.x-basket_width/2,bs.y-basket_height/2,bs.x+basket_width/2,bs.y-basket_height/2,0,0,255);
    draw_bressenham_line(bs.x+basket_width/2,bs.y-basket_height/2,bs.x+basket_width/2,bs.y+basket_height/2,0,0,255);
    draw_bressenham_line(bs.x+basket_width/2,bs.y+basket_height/2,bs.x-basket_width/2,bs.y+basket_height/2,0,0,255);
    draw_bressenham_line(bs.x-basket_width/2,bs.y+basket_height/2,bs.x-basket_width/2,bs.y-basket_height/2,0,0,255);
    for(int y=bs.y-basket_height/2+1;y<bs.y+basket_height/2;y++)
    {
        draw_bressenham_line(bs.x-basket_width/2+1,y,bs.x+basket_width/2-1,y,180,250,180);
    }
}


void MainWindow::draw_bressenham_line(int x1, int y1, int x2, int y2, int r, int g, int b) {
    QPolygon polygon;

    // Calculate the differences
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    // Determine if we are stepping in the positive or negative direction
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    // Initial coordinates
    int x = x1;
    int y = y1;

    // Handle both slope cases |m| <= 1 and |m| > 1
    if (dx >= dy) {
        // Case 1: Slope is less than or equal to 1 (|m| <= 1)
        int p = 2 * dy - dx;
        for (int i = 0; i <= dx; i++) {
            pair<int,int> point_pair=plotPixel(x,y);
            polygon << QPoint(point_pair.first, point_pair.second); // Collect points into the polygon
            if (p >= 0) {
                y += sy;
                p -= 2 * dx;
            }
            p += 2 * dy;
            x += sx;
        }
    } else {
        // Case 2: Slope is greater than 1 (|m| > 1)
        int p = 2 * dx - dy;
        for (int i = 0; i <= dy; i++) {
            pair<int,int> point_pair=plotPixel(x,y);
            polygon << QPoint(point_pair.first, point_pair.second);
            if (p >= 0) {
                x += sx;
                p -= 2 * dy;
            }
            p += 2 * dx;
            y += sy;
        }
    }

    // Now draw all the points at once using QPainter
    QPixmap canvas = ui->workArea->pixmap();
    QPainter painter(&canvas);
    painter.setPen(QPen(QColor(r, g, b), 1)); // Set the color and pen width
    painter.drawPoints(polygon); // Draw the entire polygon (batch of points)
    ui->workArea->setPixmap(canvas); // Update the canvas
}

void MainWindow::draw_bressenham_ellipse(int x_center, int y_center, int a, int b) {
    QPolygon polygon;

    int x = 0, y = b;
    double d1 = b * b - a * a * b + (0.25 * a * a);
    while (b * b * x < a * a * y) {
        pair<int,int> point_pair=plotPixel(x_center+x,y_center+y);
        polygon << QPoint(point_pair.first, point_pair.second);
        point_pair=plotPixel(x_center-x,y_center+y);
        polygon<< QPoint(point_pair.first,point_pair.second);
        point_pair=plotPixel(x_center-x,y_center-y);
        polygon<< QPoint(point_pair.first,point_pair.second);
        point_pair=plotPixel(x_center+x,y_center-y);
        polygon<< QPoint(point_pair.first,point_pair.second);
        if (d1 < 0) {
            d1 += b * b * (1 + 2 * x);
        } else {
            d1 += b * b * (2 * x + 3) + a * a * (2 - 2 * y);
            y--;
        }
        x++;
    }

    double d2 = b * b * (x + 0.5) * (x + 0.5) + a * a * (y - 1) * (y - 1) - a * a * b * b;
    while (y >= 0) {
        pair<int,int> point_pair=plotPixel(x_center+x,y_center+y);
        polygon << QPoint(point_pair.first, point_pair.second);
        point_pair=plotPixel(x_center-x,y_center+y);
        polygon<< QPoint(point_pair.first,point_pair.second);
        point_pair=plotPixel(x_center-x,y_center-y);
        polygon<< QPoint(point_pair.first,point_pair.second);
        point_pair=plotPixel(x_center+x,y_center-y);
        polygon<< QPoint(point_pair.first,point_pair.second);
        if (d2 < 0) {
            d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
            x++;
        } else {
            d2 += a * a * (-2 * y + 3);
        }
        y--;
    }

    // Draw all points in one go using QPainter
    QPixmap canvas = ui->workArea->pixmap();
    QPainter painter(&canvas);
    painter.setPen(QPen(Qt::black, 1)); // Set pen color to black for the ellipse
    painter.drawPoints(polygon); // Draw the collected points
    ui->workArea->setPixmap(canvas); // Update the canvas
}


void MainWindow::clear_canvas()
{
    QPixmap canvas = ui->workArea->pixmap(Qt::ReturnByValue);
    if (!canvas.isNull()) {
        canvas.fill(Qt::white);
        ui->workArea->setPixmap(canvas);
    }
}

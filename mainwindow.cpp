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

using namespace std;
int X_center=-230;
int Y_center=230;
int score=0;

bool game_closed=false;

Basket bs;
QVector<Egg> egg_array;
MainWindow::~MainWindow()
{
    delete ui;
}

void generate_eggs(int start_x)
{
    for(int i=0;i<initial_eggs;i++)
    {
        egg_array.push_back(Egg(start_x,Y_center));
        start_x+=30;
    }
    start_x+=100;
    for(int i=0;i<initial_eggs;i++)
    {
        egg_array.push_back(Egg(start_x,Y_center));
        start_x+=30;
    }
    start_x+=100;
    for(int i=0;i<initial_eggs;i++)
    {
        egg_array.push_back(Egg(start_x,Y_center));
        start_x+=30;
    }
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
    generate_eggs(-400);
    bs=Basket(0,baseLine_y);
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



void MainWindow::markBox(int x,int y,int r,int g,int b)
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
    colorPoint(calcX,calcY,r,g,b,GridOffset);
}

void MainWindow::plotPixel(int x,int y,int r,int g,int b)
{
    MainWindow::polygon_points.push_back({x,y});
    int centerX=(ui->workArea->width())/2;
    int centerY=ui->workArea->height()/2;
    int newX=x*GridOffset+centerX;
    int newY=centerY-y*GridOffset;
    markBox(newX,newY,r,g,b);
    MainWindow::drawn_points.insert({x,y,r,g,b});
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
        }
        else
        {
            iter->fall(speed_parameter);
            iter++;
        }
    }
    if(frame_count%10==0)
    {
        generate_eggs(-400);
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
            if(bs.x-30>=-400)bs.x-=30;
            break;
        case Qt::Key_Right:
            if(bs.x+30<=400)bs.x+=30;
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
}


void MainWindow::draw_bressenham_line(int x1, int y1, int x2, int y2,int r,int g,int b)
{
    // Calculate the differences
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    // Determine if we are stepping in the positive or negative direction
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1< y2) ? 1 : -1;

    // Initial coordinates
    int x = x1;
    int y = y1;

    // Handle both slope cases |m| <= 1 and |m| > 1
    if (dx >= dy)
    {
        // Case 1: Slope is less than or equal to 1 (|m| <= 1)
        int p = 2 * dy - dx;
        for (int i = 0; i <= dx; i++)
        {
            // Convert grid coordinates back to screen coordinates and plot
            MainWindow::polygon_points.push_back({x,y});
            MainWindow::plotPixel(x,y,r,g,b);
            // Update p and x, y
            if (p >= 0)
            {
                y += sy;
                p -= 2 * dx;
            }
            p += 2 * dy;
            x += sx;
        }
    }
    else
    {
        // Case 2: Slope is greater than 1 (|m| > 1)
        int p = 2 * dx - dy;
        for (int i = 0; i <= dy; i++)
        {
            // Convert grid coordinates back to screen coordinates and plot
            MainWindow::polygon_points.push_back({x,y});
            MainWindow::plotPixel(x,y,0,255,0);

            // Update p and x, y
            if (p >= 0)
            {
                x += sx;
                p -= 2 * dy;
            }
            p += 2 * dx;
            y += sy;
        }
    }
}



void MainWindow::draw_bressenham_ellipse(int x_center,int y_center,int a,int b)
{
    int x=0,y=b;
    double d1=b*b-a*a*b+(0.25*a*a);
    while(b*b*x<a*a*y)
    {
        plotPixel(x_center+x,y_center+y,0,0,0);
        plotPixel(x_center-x,y_center+y,0,0,0);
        plotPixel(x_center-x,y_center-y,0,0,0);
        plotPixel(x_center+x,y_center-y,0,0,0);
        if(d1<0)
        {
            d1+=b*b*(1+2*x);
        }
        else
        {
            d1+=b*b*(2*x+3)+a*a*(2-2*y);
            y--;
        }
        x++;
    }
    double d2=b*b*(x+0.5)*(x-0.5)+a*a*(y-1)*(y-1)-a*a*b*b;
    while(y>=0)
    {
        if(d2<0)
        {
            d2+=b*b*(2*x+2)+a*a*(-2*y+3);
            x++;
        }
        else
        {
            d2+=a*a*(-2*y+3);
        }
        y--;
        plotPixel(x_center+x,y_center+y,0,0,0);
        plotPixel(x_center-x,y_center+y,0,0,0);
        plotPixel(x_center-x,y_center-y,0,0,0);
        plotPixel(x_center+x,y_center-y,0,0,0);
    }
}

void MainWindow::clear_canvas()
{
    QPixmap canvas = ui->workArea->pixmap(Qt::ReturnByValue);
    if (!canvas.isNull()) {
        canvas.fill(Qt::white);
        ui->workArea->setPixmap(canvas);
    }
}

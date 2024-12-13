import java.util.ArrayList;

public ArrayList<Ball> balls = new ArrayList<Ball>();
int num = 0;
float gravity = 0.03;
float friction = -1;

void setup()
{
  size(640, 360, P3D);
  smooth();
  noStroke();
}

void draw()
{
  background(0);
  
  // create backgroud cube
  stroke(255);
  
  // background square
  line(0, 0, -500, width, 0, -500);
  line(0, 0, -500, 0, height, -500);
  line(0, height, -500, width, height, -500);
  line(width, height, -500, width, 0, -500);
  
  // perspective lines
  line(0, 0, -500, 0, 0, 0);
  line(width, 0, -500, width, 0, 0);
  line(0, height, -500, 0, height, 0);
  line(width, height, -500, width, height, 0);
  
  for (int i = 0; i < num; i++)  {
    Ball b = balls.get(i);
    b.run();
  }
  
  // Submission Info - for student to fill in
  String title_name = "SID500177868_Assignment1b";
  surface.setTitle(title_name);
}

void mouseClicked() {
  Ball b = new Ball(mouseX, mouseY, num);
  balls.add(b);
  println(b.zSpeed);
  num ++;
}

//BALL CLASS
class Ball {
  float initx, inity;
  float x, y, z;
  float xSpeed, ySpeed, zSpeed;
  float r;
  int id;
  PImage img;
  
  Ball(float x1, float y1, int n) {
    initx = x1;
    inity = y1;
    this.x = x1;
    this.y = y1;
    this.z = 1;
    this.xSpeed = random(-4,4);
    this.ySpeed = random(-4,4);
    this.zSpeed = 4;
    this.r = 30;
    this.id = n;
    // random texture for ball
    float a = random(1,5);
    int imgNum = int (a);
    this.img = loadImage("img" + imgNum +".jpg");
  }
  
  void run() {
    gravity();
    collide();
    move();
    display();
  }
  
  // collide with other balls
  void collide() {
    for (int i = 0; i < num; i++) {
      if ( i != id){
        float dx = balls.get(i).x - x;
        float dy = balls.get(i).y - y;
        float dz = balls.get(i).z - z;
        float distance = sqrt(dx*dx + dy*dy + dz*dz);
        if (distance < 100) { 
          this.xSpeed = this.xSpeed*friction;
          this.ySpeed = this.ySpeed*friction;
          this.zSpeed = this.zSpeed*friction;
        }  
      }
    }   
  }
  
  void gravity() {
    this.ySpeed+=0.1;
  }
  
  void move() {
    // stop on the floor
    if (this.y > 377){
      this.ySpeed = 0;
      this.xSpeed = 0;
      this.zSpeed = 0;
    }
    
    // change position
    this.x = this.x + this.xSpeed;
    this.y = this.y + this.ySpeed;
    this.z = this.z + this.zSpeed;
    
    // if boucing the two side
    if (this.x > width-30 || this.x < 30){
      this.xSpeed = this.xSpeed*friction;
    }
    // if boucing the top and botton
    if (this.y > height-30 || this.y < 30){
      this.ySpeed = this.ySpeed *-0.95;
    }
    // if boucing the back and front
    if (this.z > 500 || this.z < 0){
      this.zSpeed = this.zSpeed*friction;
    }
    
    // when reach to the floor.
    if(y > 350){
      // slow down x
      this.xSpeed = this.xSpeed *0.995;
      this.zSpeed = this.zSpeed *0.995;
    }
  }
  
  void display() {
    pushMatrix();
    PShape shape = createShape(SPHERE, 60); 
    shape.setTexture(img);
    translate (this.x, this.y, -this.z);
    //stroke(255);
    fill(255);
    shape(shape);
    popMatrix();
  }
}

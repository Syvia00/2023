## Instruction
1. open .pde file using processing and click the run buttom to run the file
2. use mouse to click on the screen and a ball will show on the position where mouse click and shoot out with random direction

## Code Description
The main structure is setup a window and create a cube with the width and height same as window size. Then the front screen face to us is one of surface of cube.

Then we have a Arraylist to store all the balls. We add the ball to the list when the mouse click on the screen, and ball will initial with the x, y as the position mouse clicked. For initial the ball, also set random x speed and y speed to give a random move direction. In addition, set a random texture from 4 textures to the ball.

The movement of each ball will be affect by the gravity and friction between balls and wall. So there are collide() to adjust the movement after boucing with other balls, gravity() to adjust the speed on y axis and move() to ensure ball will change direction after boucing wall, reduce speed when reach on the floor and finally stop on the floor.

At last, use display() to present the ball on the screen.